// Model backends: GLM-5.3-Flash over one or more API providers (default) with per-provider rate
// limiting and failover, and llama-server on this PC (optional, appendix plan).
#include "common.hpp"
#include <curl/curl.h>
#include <format>
#include <random>
#include <csignal>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>

namespace pbsd {
// USD per 1M tokens from the provider that served the turn; peak doubles (DeepSeek only). Local is free.
double Usage::usd() const { return local ? 0 : (peak ? 2 : 1) * (hit * p_hit + miss * p_in + out * p_out) / 1e6; }

// ---- token buckets: RPM and TPM refill continuously; capacity = one minute of budget.
RateLimiter::RateLimiter(int conc, int rpm, int64_t tpm)
    : conc_(conc), rpm_(rpm), tpm_(tpm), req_tokens_(rpm), tok_tokens_(double(tpm)) {}
void RateLimiter::refill(double t) {
  if (last_ == 0) last_ = t;
  double dt = std::max(0.0, t - last_); last_ = t;
  if (rpm_ > 0) req_tokens_ = std::min<double>(rpm_, req_tokens_ + dt * rpm_ / 60.0);
  if (tpm_ > 0) tok_tokens_ = std::min<double>(double(tpm_), tok_tokens_ + dt * double(tpm_) / 60.0);
}
bool RateLimiter::try_acquire(int64_t est, double t) {
  if (cooling(t)) return false;
  std::lock_guard g(m_);
  refill(t);
  if (conc_ > 0 && inflight_ >= conc_) return false;
  if (rpm_ > 0 && req_tokens_ < 1) return false;
  // A single request larger than the whole bucket may go once the bucket is full.
  if (tpm_ > 0 && tok_tokens_ < std::min<double>(double(est), double(tpm_))) return false;
  ++inflight_;
  if (rpm_ > 0) req_tokens_ -= 1;
  if (tpm_ > 0) tok_tokens_ -= double(est);
  return true;
}
void RateLimiter::release(int64_t est, int64_t actual, double t) {
  std::lock_guard g(m_);
  refill(t);
  --inflight_;
  if (tpm_ > 0 && actual > 0) tok_tokens_ = std::min<double>(double(tpm_), tok_tokens_ + double(est - actual));
}

// Peak: 01:00-04:00 and 06:00-10:00 UTC on weekdays (12:00-15:00 and 17:00-21:00 Sydney, AEDT).
bool Api::peak_now() {
  std::time_t t = std::time(nullptr); std::tm u{}; gmtime_r(&t, &u);
  bool wd = u.tm_wday >= 1 && u.tm_wday <= 5;
  return wd && ((u.tm_hour >= 1 && u.tm_hour < 4) || (u.tm_hour >= 6 && u.tm_hour < 10));
}

namespace {
size_t wr(char* p, size_t s, size_t n, void* u) { static_cast<std::string*>(u)->append(p, s * n); return s * n; }
size_t hd(char* p, size_t s, size_t n, void* u) {
  std::string h(p, s * n);
  if (h.size() > 12 && strncasecmp(h.c_str(), "retry-after:", 12) == 0) *static_cast<double*>(u) = atof(h.c_str() + 12);
  return s * n;
}
Http perform(const std::string& url, const std::string* payload, const std::string& bearer, long timeout_s) {
  CURL* h = curl_easy_init();
  Http r{0, {}, 0, false};
  curl_slist* hdr = curl_slist_append(nullptr, "Content-Type: application/json");
  if (!bearer.empty()) hdr = curl_slist_append(hdr, ("Authorization: Bearer " + bearer).c_str());
  curl_easy_setopt(h, CURLOPT_URL, url.c_str());
  curl_easy_setopt(h, CURLOPT_HTTPHEADER, hdr);
  if (payload) curl_easy_setopt(h, CURLOPT_POSTFIELDS, payload->c_str());
  curl_easy_setopt(h, CURLOPT_WRITEFUNCTION, wr); curl_easy_setopt(h, CURLOPT_WRITEDATA, &r.body);
  curl_easy_setopt(h, CURLOPT_HEADERFUNCTION, hd); curl_easy_setopt(h, CURLOPT_HEADERDATA, &r.retry_after);
  curl_easy_setopt(h, CURLOPT_TIMEOUT, timeout_s);
  curl_easy_setopt(h, CURLOPT_CONNECTTIMEOUT, 10L);
  curl_easy_setopt(h, CURLOPT_NOSIGNAL, 1L);
  CURLcode rc = curl_easy_perform(h);
  curl_easy_getinfo(h, CURLINFO_RESPONSE_CODE, &r.code);
  r.conn_error = rc != CURLE_OK;
  curl_slist_free_all(hdr); curl_easy_cleanup(h);
  return r;
}
std::string trim_v1(std::string u) {   // http://h:p/v1 -> http://h:p
  while (!u.empty() && u.back() == '/') u.pop_back();
  if (u.size() >= 3 && u.ends_with("/v1")) u.resize(u.size() - 3);
  return u;
}
}  // namespace
Http http_post(const std::string& url, const std::string& payload, const std::string& bearer, long t) { return perform(url, &payload, bearer, t); }
Http http_get(const std::string& url, long t) { return perform(url, nullptr, "", t); }

// ---- llama-server supervisor
LocalServer::LocalServer(std::string base, std::vector<std::string> cmd, int lt, int rm)
    : base_(std::move(base)), health_(trim_v1(base_) + "/health"), cmd_(std::move(cmd)), load_timeout_s_(lt), restart_max_(rm) {}
LocalServer::~LocalServer() = default;   // the server outlives the runner on purpose
bool LocalServer::healthy() { auto r = http_get(health_, 10); return !r.conn_error && r.code == 200; }
bool LocalServer::ensure_up() {
  std::lock_guard g(m_);
  if (healthy()) return true;
  // 503 = still loading the model: wait for it rather than restarting.
  auto wait_load = [&] {
    for (double end = now() + load_timeout_s_; now() < end; std::this_thread::sleep_for(std::chrono::seconds(5))) {
      auto r = http_get(health_, 10);
      if (!r.conn_error && r.code == 200) return true;
      if (pid_ > 0 && waitpid(pid_, nullptr, WNOHANG) == pid_) { pid_ = -1; return false; }   // died while loading
    }
    return false;
  };
  { auto r = http_get(health_, 10); if (!r.conn_error && r.code == 503 && wait_load()) return true; }
  if (cmd_.empty() || restarts_ >= restart_max_) { given_up_ = true; return false; }
  if (pid_ > 0) { kill(-pid_, SIGKILL); waitpid(pid_, nullptr, 0); pid_ = -1; }   // hung: kill our old child
  ++restarts_;
  pid_t p = fork();
  if (p == 0) {
    setsid();
    std::vector<std::string> a = cmd_;
    std::vector<char*> v; for (auto& s : a) v.push_back(s.data()); v.push_back(nullptr);
    execvp(v[0], v.data()); _exit(127);
  }
  pid_ = p;
  if (wait_load()) { given_up_ = false; return true; }
  return false;
}

// ---- Api
Api::Api(const Config& c) : c_(c) {
  curl_global_init(CURL_GLOBAL_DEFAULT);
  local_ = std::make_unique<LocalServer>(c.local_url, c.server_cmd, c.load_timeout_s, c.restart_max);
  if (!c.aux_url.empty()) aux_ = std::make_unique<LocalServer>(c.aux_url, std::vector<std::string>{}, c.load_timeout_s, 0);
  for (auto& p : c.providers) ps_.push_back(std::make_unique<ProvState>(p));
}

std::string Api::status_line() {
  double s = gen_s_.load();
  std::string b = c_.backend == "local" ? std::format("local {} @ {} ({} slots)", c_.local_model, c_.local_url, c_.local_slots)
                                         : std::string("api");
  b += std::format("   {:.1f} tok/s out", s > 0 ? out_tok_.load() / s : 0.0);
  if (c_.backend == "local") b += std::format("   restarts {}{}", local_->restarts(), local_->given_up() ? "  [server down]" : "");
  if (fallback_turns_) b += std::format("   API fallback turns {}", fallback_turns_.load());
  for (size_t i = 0; i < ps_.size(); ++i) {
    auto& p = *ps_[i];
    b += std::format("\n    {:<12} {:<8} {:<22} in flight {}/{}  ok {}  429 {}  err {}{}", c_.providers[i].name, c_.providers[i].tier,
                     c_.providers[i].model, p.lim.in_flight(), p.lim.limit(), p.ok.load(), p.limited.load(), p.failed.load(),
                     p.lim.cooling(now()) ? "  [cooling]" : "");
  }
  return b;
}

std::expected<json, std::string> Api::chat(const json& messages, const json& tools, Usage& u, const std::string& role, int slot,
                                           const std::string& affinity, const std::string& tier,
                                           const std::string& effort) {
  std::string be = c_.backend;
  if (auto it = c_.role_backend.find(role); it != c_.role_backend.end()) be = it->second;
  if (be == "aux" && !aux_) be = c_.backend;
  json body{{"messages", messages}};
  if (!tools.is_null() && !tools.empty()) body["tools"] = tools;
  if (be == "api") {
    if (!effort.empty()) body["reasoning_effort"] = effort;
    else if (auto it = c_.role_reasoning_effort.find(role); it != c_.role_reasoning_effort.end()) { if (!it->second.empty()) body["reasoning_effort"] = it->second; }
    else if (!c_.reasoning_effort.empty()) body["reasoning_effort"] = c_.reasoning_effort;
    return chat_api(body, u, affinity, tier);
  }
  body["model"] = be == "aux" ? c_.aux_model : c_.local_model;
  body["max_tokens"] = c_.max_tokens;
  auto r = chat_local(be == "aux" ? c_.aux_url : c_.local_url, body, u, slot, be == "aux" ? aux_.get() : local_.get());
  if (!r && c_.fallback_api && !c_.providers.empty()) {
    ++fallback_turns_;
    json b2{{"messages", messages}};
    if (!tools.is_null() && !tools.empty()) b2["tools"] = tools;
    u.local = false;
    return chat_api(b2, u, affinity, tier);
  }
  return r;
}

std::expected<json, std::string> Api::chat_local(const std::string& url, json body, Usage& u, int slot, LocalServer* srv) {
  body["cache_prompt"] = true;                    // reuse the slot's KV prefix
  if (slot >= 0) body["id_slot"] = slot;
  std::string payload = body.dump();
  u.local = true;
  for (int attempt = 0; attempt < 6; ++attempt) {
    double t0 = now();
    Http r = http_post(trim_v1(url) + "/v1/chat/completions", payload, "", c_.request_timeout_s);
    if (!r.conn_error && r.code == 200) {
      json j = json::parse(r.body, nullptr, false);
      if (j.is_discarded()) continue;
      auto us = j.value("usage", json::object());
      int64_t out = us.value("completion_tokens", 0), pr = us.value("prompt_tokens", 0);
      u.miss += pr; u.out += out;
      out_tok_ += out; prompt_tok_ += pr;
      gen_s_ = gen_s_.load() + (now() - t0);
      return j;
    }
    // 400 with a context error: the caller must compact; anything else: check the server.
    if (r.code == 400) return std::unexpected("400: " + r.body.substr(0, 400));
    if (!srv->ensure_up()) return std::unexpected("local server down: " + (r.conn_error ? std::string("no connection") : std::to_string(r.code)));
    std::this_thread::sleep_for(std::chrono::seconds(std::min(60, 5 << attempt)));
  }
  return std::unexpected("local server failed after 6 tries");
}

std::vector<size_t> provider_order(const std::vector<Provider>& ps, const std::string& affinity, const std::string& tier) {
  bool any_of_tier = false;
  for (auto& p : ps) any_of_tier = any_of_tier || (!tier.empty() && p.tier == tier);
  std::vector<size_t> primary, overflow;
  for (size_t i = 0; i < ps.size(); ++i) {
    if (any_of_tier && ps[i].tier != tier) continue;
    (ps[i].weight > 0 ? primary : overflow).push_back(i);
  }
  if (primary.empty()) std::swap(primary, overflow);
  std::vector<size_t> out;
  if (!primary.empty()) {
    double total = 0; for (size_t i : primary) total += ps[i].weight > 0 ? ps[i].weight : 1;
    // Weighted choice of the first provider, stable for a given affinity key.
    double x = affinity.empty() ? 0 : double(std::hash<std::string>{}(affinity) % 1000003) / 1000003.0 * total;
    size_t start = 0;
    for (size_t k = 0; k < primary.size(); ++k) { x -= ps[primary[k]].weight > 0 ? ps[primary[k]].weight : 1; if (x < 0) { start = k; break; } }
    for (size_t k = 0; k < primary.size(); ++k) out.push_back(primary[(start + k) % primary.size()]);
  }
  out.insert(out.end(), overflow.begin(), overflow.end());
  return out;
}

// Provider order for one request: provider_order() (weighted affinity, then overflow providers).
std::expected<json, std::string> Api::chat_api(json body, Usage& u, const std::string& affinity, const std::string& tier) {
  if (ps_.empty()) return std::unexpected("no API providers configured (set ZAI_API_KEY or providers in config.json)");
  std::mt19937 rng(std::random_device{}());
  // Only providers serving the requested tier; if none is configured, any provider.
  bool any_of_tier = false;
  for (auto& p : c_.providers) any_of_tier = any_of_tier || (!tier.empty() && p.tier == tier);
  auto eligible = [&](size_t i) { return !any_of_tier || c_.providers[i].tier == tier; };
  const std::vector<size_t> order = provider_order(c_.providers, affinity, tier);
  const size_t n = order.size();
  const int64_t est = int64_t(body.dump().size() / 4) + c_.expected_out_tokens;
  std::string last_err = "no provider accepted the request";
  for (int attempt = 0; attempt < 12; ++attempt) {
    // Wait (up to 10 min) for a provider with room in its buckets; first choice by affinity.
    int pick = -1;
    for (double end = now() + 600; pick < 0 && now() < end; ) {
      for (size_t k = 0; k < n && pick < 0; ++k) {
        size_t i = order[k];
        if (!eligible(i)) continue;
        if (c_.providers[i].offpeak_only && peak_now()) continue;
        if (c_.providers[i].keys.empty()) continue;
        if (ps_[i]->lim.try_acquire(est, now())) pick = int(i);
      }
      if (pick < 0) std::this_thread::sleep_for(std::chrono::milliseconds(200 + rng() % 300));
    }
    if (pick < 0) return std::unexpected("API: all providers saturated or cooling for 10 min");
    const Provider& p = c_.providers[pick];
    ProvState& st = *ps_[pick];
    json b = body;
    b["model"] = p.model;
    b["max_tokens"] = c_.api_max_tokens;
    for (auto& [k, v] : p.extra.items()) b[k] = v;
    std::string key; { std::lock_guard g(m_); key = p.keys[std::min(st.key, p.keys.size() - 1)]; }
    double t0 = now();
    Http r = http_post(p.url, b.dump(), key, c_.api_timeout_s);
    if (!r.conn_error && r.code == 200) {
      json j = json::parse(r.body, nullptr, false);
      if (j.is_discarded() || !j.contains("choices")) { st.lim.release(est, 0, now()); ++st.failed; last_err = p.name + ": bad body"; continue; }
      auto us = j.value("usage", json::object());
      int64_t prompt = us.value("prompt_tokens", int64_t(0)), out = us.value("completion_tokens", int64_t(0));
      int64_t hit = us.value("prompt_cache_hit_tokens", int64_t(0));
      if (!hit && us.contains("prompt_tokens_details") && us["prompt_tokens_details"].is_object())
        hit = us["prompt_tokens_details"].value("cached_tokens", int64_t(0));
      st.lim.release(est, prompt + out, now());
      ++st.ok;
      u.hit += hit; u.miss += std::max<int64_t>(0, prompt - hit); u.out += out;
      u.p_in = p.price_in; u.p_hit = p.price_hit; u.p_out = p.price_out; u.provider = p.name;
      u.peak = p.peak_double && peak_now(); u.local = false;
      if (c_.backend == "api") { out_tok_ += out; prompt_tok_ += prompt; gen_s_ = gen_s_.load() + (now() - t0); }
      return j;
    }
    st.lim.release(est, 0, now());
    last_err = std::format("{}: {}", p.name, r.conn_error ? std::string("connection error") : std::to_string(r.code) + " " + r.body.substr(0, 200));
    if (r.code == 401 || r.code == 403) {   // bad key: rotate, else take this provider out for an hour
      std::lock_guard g(m_);
      if (st.key + 1 < p.keys.size()) { ++st.key; continue; }
      st.lim.cooldown(now() + 3600); ++st.failed; continue;
    }
    if (r.code == 402) { st.lim.cooldown(now() + 3600); ++st.failed;   // out of credit here: others carry on
      bool any = false; for (size_t i : order) any = any || (eligible(i) && !ps_[i]->lim.cooling(now()));
      if (!any) return std::unexpected("402: API balance exhausted on every provider of this tier"); continue; }
    if (r.code == 400 || r.code == 413) return std::unexpected(last_err);   // the request itself is bad
    // 429 / 5xx / connection: cool this provider (Retry-After or exponential), fail over to the next.
    if (r.code == 429) ++st.limited; else ++st.failed;
    double back = std::min(300.0, 2.0 * (1 << std::min(attempt, 7))) * std::uniform_real_distribution<>(0.5, 1.5)(rng);
    st.lim.cooldown(now() + std::max(back, r.retry_after));
  }
  return std::unexpected("API failed after 12 tries: " + last_err);
}

// ---- context management
size_t approx_tokens(const json& msgs) { return msgs.dump().size() / 4; }   // about 4 bytes per token
int compact_history(json& msgs, size_t budget) {
  // Keep system messages, the task message and the last 6 messages; elide older tool results,
  // then older assistant contents, oldest first.
  int n = 0;
  for (const char* kind : {"tool", "assistant"})
    for (size_t i = 0; i + 6 < msgs.size() && approx_tokens(msgs) > budget; ++i) {
      auto& m = msgs[i];
      if (m.value("role", "") != kind) continue;
      if (m.contains("content") && m["content"].is_string() && m["content"].get<std::string>().size() > 200) {
        m["content"] = "[elided to fit the context; reread the file if needed]"; ++n;
      }
    }
  return n;
}
}  // namespace pbsd
