// Plan analysis: critical path of the task graph and a throughput model of the whole run.
// pbsd_run cpath                 longest dependency chain (by turns and by tasks) and graph width
// pbsd_run estimate [k=v ...]    wall clock, binding constraint and USD cost per agent count
// Every figure from `estimate` is an estimate from the assumptions it prints, not a measurement.
#include "common.hpp"
#include <algorithm>
#include <cmath>
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

// Cost and time model (AGENTS.md section 8). One turn = one API request carrying the whole
// conversation so far. Per task class: requests per attempt (the task's turns_budget), average
// input tokens per request, output tokens per request (reasoning included), attempts (retries,
// review rounds, proof iterations), gate runs per attempt and gate core-minutes per run.
int estimate(const Config& c, int argc, char** argv) {
  struct M { double in, out, attempts, gate_runs, gate_min; };
  std::map<std::string, M> m{
      {"decomposer", {35000, 5000, 1.3, 2, 4}},   {"implementer", {55000, 6000, 1.5, 6, 10}},
      {"tester", {50000, 5000, 1.4, 6, 10}},      {"reviewer", {60000, 5000, 1.3, 1, 10}},
      {"integrator", {80000, 6000, 2.0, 10, 300}},
      {"T0", {30000, 3000, 1.2, 2, 5}},           {"T1", {90000, 10000, 2.5, 100, 5}},
      {"T2", {90000, 8000, 2.5, 40, 30}},         {"T3", {70000, 6000, 1.6, 15, 15}},
      {"T4", {50000, 5000, 1.4, 6, 10}}};
  std::map<std::string, double> a{
      {"hit", 0.85},              // share of input billed as cache hits (agent loops resend the prefix)
      {"req", 1.0}, {"out", 1.0}, {"gate", 1.0},   // scenario multipliers on requests, output, gate time
      {"cores", 0},               // gate cores (physical); 0 = local slots + workers file
      {"core_usd", 0.0089},       // US$ per burst core-hour (GCP c3d-highcpu-360 spot, Sydney: $1.59984/h for 180 cores, 4 Nov 2026)
      {"pc_cores", 28},           // P920 cores used for gates (32 minus 4 for the OS, runner and git)
      {"pc_watts", 450}, {"aud_kwh", 0.33},   // P920 electricity
      {"tok_s_frontier", 80}, {"tok_s_flash", 70}, {"ttft", 2.0}, {"tool_s", 5},
      {"gate_cores", 2},          // cores held by one gate job
      {"api_conc", 0},            // cap on requests in flight over all providers (0 = none)
      {"p_in_frontier", 1.40}, {"p_hit_frontier", 0.26}, {"p_out_frontier", 4.40},
      {"p_in_flash", 0.15}, {"p_hit_flash", 0.03}, {"p_out_flash", 0.50},
      {"usd_aud", 1.4335},
      // Attempts on GLM-5.3-Flash relative to GLM-5.3: Flash solves fewer agentic tasks (Terminal-Bench
      // 2.1: 62.9 vs 71.5) and far fewer Lean proofs (ProofBench: 21% vs 49%). Guesses until the pilot.
      {"flash_attempts", 1.15}, {"flash_proof_attempts", 1.6}};
  std::string scenario = "mid";
  for (int i = 2; i < argc; ++i) {
    std::string s = argv[i]; auto e = s.find('=');
    if (s == "low" || s == "mid" || s == "high") { scenario = s; continue; }
    if (e == std::string::npos) { std::cerr << "unknown argument " << s << "\n"; return 2; }
    std::string k = s.substr(0, e); double v = std::stod(s.substr(e + 1));
    if (auto d = k.find('.'); d != std::string::npos && m.count(k.substr(0, d))) {   // e.g. T2.attempts=4
      auto& x = m[k.substr(0, d)]; std::string f = k.substr(d + 1);
      if (f == "in") x.in = v; else if (f == "out") x.out = v; else if (f == "attempts") x.attempts = v;
      else if (f == "gate_runs") x.gate_runs = v; else if (f == "gate_min") x.gate_min = v;
      else { std::cerr << "unknown field " << k << "\n"; return 2; }
    } else if (a.count(k)) a[k] = v;
    else { std::cerr << "unknown assumption " << k << "\n"; return 2; }
  }
  if (scenario == "low") { a["req"] *= 0.6; a["out"] *= 0.7; a["gate"] *= 0.6; a["hit"] = std::max(a["hit"], 0.90); }
  if (scenario == "high") { a["req"] *= 1.8; a["out"] *= 1.5; a["gate"] *= 2.0; a["hit"] = std::min(a["hit"], 0.75); }
  if (a["cores"] <= 0) {
    double local = 0; for (auto& [k, s] : c.slots) if (k != "agent") local += s.count * std::max(1, s.cores);
    double w = 0; if (!c.workers_file.empty()) for (auto& x : load_workers(c.workers_file)) w += x.slots * a["gate_cores"];
    a["cores"] = std::max(1.0, local + w);
  }
  Graph g = load_graph(c);
  struct Row { int64_t tasks = 0; double req = 0, in = 0, out = 0, core_h = 0, api_s = 0, esc = 0; std::string tier; };
  std::map<std::string, Row> rows;
  auto tier_of = [&](const json& t) { json x = t; x["_attempt"] = 1; return role_tier(x, c); };
  for (auto& t : g.t) {
    std::string role = t.value("role", "implementer"), cls = role == "prover" ? t.value("proof_tier", std::string("T3")) : role;
    if (!m.count(cls)) cls = "implementer";
    const M& x = m[cls]; Row& r = rows[cls];
    std::string tier = tier_of(t);
    double att = x.attempts;
    if (tier == "flash") att *= (cls == "T1" || cls == "T2") ? a["flash_proof_attempts"] : a["flash_attempts"];
    double req = t.value("turns_budget", 30) * att * a["req"];
    // Attempts are geometric with mean att, so a share p^(escalate_at-1) of them (p = 1 - 1/att)
    // runs from attempt escalate_at onwards, on escalate_tier.
    double esc = tier != c.escalate_tier ? std::pow(1 - 1 / att, c.escalate_at - 1) : 0;
    r.tasks++; r.req += req; r.in += req * x.in; r.out += req * x.out * a["out"]; r.esc = esc;
    r.core_h += att * x.gate_runs * x.gate_min * a["gate"] / 60;
    double tok_s = tier == "flash" ? a["tok_s_flash"] : a["tok_s_frontier"];
    r.api_s += req * (a["ttft"] + x.out * a["out"] / tok_s);
    r.tier = esc > 0 ? tier + "->" + c.escalate_tier : tier;
  }
  auto price = [&](const std::string& tier, double in, double out) {
    std::string k = tier == "flash" ? "flash" : "frontier";
    return (in * a["hit"] * a["p_hit_" + k] + in * (1 - a["hit"]) * a["p_in_" + k] + out * a["p_out_" + k]) / 1e6;
  };
  auto usd_row = [&](const Row& r) {   // split by escalation: attempts before escalate_at on the role's tier
    std::string base = r.tier.substr(0, r.tier.find("->"));
    if (r.tier.find("->") == std::string::npos) return price(base, r.in, r.out);
    return (1 - r.esc) * price(base, r.in, r.out) + r.esc * price(c.escalate_tier, r.in, r.out);
  };
  double REQ = 0, IN = 0, OUT = 0, CORE_H = 0, API_S = 0, USD = 0, TOOL_S = 0;
  std::cout << std::format("ESTIMATE, {} case (assumptions, not measurements; AGENTS.md section 8)\n\n", scenario)
            << "| class | tasks | tier | requests | input tokens | output tokens | gate core-h | API US$ |\n|---|---|---|---|---|---|---|---|\n";
  for (auto& [k, r] : rows) {
    double usd = usd_row(r);
    std::cout << std::format("| {} | {} | {} | {:.0f}k | {:.2f}B | {:.0f}M | {:.0f} | {:.0f} |\n", k, r.tasks, r.tier, r.req / 1e3,
                             r.in / 1e9, r.out / 1e6, r.core_h, usd);
    REQ += r.req; IN += r.in; OUT += r.out; CORE_H += r.core_h; API_S += r.api_s; USD += usd; TOOL_S += r.req * a["tool_s"];
  }
  std::cout << std::format("| **total** | {} | | **{:.0f}k** | **{:.1f}B** | **{:.2f}B** | **{:.0f}** | **{:.0f}** |\n\n",
                           g.t.size(), REQ / 1e3, IN / 1e9, OUT / 1e9, CORE_H, USD);
  // Every class on one tier, for comparison.
  std::cout << std::format("API cost for this config: ${:.0f} (compare configs with PBSD_CONFIG=config.cloud-frontier.json / -escalate.json)\n", USD);
  const double gate_cs = CORE_H * 3600, cores = a["cores"], gate_job_s = gate_cs / a["gate_cores"];
  const double burst_share = cores > a["pc_cores"] ? (cores - a["pc_cores"]) / cores : 0;   // gate work done on cloud cores
  const double agent_s = API_S + TOOL_S + gate_job_s;
  Cp cp = critical_path(g);
  const double cp_s = cp.turns * 1.5 * (a["ttft"] + 6000 / a["tok_s_frontier"] + a["tool_s"]) + cp.tasks * 3600;
  std::cout << std::format("gates: {:.0f} core-h on {:.0f} cores ({:.0f} on the PC) = {:.1f} d; cloud burst share {:.0f}% at ${}/core-h: ${:.0f}\n",
                           CORE_H, cores, std::min(cores, a["pc_cores"]), gate_cs / cores / 86400, 100 * burst_share, a["core_usd"],
                           CORE_H * burst_share * a["core_usd"] * 1.25);
  std::cout << std::format("critical path: {} tasks, {} budgeted turns -> about {:.1f} d (1.5 attempts per task, 1 h of gates per task)\n\n",
                           cp.tasks, cp.turns, cp_s / 86400);
  std::cout << "| agents | agent-bound | gate-bound | API cap | wall clock | binding | requests in flight | output tok/min |\n|---|---|---|---|---|---|---|---|\n";
  for (int n : {16, 24, 32, 48, 64, 96, 128, 192, 256}) {
    double b_agent = agent_s / n, b_gate = gate_cs / cores, b_api = a["api_conc"] > 0 ? API_S / a["api_conc"] : 0;
    double wall = std::max({b_agent, b_gate, b_api, cp_s});
    const char* bind = wall == b_agent ? "agents" : wall == b_gate ? "gate CPU" : wall == b_api ? "API concurrency" : "critical path";
    auto d = [](double s) { return std::format("{:.1f} d", s / 86400); };
    std::cout << std::format("| {} | {} | {} | {} | **{}** | {} | {:.0f} | {:.0f}k |\n", n, d(b_agent), d(b_gate),
                             b_api > 0 ? d(b_api) : std::string("-"), d(wall), bind, API_S / wall, OUT / wall * 60 / 1e3);
  }
  double wall_best = std::max({gate_cs / cores, cp_s, a["api_conc"] > 0 ? API_S / a["api_conc"] : 0.0});
  const double n_sat = agent_s / wall_best;
  double days = wall_best / 86400 * 1.3;   // at saturation, +30% for fix rounds and queueing
  double power = a["pc_watts"] / 1000 * 24 * days * a["aud_kwh"] / a["usd_aud"];
  double total = USD + CORE_H * burst_share * a["core_usd"] * 1.25 + power;
  std::cout << std::format("\nagents to saturate the binding resource: {:.0f} (run about 15% more)\n"
                           "run total at saturation (API + cloud burst + PC electricity, about {:.0f} days): US${:.0f} = A${:.0f}\n",
                           n_sat, days, total, total * a["usd_aud"]);
  return 0;
}
}  // namespace pbsd
