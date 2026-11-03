#include "common.hpp"
#include <curl/curl.h>
#include <random>
#include <thread>

namespace pbsd {
// USD per 1M tokens, off-peak; peak is double.
static constexpr double kHit = 0.003, kMiss = 0.15, kOut = 0.60;
double Usage::usd() const { return (peak ? 2 : 1) * (hit * kHit + miss * kMiss + out * kOut) / 1e6; }

// Peak: 01:00-04:00 and 06:00-10:00 UTC on weekdays (12:00-15:00 and 17:00-21:00 Sydney, AEDT).
bool Api::peak_now() {
  std::time_t t = std::time(nullptr); std::tm u{}; gmtime_r(&t, &u);
  bool wd = u.tm_wday >= 1 && u.tm_wday <= 5;
  return wd && ((u.tm_hour >= 1 && u.tm_hour < 4) || (u.tm_hour >= 6 && u.tm_hour < 10));
}
Api::Api(const Config& c) : c_(c) { curl_global_init(CURL_GLOBAL_DEFAULT); }

namespace {
size_t wr(char* p, size_t s, size_t n, void* u) { static_cast<std::string*>(u)->append(p, s * n); return s * n; }
size_t hd(char* p, size_t s, size_t n, void* u) {
  std::string h(p, s * n);
  if (h.size() > 12 && strncasecmp(h.c_str(), "retry-after:", 12) == 0) *static_cast<double*>(u) = atof(h.c_str() + 12);
  return s * n;
}
}  // namespace

std::expected<json, std::string> Api::chat(const json& messages, const json& tools, Usage& u) {
  if (c_.keys.empty()) return std::unexpected("DEEPSEEK_API_KEY not set");
  std::mt19937 rng(std::random_device{}());
  json body{{"model", c_.model}, {"messages", messages}};
  if (!tools.is_null() && !tools.empty()) body["tools"] = tools;
  std::string payload = body.dump();
  for (int attempt = 0; attempt < 8; ++attempt) {
    while (now() < open_until_.load()) std::this_thread::sleep_for(std::chrono::seconds(5));
    while (peak_now()) std::this_thread::sleep_for(std::chrono::minutes(1));   // off-peak only
    std::string key; { std::lock_guard g(m_); key = c_.keys[std::min(key_, c_.keys.size() - 1)]; }
    CURL* h = curl_easy_init();
    std::string resp; double retry_after = 0; long code = 0;
    curl_slist* hdr = nullptr;
    hdr = curl_slist_append(hdr, "Content-Type: application/json");
    hdr = curl_slist_append(hdr, ("Authorization: Bearer " + key).c_str());
    curl_easy_setopt(h, CURLOPT_URL, c_.api_url.c_str());
    curl_easy_setopt(h, CURLOPT_HTTPHEADER, hdr);
    curl_easy_setopt(h, CURLOPT_POSTFIELDS, payload.c_str());
    curl_easy_setopt(h, CURLOPT_WRITEFUNCTION, wr); curl_easy_setopt(h, CURLOPT_WRITEDATA, &resp);
    curl_easy_setopt(h, CURLOPT_HEADERFUNCTION, hd); curl_easy_setopt(h, CURLOPT_HEADERDATA, &retry_after);
    curl_easy_setopt(h, CURLOPT_TIMEOUT, 900L);
    curl_easy_setopt(h, CURLOPT_NOSIGNAL, 1L);
    CURLcode rc = curl_easy_perform(h);
    curl_easy_getinfo(h, CURLINFO_RESPONSE_CODE, &code);
    curl_slist_free_all(hdr); curl_easy_cleanup(h);
    if (rc == CURLE_OK && code == 200) {
      json j = json::parse(resp, nullptr, false);
      if (j.is_discarded()) continue;
      auto us = j.value("usage", json::object());
      u.hit += us.value("prompt_cache_hit_tokens", 0);
      u.miss += us.value("prompt_cache_miss_tokens", us.value("prompt_tokens", 0) - us.value("prompt_cache_hit_tokens", 0));
      u.out += us.value("completion_tokens", 0); u.peak = peak_now();
      return j;
    }
    if (code == 402) return std::unexpected("402: API balance exhausted");
    if (code == 401) { std::lock_guard g(m_); if (key_ + 1 < c_.keys.size()) { ++key_; continue; } return std::unexpected("401: bad key"); }
    {
      std::lock_guard g(m_);
      double t = now(); fails_.push_back(t);
      std::erase_if(fails_, [t](double f) { return f < t - 60; });
      if (fails_.size() > 20) { open_until_ = t + 300; fails_.clear(); }   // circuit breaker
    }
    double back = std::min(600.0, double(1 << attempt)) * std::uniform_real_distribution<>(0.5, 1.5)(rng);
    std::this_thread::sleep_for(std::chrono::duration<double>(std::max(back, retry_after)));
  }
  return std::unexpected("API failed after 8 tries");
}
}  // namespace pbsd
