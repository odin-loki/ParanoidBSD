#include "common.hpp"
#include <cmath>
#include <ctime>
#include <format>

namespace pbsd {
static std::string bar(double f, int w) {
  f = std::clamp(f, 0.0, 1.0); int n = int(f * w); std::string s;
  for (int i = 0; i < w; ++i) s += i < n ? "\u2588" : "\u2591";
  return s;
}
static std::string dur(double h) {
  if (!std::isfinite(h)) return "unknown (no throughput yet)";
  long m = long(h * 60); return std::format("{}d {}h {}m", m / 1440, (m % 1440) / 60, m % 60);
}
static std::string clock_at(double h) {
  if (!std::isfinite(h)) return "-";
  std::time_t t = std::time(nullptr) + std::time_t(h * 3600); std::tm l{}; localtime_r(&t, &l);
  char b[64]; std::strftime(b, sizeof b, "%a %d %b %H:%M %Z", &l); return b;
}
static std::string num(int64_t v) {
  std::string s = std::to_string(v); for (int i = int(s.size()) - 3; i > 0; i -= 3) s.insert(i, ","); return s;
}
std::string render(const Db::Snapshot& s, const std::map<std::string, int>& running, const Config& c, double start, int w) {
  auto g = [&](const char* k) { auto it = s.states.find(k); return it == s.states.end() ? int64_t(0) : it->second; };
  int64_t all = 0; for (auto& [_, v] : s.states) all += v;
  int64_t done = g("passed"), left = all - done - g("escalated") - g("quarantined");
  double since = s.first_pass > 0 ? s.first_pass : start;
  double hours = std::max(1e-9, (now() - since) / 3600);
  double rate = (hours >= 24 && s.passed_24h) ? s.passed_24h / 24.0 : done / hours;   // rolling 24 h
  double eta = rate > 0 ? left / rate : INFINITY;
  double per = done ? s.cost / done : 0;
  std::string o = std::format("PBSD  {} {}/{} ({:.1f}%)\n", bar(all ? double(done) / all : 0, w), num(done), num(all),
                              all ? 100.0 * done / all : 0.0);
  o += std::format("  running {}  queued {}  blocked {}  failed {}  escalated {}  quarantined {}\n",
                   num(g("leased") + g("submitted")), num(g("ready")), num(g("blocked")), num(g("failed")),
                   num(g("escalated")), num(g("quarantined")));
  o += std::format("  throughput {:.1f} tasks/h   time left {}   ETA {}\n", rate, dur(eta), clock_at(eta));
  if (c.backend == "api" || s.cost > 0)
    o += std::format("  API cost so far ${:.2f}   projected total ${:.0f}{}\n", s.cost, s.cost + per * left,
                     "");
  for (auto& [p, n, d] : s.phases) o += std::format("  P{} {} {}/{}\n", p, bar(n ? double(d) / n : 0, 30), num(d), num(n));
  if (!running.empty()) {
    o += "  slots";
    for (auto& [k, v] : running)
      o += std::format("  {} {}/{}", k, v, k == "agent" && c.backend == "local" ? c.local_slots : (c.slots.count(k) ? c.slots.at(k).count : 0));
    o += std::format("   RAM {:.0f}%  load {:.1f}\n", 100 * mem_used_frac(), load1());
  }
  return o;
}
}  // namespace pbsd
