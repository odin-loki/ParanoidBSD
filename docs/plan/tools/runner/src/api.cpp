// Model backends: llama-server on this PC (default) and the DeepSeek API (optional fallback).
#include "common.hpp"
#include <curl/curl.h>
#include <format>
#include <random>
#include <csignal>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>

namespace pbsd {
// USD per 1M tokens, off-peak; peak is double. Local turns cost nothing.
static constexpr double kHit = 0.003, kMiss = 0.15, kOut = 0.60;
double Usage::usd() const { return local ? 0 : (peak ? 2 : 1) * (hit * kHit + miss * kMiss + out * kOut) / 1e6; }

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
}

std::string Api::status_line() {
  double s = gen_s_.load();
  std::string b = c_.backend == "local" ? std::format("local {} @ {} ({} slots)", c_.local_model, c_.local_url, c_.local_slots)
                                         : "api " + c_.model;
  b += std::format("   {:.1f} tok/s out", s > 0 ? out_tok_.load() / s : 0.0);
  if (c_.backend == "local") b += std::format("   restarts {}{}", local_->restarts(), local_->given_up() ? "  [server down]" : "");
  if (fallback_turns_) b += std::format("   API fallback turns {}", fallback_turns_.load());
  return b;
}

std::expected<json, std::string> Api::chat(const json& messages, const json& tools, Usage& u, const std::string& role, int slot) {
  std::string be = c_.backend;
  if (auto it = c_.role_backend.find(role); it != c_.role_backend.end()) be = it->second;
  if (be == "aux" && !aux_) be = c_.backend;
  json body{{"messages", messages}};
  if (!tools.is_null() && !tools.empty()) body["tools"] = tools;
  if (be == "api") { body["model"] = c_.model; return chat_api(body.dump(), u); }
  body["model"] = be == "aux" ? c_.aux_model : c_.local_model;
  body["max_tokens"] = c_.max_tokens;
  auto r = chat_local(be == "aux" ? c_.aux_url : c_.local_url, body, u, slot, be == "aux" ? aux_.get() : local_.get());
  if (!r && c_.fallback_api && !c_.keys.empty()) {
    ++fallback_turns_;
    json b2{{"model", c_.model}, {"messages", messages}};
    if (!tools.is_null() && !tools.empty()) b2["tools"] = tools;
    u.local = false;
    return chat_api(b2.dump(), u);
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

std::expected<json, std::string> Api::chat_api(const std::string& payload, Usage& u) {
  if (c_.keys.empty()) return std::unexpected("DEEPSEEK_API_KEY not set");
  std::mt19937 rng(std::random_device{}());
  for (int attempt = 0; attempt < 8; ++attempt) {
    while (now() < open_until_.load()) std::this_thread::sleep_for(std::chrono::seconds(5));
    while (peak_now()) std::this_thread::sleep_for(std::chrono::minutes(1));   // off-peak only
    std::string key; { std::lock_guard g(m_); key = c_.keys[std::min(key_, c_.keys.size() - 1)]; }
    double t0 = now();
    Http r = http_post(c_.api_url, payload, key, 900);
    if (!r.conn_error && r.code == 200) {
      json j = json::parse(r.body, nullptr, false);
      if (j.is_discarded()) continue;
      auto us = j.value("usage", json::object());
      u.hit += us.value("prompt_cache_hit_tokens", 0);
      u.miss += us.value("prompt_cache_miss_tokens", us.value("prompt_tokens", 0) - us.value("prompt_cache_hit_tokens", 0));
      u.out += us.value("completion_tokens", 0); u.peak = peak_now();
      if (c_.backend == "api") { out_tok_ += us.value("completion_tokens", 0); gen_s_ = gen_s_.load() + (now() - t0); }
      return j;
    }
    if (r.code == 402) return std::unexpected("402: API balance exhausted");
    if (r.code == 401) { std::lock_guard g(m_); if (key_ + 1 < c_.keys.size()) { ++key_; continue; } return std::unexpected("401: bad key"); }
    {
      std::lock_guard g(m_);
      double t = now(); fails_.push_back(t);
      std::erase_if(fails_, [t](double f) { return f < t - 60; });
      if (fails_.size() > 20) { open_until_ = t + 300; fails_.clear(); }   // circuit breaker
    }
    double back = std::min(600.0, double(1 << attempt)) * std::uniform_real_distribution<>(0.5, 1.5)(rng);
    std::this_thread::sleep_for(std::chrono::duration<double>(std::max(back, r.retry_after)));
  }
  return std::unexpected("API failed after 8 tries");
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
