// One task end to end: tmpfs sparse worktree, layered prompt, tool-calling turns with DeepSeek
// Flash, WIP checkpoints, commit to t/<id>, acceptance gate via a NUMA-pinned watchdogged job.
#include "common.hpp"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <thread>

namespace fs = std::filesystem;
namespace pbsd {
namespace {
std::string slurp(const fs::path& p, size_t max = 400000) {
  std::ifstream f(p, std::ios::binary); std::string s((std::istreambuf_iterator<char>(f)), {});
  if (s.size() > max) s.resize(max); return s;
}
std::string sh(const std::string& cwd, const std::vector<std::string>& a, int to = 600) { return run_job(a, -1, to, cwd, 0).tail; }
bool owned(const json& t, const std::string& rel) {
  if (rel.find("..") != std::string::npos) return false;
  for (auto& o : t["owns"]) { std::string p = o; if (!p.empty() && p.back() == '*') p.pop_back(); if (rel.rfind(p, 0) == 0) return true; }
  return false;
}
// Which slot class and NUMA node a gate uses.
std::pair<std::string, int> gate_slot(const std::string& role, const Config& c) {
  std::string k = role == "prover" ? "proof" : role == "integrator" ? "qemu" : "compile";
  return {k, c.slots.at(k).node};
}
int gate_timeout(const json& t, const std::string& kind, const Config& c) {
  if (kind == "proof") return 60 * t.value("proof_budget_minutes", 60);
  return kind == "qemu" ? c.qemu_watchdog_s : c.build_watchdog_s;
}
json tools_schema() {
  auto fn = [](const char* n, const char* d, json props, std::vector<std::string> req) {
    return json{{"type", "function"}, {"function", {{"name", n}, {"description", d},
      {"parameters", {{"type", "object"}, {"properties", props}, {"required", req}}}}}};
  };
  return json::array({
    fn("read_file", "Read a file in the worktree", {{"path", {{"type", "string"}}}}, {"path"}),
    fn("write_file", "Write a file inside this task's owned paths", {{"path", {{"type", "string"}}}, {"content", {{"type", "string"}}}}, {"path", "content"}),
    fn("list_dir", "List a directory in the worktree", {{"path", {{"type", "string"}}}}, {"path"}),
    fn("run_gate", "Run this task's acceptance gate (build, tests incl. fault injection, proof); returns the log tail", json::object(), {}),
    fn("post_message", "Post to the message board (question, contract_change, review, failure, handoff)",
       {{"kind", {{"type", "string"}}}, {"body", {{"type", "string"}}}}, {"kind", "body"}),
    fn("submit", "Finish the task when the gate passes", {{"summary", {{"type", "string"}}}}, {"summary"})});
}
}  // namespace

TaskOutcome run_task(const json& t, Api& api, const Config& c, Db& db, int slot) {
  TaskOutcome out{Result::failed, {}};
  const std::string id = t["id"], role = t["role"], branch = "t/" + id;
  const int attempt = t.value("_attempt", 1);
  // 1. Worktree in tmpfs (PBSD_WT, default /run/pbsd/wt), sparse to owned paths plus the contract headers.
  const char* wtroot = getenv("PBSD_WT");
  fs::path wt = fs::path(wtroot ? wtroot : "/run/pbsd/wt") / id;
  fs::create_directories(wt.parent_path());
  if (!fs::exists(wt)) {
    sh(c.repo, {"git", "worktree", "prune"});
    // Resume from wip/<id> if a checkpoint exists, else from t/<id>, else from the package branch.
    std::string base = sh(c.repo, {"git", "rev-parse", "--verify", "-q", "wip/" + id}).empty() ? "" : "wip/" + id;
    if (base.empty() && !sh(c.repo, {"git", "rev-parse", "--verify", "-q", branch}).empty()) base = branch;
    if (base.empty()) base = "wp/" + t["wp"].get<std::string>();
    sh(c.repo, {"git", "worktree", "add", "--no-checkout", "-B", branch, wt.string(), base});
    std::vector<std::string> sp{"git", "sparse-checkout", "set", "--no-cone", "/pbsd/kernel/include/"};
    for (auto& o : t["owns"]) sp.push_back("/" + o.get<std::string>());
    sh(wt.string(), sp); sh(wt.string(), {"git", "checkout"});
  }
  // 2. Layered prompt: global, role, package contracts, unit source, then the variable tail (cache-friendly).
  fs::path plan = fs::path(c.repo) / "docs/plan";
  // Full prompt (API, large context) or compact prompt (local model, ctx_per_slot tokens): the
  // compact one sends the role template, the task and file lists; the agent reads what it needs.
  const size_t ctx_chars = size_t(c.ctx_per_slot) * 4;
  const bool compact = c.backend != "api" && c.compact_prompt;
  std::string global, agents;
  if (compact) {   // AGENTS.md section 6: the shared prefix and this role's template only
    std::string all = slurp(plan / "AGENTS.md", 400000);
    auto block = [&](const std::string& marker) {
      auto a = all.find(marker); if (a == std::string::npos) return std::string();
      return all.substr(a, all.find("```", a) - a);
    };
    global = block("You are one agent in the PBSD build.");
    agents = block("Role: " + role + ".");
  } else {
    global = slurp(plan / "PBSD_PLAN.md", 120000);
    agents = slurp(plan / "AGENTS.md", 60000);
  }
  std::string contracts; for (auto& e : fs::recursive_directory_iterator(wt / "pbsd/kernel/include", fs::directory_options::skip_permission_denied))
    if (e.is_regular_file()) contracts += compact ? e.path().lexically_relative(wt).string() + "\n"
                                                  : "// " + e.path().string() + "\n" + slurp(e.path(), 40000);
  std::string source; if (t.contains("source")) for (auto& s : (t["source"].is_array() ? t["source"] : json::array({t["source"]})))
    if (s.is_string() && fs::exists(fs::path(c.repo) / s.get<std::string>())) source += slurp(fs::path(c.repo) / s.get<std::string>(), compact ? ctx_chars / 4 : 200000);
  json msgs = json::array({
    {{"role", "system"}, {"content", (compact ? "PBSD agent rules:\n" : "PBSD plan:\n") + global}},
    {{"role", "system"}, {"content", "Agent rules and role templates:\n" + agents + "\nYour role: " + role}},
    {{"role", "system"}, {"content", (compact ? "Frozen contract headers (read_file to open):\n" : "Frozen contracts:\n") + contracts.substr(0, compact ? ctx_chars / 16 : 300000)}},
    {{"role", "user"}, {"content", "Unit source:\n" + source}},
    {{"role", "user"}, {"content", std::string("Task:\n") + t.dump(2) + "\nAttempt " + std::to_string(attempt) +
      (attempt >= 5 ? ". This task has failed repeatedly: if it is too large, post a handoff message proposing a split." : "")}}});
  // Conversation state saved for resume (fresh context from attempt 3).
  fs::path state = fs::path(c.state_dir) / "conv" / (id + ".json");
  fs::create_directories(state.parent_path());
  if (attempt < 3 && fs::exists(state)) { json s = json::parse(slurp(state), nullptr, false); if (s.is_array()) msgs = s; }

  auto [kind, node] = gate_slot(role, c);
  double last_ck = now(), turn_start = now();
  bool gate_ok = false;
  json tools = tools_schema();
  for (int turn = 0; turn < c.max_turns; ++turn) {
    db.heartbeat(id, c.lease_s);
    Usage u;
    // Leave room for the reply inside this slot's context.
    if (int n = compact_history(msgs, size_t(c.ctx_per_slot - c.max_tokens - 1024) * (c.backend == "api" ? 8 : 1)))
      db.event(id, "compacted", n);
    auto r = api.chat(msgs, tools, u, role, slot);
    out.usage.push_back(u);
    if (!r) { db.event(id, "api_error", r.error()); if (r.error().starts_with("402")) throw std::runtime_error(r.error()); return out; }
    json m = (*r)["choices"][0]["message"];
    msgs.push_back(m);
    if (!m.contains("tool_calls") || m["tool_calls"].empty()) { msgs.push_back({{"role", "user"}, {"content", "Use the tools; call submit when the gate passes."}}); continue; }
    bool submitted = false;
    for (auto& call : m["tool_calls"]) {
      std::string fnm = call["function"]["name"];
      json a = json::parse(call["function"].value("arguments", "{}"), nullptr, false);
      std::string res;
      if (fnm == "read_file") res = slurp(wt / a.value("path", ""), c.backend == "api" ? 200000 : size_t(c.tool_result_chars));
      else if (fnm == "list_dir") { for (auto& e : fs::directory_iterator(wt / a.value("path", "."), fs::directory_options::skip_permission_denied)) res += e.path().filename().string() + "\n"; }
      else if (fnm == "write_file") {
        std::string p = a.value("path", "");
        if (!owned(t, p)) res = "refused: " + p + " is outside this task's owned paths";
        else { fs::create_directories((wt / p).parent_path()); std::ofstream(wt / p) << a.value("content", ""); res = "written"; }
      } else if (fnm == "run_gate") {
        while (mem_used_frac() > c.ram_pause) std::this_thread::sleep_for(std::chrono::seconds(10));  // memory throttle
        // Gate script per role; must include the fault-injection tests in task.accept.
        fs::path gate = fs::path(c.repo) / "tools/gates" / (role + ".sh");
        auto g = run_job({"sh", gate.string(), id, wt.string()}, node, gate_timeout(t, kind, c), wt.string());
        gate_ok = g.code == 0;
        db.event(id, "gate", json{{"code", g.code}, {"timeout", g.timed_out}});
        std::string tail = g.tail.size() > size_t(c.tool_result_chars) ? g.tail.substr(g.tail.size() - c.tool_result_chars) : g.tail;
        res = std::format("exit {}{}\n{}", g.code, g.timed_out ? " (watchdog timeout)" : "", tail);
      } else if (fnm == "post_message") {
        db.event(id, "msg:" + a.value("kind", "note"), a.value("body", "")); res = "posted";
      } else if (fnm == "submit") {
        submitted = true; res = gate_ok ? "accepted" : "rejected: gate has not passed on the current tree";
      } else res = "unknown tool";
      msgs.push_back({{"role", "tool"}, {"tool_call_id", call["id"]}, {"content", res}});
    }
    // 3. Atomic commit at turn end; WIP checkpoint to the HDD-backed repo every checkpoint_s.
    sh(wt.string(), {"git", "add", "-A"});
    sh(wt.string(), {"git", "-c", "user.name=pbsd-agent", "-c", "user.email=agent@pbsd.local", "commit", "-q", "-m", id + " turn " + std::to_string(turn)});
    if (now() - last_ck > c.checkpoint_s) {
      sh(wt.string(), {"git", "update-ref", "refs/heads/wip/" + id, "HEAD"});
      std::ofstream(state) << msgs.dump(); last_ck = now();
    }
    if (submitted && gate_ok) { out.r = Result::passed; break; }
    if (now() - turn_start > c.agent_watchdog_s * c.max_turns) break;   // whole-task watchdog
  }
  sh(wt.string(), {"git", "update-ref", "refs/heads/wip/" + id, "HEAD"});
  std::ofstream(state) << msgs.dump();
  if (out.r == Result::passed) { sh(c.repo, {"git", "worktree", "remove", "--force", wt.string()}); fs::remove(state); }
  return out;
}
}  // namespace pbsd
