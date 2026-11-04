// Plan analysis: critical path of the task graph and a throughput model of the whole run.
// pbsd_run cpath                 longest dependency chain (by turns and by tasks) and graph width
// pbsd_run estimate [k=v ...]    wall clock, binding constraint and USD cost per agent count
// Every figure from `estimate` is an estimate from the assumptions it prints, not a measurement.
#include "common.hpp"
#include <algorithm>
#include <format>
#include <fstream>
#include <functional>
#include <iostream>
#include <unordered_map>

namespace pbsd {
namespace {
struct Graph {
  std::vector<json> t;
  std::unordered_map<std::string, size_t> idx;
  int64_t turns = 0;
};
Graph load_graph(const Config& c) {
  std::ifstream f(c.tasks_json);
  if (!f) throw std::runtime_error("cannot read " + c.tasks_json);
  Graph g; json j = json::parse(f);
  for (auto& x : j) { g.idx[x["id"].get<std::string>()] = g.t.size(); g.turns += x.value("turns_budget", 6); g.t.push_back(x); }
  return g;
}
struct Cp { int64_t turns = 0, tasks = 0, depth = 0; std::vector<std::string> chain; std::map<int64_t, int64_t> width; };
Cp critical_path(const Graph& g) {
  std::vector<int64_t> lt(g.t.size(), -1), dep(g.t.size(), 0);
  std::vector<int64_t> lk(g.t.size(), 0);
  std::vector<long> nxt(g.t.size(), -1);
  std::function<void(size_t)> visit = [&](size_t i) {   // memoised DFS; the graph is a DAG
    if (lt[i] >= 0) return;
    int64_t best = 0, bestk = 0, bd = 0; long arg = -1;
    for (auto& d : g.t[i].value("deps", json::array())) {
      auto it = g.idx.find(d.get<std::string>()); if (it == g.idx.end()) continue;
      visit(it->second);
      if (lt[it->second] > best) { best = lt[it->second]; bestk = lk[it->second]; arg = long(it->second); }
      bd = std::max(bd, dep[it->second]);
    }
    lt[i] = best + g.t[i].value("turns_budget", 6); lk[i] = bestk + 1; nxt[i] = arg; dep[i] = bd + 1;
  };
  Cp r; size_t end = 0;
  for (size_t i = 0; i < g.t.size(); ++i) { visit(i); if (lt[i] > lt[end]) end = i; r.width[dep[i]]++; r.depth = std::max(r.depth, dep[i]); }
  r.turns = lt[end]; r.tasks = lk[end];
  for (long i = long(end); i >= 0; i = nxt[i]) r.chain.insert(r.chain.begin(), g.t[i]["id"].get<std::string>());
  return r;
}
}  // namespace

int cpath(const Config& c) {
  Graph g = load_graph(c); Cp r = critical_path(g);
  std::cout << std::format("{} tasks, {} budgeted turns\ncritical path: {} tasks, {} turns, depth {} levels\n  ",
                           g.t.size(), g.turns, r.tasks, r.turns, r.depth);
  for (size_t i = 0; i < r.chain.size(); ++i) std::cout << (i ? " -> " : "") << r.chain[i];
  std::cout << "\nwidth (tasks per dependency level):";
  for (auto& [d, n] : r.width) std::cout << std::format("  L{}={}", d, n);
  std::cout << "\n";
  return 0;
}

int estimate(const Config& c, int argc, char** argv) {
  // Assumptions (override with key=value). Defaults: AGENTS.md section 8.1.
  std::map<std::string, double> a{
      {"mult", 1.3},            // splits and retries, on tasks, turns and gate time
      {"out", 4000},            // output tokens per turn, reasoning included
      {"in", 40000},            // input tokens per turn (whole conversation resent)
      {"hit", 0.8},             // share of input billed as cache hits
      {"tok_s", 71},            // output tokens/s per stream (Z.ai, Artificial Analysis)
      {"ttft", 1.55},           // seconds to first token
      {"tool_s", 4},            // non-gate tool time per turn (git, file I/O, runner)
      {"gate_core_h", 7700},    // proofs 5,400 + compile 1,900 + QEMU 400 core-hours (plan 8.1)
      {"cores", 28},            // P920 cores for gates (32 minus 4 for OS, runner, git, tmpfs)
      {"burst_cores", 0},       // extra cloud CI cores
      {"burst_usd_core_h", 0.03},
      {"gate_cores", 2},        // cores per gate job (average)
      {"api_conc", 0},          // cap on requests in flight over all providers (0 = none)
      {"p_in", 0.15}, {"p_hit", 0.03}, {"p_out", 0.50}};
  for (int i = 2; i < argc; ++i) {
    std::string s = argv[i]; auto e = s.find('=');
    if (e == std::string::npos || !a.count(s.substr(0, e))) { std::cerr << "unknown assumption " << s << "\n"; return 2; }
    a[s.substr(0, e)] = std::stod(s.substr(e + 1));
  }
  Graph g = load_graph(c); Cp cp = critical_path(g);
  const double tasks = g.t.size() * a["mult"], turns = g.turns * a["mult"];
  const double turn_api = a["ttft"] + a["out"] / a["tok_s"];                       // s per API call
  const double api_s = turns * turn_api, tool_s = turns * a["tool_s"];
  const double gate_cs = a["gate_core_h"] * 3600 * a["mult"];                        // core-seconds
  const double gate_wall = gate_cs / a["gate_cores"];                                // job-seconds
  const double cores = a["cores"] + a["burst_cores"];
  const double agent_s = api_s + tool_s + gate_wall;                                 // agent-seconds held
  const double cp_s = cp.turns * a["mult"] * (turn_api + a["tool_s"]) + cp.tasks * gate_wall / tasks * 2;
  const double in_tok = turns * a["in"], out_tok = turns * a["out"];
  const double usd_api = (in_tok * a["hit"] * a["p_hit"] + in_tok * (1 - a["hit"]) * a["p_in"] + out_tok * a["p_out"]) / 1e6;
  const double usd_burst = a["burst_cores"] > 0 ? gate_cs / 3600 * a["burst_cores"] / cores * a["burst_usd_core_h"] : 0;
  std::cout << std::format(
      "ESTIMATE (assumptions, not measurements)\n"
      "tasks {:.0f} = {} x {}; turns {:.0f} = {} x {}; API call {:.1f} s = {} + {} / {}\n"
      "API stream-seconds {:.2f}M; tool {:.2f}M; gates {:.0f} core-h x {} = {:.0f} core-h on {:.0f} cores\n"
      "critical path {} tasks / {} turns -> {:.1f} h\n"
      "tokens: in {:.2f}B ({:.0f}% hits), out {:.0f}M -> API ${:.0f}{}\n\n",
      tasks, g.t.size(), a["mult"], turns, g.turns, a["mult"], turn_api, a["ttft"], a["out"], a["tok_s"],
      api_s / 1e6, tool_s / 1e6, a["gate_core_h"], a["mult"], gate_cs / 3600, cores, cp.tasks, cp.turns, cp_s / 3600,
      in_tok / 1e9, 100 * a["hit"], out_tok / 1e6, usd_api,
      usd_burst > 0 ? std::format(" + cloud CI ${:.0f}", usd_burst) : std::string());
  std::cout << "| agents | agent-bound | gate CPU | API cap | critical path | wall clock | binding | API in flight | out tok/min | in tok/min |\n"
               "|---|---|---|---|---|---|---|---|---|---|\n";
  for (int n : {4, 8, 16, 24, 32, 48, 64, 96, 128, 160, 192, 256}) {
    double b_agent = agent_s / n, b_gate = gate_cs / cores, b_api = a["api_conc"] > 0 ? api_s / a["api_conc"] : 0;
    double wall = std::max({b_agent, b_gate, b_api, cp_s});
    const char* bind = wall == b_agent ? "agents" : wall == b_gate ? "gate CPU" : wall == b_api ? "API concurrency" : "critical path";
    auto d = [](double s) { return std::format("{:.1f} d", s / 86400); };
    std::cout << std::format("| {} | {} | {} | {} | {} | **{}** | {} | {:.0f} | {:.0f}k | {:.2f}M |\n", n, d(b_agent), d(b_gate),
                             b_api > 0 ? d(b_api) : std::string("-"), d(cp_s), d(wall), bind, api_s / wall,
                             out_tok / wall * 60 / 1e3, in_tok / wall * 60 / 1e6);
  }
  double n_opt = agent_s / std::max({gate_cs / cores, a["api_conc"] > 0 ? api_s / a["api_conc"] : 0.0, cp_s});
  std::cout << std::format("\nagents needed to saturate the binding resource: {:.0f} (run about 15% more to cover queueing)\n", n_opt);
  return 0;
}
}  // namespace pbsd
