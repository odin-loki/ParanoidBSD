// One task end to end (AGENTS.md sections 2, 5 and 9):
//  1. a tmpfs worktree on t/<id>, cut from wp/<WP> after wp/<WP> has merged the integrated branch, so
//     a task sees every dependency's output; big reference trees stay out (read_file "ref:..." instead);
//  2. a layered, cache-friendly prompt plus the messages addressed to this task;
//  3. tool-calling turns on the role's model tier; writes only inside the task's owned paths;
//  4. the acceptance gate on a committed SHA, in a sandbox without secrets (or on a cloud worker);
//     any write after a passing gate invalidates it;
//  5. on success a deterministic merge: t/<id> into wp/<WP>; integrators and -IF tasks then merge
//     wp/<WP> into the integration branch. Reviewers return a verdict that can reopen the unit.
#include "common.hpp"
#include <algorithm>
#include <cctype>
#include <condition_variable>
#include <filesystem>
#include <format>
#include <fstream>
#include <regex>
#include <set>
#include <sstream>
#include <thread>

extern char** environ;
namespace fs = std::filesystem;
namespace pbsd {
namespace {
std::string slurp(const fs::path& p, size_t max = 400000) {
  std::ifstream f(p, std::ios::binary);
  std::string s((std::istreambuf_iterator<char>(f)), {});
  if (s.size() > max) s.resize(max);
  return s;
}
std::string sh(const std::string& cwd, const std::vector<std::string>& a, int to = 600) { return run_job(a, -1, to, cwd, 0).tail; }
std::string trim(std::string s) {
  while (!s.empty() && std::isspace((unsigned char)s.back())) s.pop_back();
  size_t i = 0; while (i < s.size() && std::isspace((unsigned char)s[i])) ++i;
  return s.substr(i);
}
std::vector<std::string> expand_braces(const std::string& s) {
  auto a = s.find('{'); if (a == std::string::npos) return {s};
  auto b = s.find('}', a); if (b == std::string::npos) return {s};
  std::vector<std::string> out; std::string mid = s.substr(a + 1, b - a - 1), part; std::istringstream is(mid);
  while (std::getline(is, part, ','))
    for (auto& r : expand_braces(s.substr(0, a) + trim(part) + s.substr(b + 1))) out.push_back(r);
  return out;
}
// A relative path that stays inside root (no absolute paths, no "..", no symlink escape).
std::optional<fs::path> in_tree(const fs::path& root, const std::string& rel) {
  if (rel.empty() || rel[0] == '/' || rel.find("..") != std::string::npos || rel.find('\0') != std::string::npos) return std::nullopt;
  std::error_code ec;
  fs::path r = fs::weakly_canonical(root, ec), p = fs::weakly_canonical(root / rel, ec);
  if (ec) return std::nullopt;
  auto [ra, pa] = std::mismatch(r.begin(), r.end(), p.begin(), p.end());
  if (ra != r.end()) return std::nullopt;
  return p;
}
bool owned(const json& t, const std::string& rel) {
  if (rel.empty() || rel[0] == '/' || rel.find("..") != std::string::npos) return false;
  for (auto& p : owns_list(t)) {
    if (p.ends_with("/") ? rel.rfind(p, 0) == 0 : rel == p) return true;
    // "dir/name.*" owns every file of that stem in that directory
    if (p.ends_with(".*") && rel.rfind(p.substr(0, p.size() - 1), 0) == 0 && rel.find('/', p.size() - 1) == std::string::npos) return true;
  }
  return false;
}
// Which slot class and NUMA node a gate uses.
std::pair<std::string, int> gate_slot(const std::string& role, const Config& c) {
  std::string k = role == "prover" ? "proof" : role == "integrator" ? "qemu" : "compile";
  return {k, c.slots.count(k) ? c.slots.at(k).node : -1};
}
int gate_timeout(const json& t, const std::string& kind, const Config& c) {
  if (kind == "proof") return 60 * t.value("proof_budget_minutes", 60);
  return kind == "qemu" ? c.qemu_watchdog_s : c.build_watchdog_s;
}
json tools_schema(const std::string& role, bool lean = false) {
  auto fn = [](const char* n, const char* d, json props, std::vector<std::string> req) {
    return json{{"type", "function"}, {"function", {{"name", n}, {"description", d},
      {"parameters", {{"type", "object"}, {"properties", props}, {"required", req}}}}}};
  };
  json str = {{"type", "string"}}, num = {{"type", "integer"}};
  json submit_props = {{"summary", str}};
  std::vector<std::string> submit_req{"summary"};
  if (role == "reviewer") {
    submit_props["verdict"] = {{"type", "string"}, {"enum", {"approve", "changes"}}};
    submit_props["findings"] = {{"type", "array"}, {"items", {{"type", "object"}, {"properties",
      {{"target", {{"type", "string"}, {"enum", {"spec", "impl", "test", "proof"}}}}, {"file", str}, {"line", num}, {"issue", str}}}}}};
    submit_req.push_back("verdict");
  }
  return json::array({
    fn("read_file", "Read a file: a worktree path, or ref:<tree>/<path> for a read-only reference tree (ref:linux/..., ref:wine/...). Optional line range.",
       {{"path", str}, {"start_line", num}, {"end_line", num}}, {"path"}),
    fn("list_dir", "List a directory (worktree path or ref:<tree>/<path>)", {{"path", str}}, {"path"}),
    fn("grep", "Search files under a path for a regular expression (worktree or ref:<tree>/<path>); returns up to 200 matching lines",
       {{"pattern", str}, {"path", str}}, {"pattern", "path"}),
    fn("write_file", "Create or replace a file inside this task's owned paths; append=true adds to the end (write a large file in parts)",
       {{"path", str}, {"content", str}, {"append", {{"type", "boolean"}}}}, {"path", "content"}),
    fn("edit_file", "Replace one exact, unique occurrence of old_text with new_text in an owned file",
       {{"path", str}, {"old_text", str}, {"new_text", str}}, {"path", "old_text", "new_text"}),
    fn("run_gate", "Commit the owned files and run this task's acceptance gate (build, tests incl. fault injection, proofs); returns the log tail. Any later write invalidates the result.",
       json::object(), {}),
    fn("post_message", "Post a typed message. to: a task id, or spec|impl|test|proof|if (this unit's or package's task). kind: question, contract_change_request, review_findings, test_failure, proof_failure, handoff",
       {{"to", str}, {"kind", str}, {"body", str}}, {"to", "kind", "body"}),
    fn("read_messages", "Read the messages addressed to this task or its package", json::object(), {}),
    fn("lean_prove", "Prove one Lean 4 theorem with PRISM and the local Goedel prover model (prism prove). The theorem must be in an owned .lean file with a whole-proof `sorry`. On success the proof is written into the file. Pass theorem \"*\" to try EVERY theorem with a sorry in the project in one call (the model is loaded once, so this is much cheaper than one call per theorem); otherwise state small lemmas and prove them one at a time.",
       {{"file", str}, {"theorem", str}}, {"file", "theorem"}),
    fn("submit", role == "reviewer" ? "Finish the review: verdict approve, or changes with findings (each names the task to reopen)"
                                    : "Finish the task; accepted only if the gate passed on the current tree", submit_props, submit_req)});
}
// Task source as text. Formats: "hbsd/src/...c" (repository path), "linux:<path>" (file or directory
// in ref_roots["linux"]), "<tree>:<dir> part k/N" (slice k of N: the directory's .c/.h/.S files sorted
// by path, concatenated, cut into N equal line ranges). Descriptive sources return the text only.
std::string load_source(const json& t, const Config& c, size_t max_chars) {
  if (!t.contains("source") || !t["source"].is_string()) return "";
  std::string s = t["source"];
  static const std::regex part_re(R"(^(.*) part (\d+)/(\d+)$)");
  std::smatch m; int k = 0, n = 0;
  std::string spec = s;
  if (std::regex_match(s, m, part_re)) { spec = m[1]; k = std::stoi(m[2]); n = std::stoi(m[3]); }
  fs::path root = c.repo; std::string rel = spec;
  if (auto colon = spec.find(':'); colon != std::string::npos && spec.find('/') > colon) {
    auto it = c.ref_roots.find(spec.substr(0, colon));
    if (it == c.ref_roots.end()) return "(source tree '" + spec.substr(0, colon) + "' is not configured in ref_roots: " + s + ")";
    root = it->second; rel = spec.substr(colon + 1);
  }
  auto p = in_tree(root, rel);
  if (!p || !fs::exists(*p)) return "(source not found on disk: " + s + "; find the files with list_dir/grep and record them in the spec)";
  std::vector<fs::path> files;
  if (fs::is_regular_file(*p)) files.push_back(*p);
  else for (auto& e : fs::recursive_directory_iterator(*p, fs::directory_options::skip_permission_denied)) {
    auto x = e.path().extension().string();
    if (e.is_regular_file() && (x == ".c" || x == ".h" || x == ".S")) files.push_back(e.path());
  }
  std::sort(files.begin(), files.end());
  std::vector<std::pair<fs::path, std::vector<std::string>>> lines; size_t total = 0;
  for (auto& f : files) { std::ifstream in(f); std::vector<std::string> v; std::string l; while (std::getline(in, l)) v.push_back(l); total += v.size(); lines.emplace_back(f, std::move(v)); }
  size_t lo = 0, hi = total;
  if (n > 0 && k >= 1 && k <= n) { lo = total * size_t(k - 1) / size_t(n); hi = total * size_t(k) / size_t(n); }
  std::string out = std::format("Source {} (lines {}-{} of {} over {} files)\n", s, lo + 1, hi, total, files.size());
  size_t at = 0;
  for (auto& [f, v] : lines) {
    size_t a = std::max(lo, at), b = std::min(hi, at + v.size());
    if (a < b) {
      out += std::format("// ---- {} lines {}-{}\n", fs::relative(f, root).string(), a - at + 1, b - at);
      for (size_t i = a; i < b && out.size() < max_chars; ++i) out += v[i - at] + "\n";
    }
    at += v.size();
    if (out.size() >= max_chars) { out += "\n[source truncated; read the rest with read_file]\n"; break; }
  }
  return out;
}
std::string rev(const Config& c, const std::string& ref) {
  auto r = run_job({"git", "rev-parse", "--verify", "-q", ref + "^{commit}"}, -1, 60, c.repo, 0);
  return r.code == 0 ? trim(r.tail) : "";
}
// Unit id prefix of a task: P1-02-U001-IMPL -> P1-02-U001.
std::string unit_of(const std::string& id) { auto d = id.rfind('-'); return d == std::string::npos ? id : id.substr(0, d); }
std::string resolve_to(const json& t, const std::string& to) {
  const std::string id = t["id"], wp = t.value("wp", "");
  if (to == "spec" || to == "impl" || to == "test" || to == "proof" || to == "review") {
    std::string up = to; std::transform(up.begin(), up.end(), up.begin(), ::toupper); return unit_of(id) + "-" + up;
  }
  if (to == "if" || to == "decomposer") return wp + "-IF";
  return to;
}
}  // namespace

std::vector<std::string> owns_list(const json& t) {
  std::vector<std::string> raw, out;
  if (!t.contains("owns")) return out;
  if (t["owns"].is_array()) { for (auto& o : t["owns"]) if (o.is_string()) raw.push_back(o); }
  else if (t["owns"].is_string()) {   // comma-separated, commas inside {...} kept
    std::string s = t["owns"], cur; int depth = 0;
    for (char ch : s) { if (ch == '{') ++depth; if (ch == '}') --depth; if (ch == ',' && depth == 0) { raw.push_back(cur); cur.clear(); } else cur += ch; }
    raw.push_back(cur);
  }
  for (auto& r : raw) {
    std::string s = trim(r);
    if (s.empty() || s == "-" || s.find(' ') != std::string::npos || s.find('<') != std::string::npos) continue;
    for (auto e : expand_braces(s)) { if (e.ends_with("*") && !e.ends_with(".*")) e.pop_back(); if (!e.empty()) out.push_back(e); }
  }
  return out;
}

std::string role_tier(const json& t, const Config& c) {
  if (t.contains("tier") && t["tier"].is_string()) return t["tier"];
  if (t.value("_attempt", 1) >= c.escalate_at) return c.escalate_tier;
  auto it = c.role_tier.find(t.value("role", ""));
  return it != c.role_tier.end() ? it->second : c.default_tier;
}

std::string merge_branch(const Config& c, const std::string& dst, const std::string& src, const std::string& msg) {
  static std::mutex mm; std::lock_guard g(mm);
  const std::string d = rev(c, "refs/heads/" + dst), s = rev(c, "refs/heads/" + src);
  if (s.empty()) return "no branch " + src;
  auto git = [&](std::vector<std::string> a) { a.insert(a.begin(), {"git", "-c", "user.name=pbsd-runner", "-c", "user.email=runner@pbsd.local"}); return run_job(a, -1, 600, c.repo, 0); };
  if (d.empty()) { auto r = git({"update-ref", "refs/heads/" + dst, s, ""}); return r.code == 0 ? "" : "update-ref failed: " + r.tail; }
  if (git({"merge-base", "--is-ancestor", s, d}).code == 0) return "";                       // already merged
  std::string target;
  if (git({"merge-base", "--is-ancestor", d, s}).code == 0) target = s;                    // fast-forward
  else {
    auto mt = git({"merge-tree", "--write-tree", "--messages", d, s});
    if (mt.code != 0) return "merge conflict merging " + src + " into " + dst + ":\n" + mt.tail.substr(0, 4000);
    std::string tree = trim(mt.tail.substr(0, mt.tail.find('\n')));
    auto ct = git({"commit-tree", tree, "-p", d, "-p", s, "-m", msg});
    if (ct.code != 0) return "commit-tree failed: " + ct.tail;
    target = trim(ct.tail);
  }
  auto u = git({"update-ref", "refs/heads/" + dst, target, d});                             // compare-and-swap
  return u.code == 0 ? "" : "update-ref failed: " + u.tail;
}

// ---- gate admission. Without it, hundreds of agents would start hundreds of builds and proofs at once.
namespace {
std::mutex g_gm; std::condition_variable g_gcv;
std::map<std::string, int> g_gbusy; int g_burst_busy = 0;
std::map<std::string, int> g_wbusy; std::vector<Worker> g_workers; double g_workers_at = 0;
}
std::vector<Worker> load_workers(const std::string& path) {
  std::vector<Worker> w; std::ifstream f(path); std::string line;
  while (std::getline(f, line)) {
    if (line.empty() || line[0] == '#') continue;
    std::istringstream is(line); Worker x; std::string flag;
    if (is >> x.host >> x.slots && x.slots > 0) { while (is >> flag) if (flag == "kvm") x.kvm = true; w.push_back(x); }
  }
  return w;
}
GateLease gate_acquire(const std::string& kind, const Config& c) {
  std::unique_lock l(g_gm);
  const int cap = c.slots.count(kind) ? c.slots.at(kind).count : 1;
  for (;;) {
    if (g_gbusy[kind] < cap) { ++g_gbusy[kind]; return {kind, false, ""}; }
    if (!c.workers_file.empty()) {
      if (now() - g_workers_at > 30) { g_workers = load_workers(c.workers_file); g_workers_at = now(); }
      const Worker* best = nullptr; double load = 1.0;
      for (auto& w : g_workers) {
        if (kind == "qemu" && !w.kvm) continue;   // QEMU without KVM is far too slow for a gate
        double x = double(g_wbusy[w.host]) / w.slots; if (x < load) { load = x; best = &w; }
      }
      if (best) { ++g_wbusy[best->host]; return {kind, true, best->host}; }
    }
    if (!c.burst_cmd.empty() && g_burst_busy < c.burst_slots) { ++g_burst_busy; return {kind, true, ""}; }
    g_gcv.wait_for(l, std::chrono::seconds(5));
  }
}
void gate_release(const GateLease& g) {
  { std::lock_guard l(g_gm); if (!g.host.empty()) --g_wbusy[g.host]; else if (g.burst) --g_burst_busy; else --g_gbusy[g.kind]; }
  g_gcv.notify_all();
}
std::string gate_status(const Config& c) {
  std::lock_guard l(g_gm);
  std::string s = "  gates";
  for (const char* k : {"compile", "qemu", "proof"})
    if (c.slots.count(k) && c.slots.at(k).count > 0) s += std::format("  {} {}/{}", k, g_gbusy[k], c.slots.at(k).count);
  if (c.burst_slots) s += std::format("  burst {}/{}", g_burst_busy, c.burst_slots);
  int busy = 0, all = 0;
  for (auto& w : g_workers) { busy += g_wbusy[w.host]; all += w.slots; }
  if (!g_workers.empty()) s += std::format("  workers {} ({}/{} slots)", g_workers.size(), busy, all);
  return s + "\n";
}

// lean_prove: `prism prove FILE THEOREM --write --allow-exec`. PRISM runs best-first tactic search with the
// prover model on FORGE (PRISM_PROVER_FPK), validates every candidate, checks the axioms and runs
// `lake build`; only then does it write the proof. Exit 0 proved, 1 not proved, 3 not run, 2 error.
// At most Config::prover_slots `prism prove` runs at once: each holds its own copy of the model on the GPU.
struct ProverSlot {
  static std::mutex& mu() { static std::mutex m; return m; }
  static std::condition_variable& cv() { static std::condition_variable v; return v; }
  static int& used() { static int n = 0; return n; }
  // weight 1 = one 8B prover process (about 9 GB of the 24 GB GPU: two fit side by side). A weight of the whole limit = exclusive use of the GPU,
  // which the 32B escalation model needs (about 19 GB); new single users wait while an exclusive request is pending.
  static int& want() { static int n = 0; return n; }
  explicit ProverSlot(int limit, int weight = 1) : lim_(std::max(1, limit)), w_(std::clamp(weight, 1, std::max(1, limit))) {
    std::unique_lock lk(mu());
    const bool excl = w_ > 1 || lim_ == 1;
    if (w_ == lim_ && lim_ > 1) ++want();
    cv().wait(lk, [&] { return (w_ == lim_ || want() == 0) && used() + w_ <= lim_; });
    if (w_ == lim_ && lim_ > 1) --want();
    (void)excl;
    used() += w_;
  }
  ~ProverSlot() { { std::lock_guard lk(mu()); used() -= w_; } cv().notify_all(); }
  int lim_, w_;
  ProverSlot(const ProverSlot&) = delete; ProverSlot& operator=(const ProverSlot&) = delete;
};

// Link the shared Lake packages into the model project that holds `file`, so neither `prism prove` (whose scratch copy follows the
// link) nor `lake build` fetches Mathlib into every worktree. .lake is ignored by git, so the link is never committed.
void link_lean_packages(const Config& c, const fs::path& file) {
  if (c.lean_packages.empty() || !fs::is_directory(c.lean_packages)) return;
  std::error_code ec;
  for (fs::path d = fs::absolute(file, ec).parent_path(); !d.empty() && d != d.root_path(); d = d.parent_path()) {
    if (fs::exists(d / "lakefile.lean", ec) || fs::exists(d / "lakefile.toml", ec)) {
      if (!fs::exists(d / ".lake" / "packages", ec) && !fs::is_symlink(d / ".lake" / "packages", ec)) {
        fs::create_directories(d / ".lake", ec);
        fs::create_directory_symlink(fs::absolute(c.lean_packages, ec), d / ".lake" / "packages", ec);
      }
      return;
    }
  }
}

std::string lean_prove(const Config& c, const json& t, const fs::path& wt, const fs::path& file, const std::string& theorem, Db& db) {
  static const std::regex name_re(R"(^[A-Za-z_][A-Za-z0-9_.']*$)");
  // theorem "*" proves every theorem with a sorry in the model project in ONE prism process: the model is loaded and warmed up once
  // (about 35 s plus the Lean check) instead of once per theorem.
  const bool batch = theorem == "*";
  if (!batch && !std::regex_match(theorem, name_re)) return "refused: bad theorem name";
  // Fifth rehearsal (7 Nov 2026): the agent called lean_prove on the same two hard statements 2-3 times each (every call is the 8B and then the 32B,
  // 7-10 minutes) and spent 90 minutes for one proof. Each theorem gets two calls per task; after that the agent must prove it by hand or split it.
  static std::mutex cap_mu; static std::map<std::string, int> cap_n;
  { std::lock_guard<std::mutex> lk(cap_mu);
    int& k = cap_n[t["id"].get<std::string>() + "|" + theorem + "|" + std::to_string(t.value("_attempt", 1))];
    if (k >= 2) return "refused: lean_prove already failed twice on " + theorem + " in this attempt. Do not call it again for this theorem: prove it with Mathlib tactics by hand (intro, simp, omega, aesop, Finset/Set lemmas), state and prove smaller lemmas first, or post_message(kind=proof_failure) if the statement itself looks wrong or too hard.";
    ++k; }
  auto slot = std::make_unique<ProverSlot>(c.prover_slots, 1);
  link_lean_packages(c, file);
  auto env = gate_env("");
  if (!std::getenv("PRISM_MAX_NEW")) env.push_back("PRISM_MAX_NEW=400");   // a tactic reply needs far fewer tokens than the default 768-1024
  std::vector<std::string> argv = batch ? std::vector<std::string>{c.prism_bin, "prove", "--all", file.parent_path().string(), "--write", "--allow-exec"}
                                        : std::vector<std::string>{c.prism_bin, "prove", file.string(), theorem, "--write", "--allow-exec"};
  const int to = c.prove_timeout_s * (batch ? 4 : 1);
  if (!c.gate_wrap.empty()) argv.insert(argv.begin(), c.gate_wrap.begin(), c.gate_wrap.end());
  auto r = run_job(argv, -1, to, wt.string(), 500, &env);
  if (r.code == 1 && !c.prover_fpk_escalate.empty() && fs::exists(c.prover_fpk_escalate)) {   // not proved: one retry with the larger model
    slot.reset();
    ProverSlot excl(c.prover_slots, c.prover_slots);   // the 32B model needs the whole GPU
    std::erase_if(env, [](const std::string& kv) { return kv.starts_with("PRISM_PROVER_FPK="); });
    env.push_back("PRISM_PROVER_FPK=" + c.prover_fpk_escalate);
    auto r2 = run_job(argv, -1, to, wt.string(), 500, &env);
    db.event(t["id"], "lean_prove_escalated", json{{"theorem", theorem}, {"code", r2.code}});
    r2.tail = "[escalated to " + fs::path(c.prover_fpk_escalate).filename().string() + "]\n" + r2.tail;
    r = r2;
  }
  db.event(t["id"], "lean_prove", json{{"theorem", theorem}, {"code", r.code}});
  std::string tail = r.tail.size() > 6000 ? r.tail.substr(r.tail.size() - 6000) : r.tail;
  if (r.code == 0) return "proved and written to " + fs::relative(file, wt).string() + "\n" + tail;
  if (r.code == 3) return "NOTRUN: prism prove has no prover model (set PRISM_PROVER_FPK) or Lean is missing\n" + tail;
  // Sixth rehearsal (7 Nov 2026): after a failed hand proof the file no longer compiled, and every lean_prove call then ended at once with
  // code 2 (an error, not "not proved") while the agent kept retrying other theorems. Say what code 2 means and what to do.
  if (r.code != 1) return "error (the file probably does not compile): lean_prove needs the whole file to compile with every unfinished proof written as `:= by sorry`. Fix the Lean error below, or replace the proof that fails with `by sorry`, then call lean_prove again. Do not call it on other theorems until the file compiles.\n" + tail;
  return std::string(r.code == 1 ? "not proved" : "error") + ". State smaller lemmas (each `theorem name : stmt := by sorry`) and prove them one at a time.\n" + tail;
}

TaskOutcome run_task(const json& t, Api& api, const Config& c, Db& db, int slot) {
  TaskOutcome out{Result::failed, {}, ""};
  const std::string id = t["id"], role = t["role"], wp = t.value("wp", std::string("misc")), branch = "t/" + id;
  const int attempt = t.value("_attempt", 1);
  const std::string tier = role_tier(t, c);
  const auto owns = owns_list(t);
  // 1. Worktree in tmpfs (PBSD_WT). The package branch first takes in the integration branch, so a
  //    task sees its dependencies' merged output and every integrated upstream package.
  const char* wtroot = getenv("PBSD_WT");
  fs::path wt = fs::path(wtroot ? wtroot : "/run/pbsd/wt") / id;
  fs::create_directories(wt.parent_path());
  if (!fs::exists(wt)) {
    const std::string wpb = "wp/" + wp;
    if (rev(c, "refs/heads/" + wpb).empty()) {
      std::string from = rev(c, "refs/heads/" + c.merge_target); if (from.empty()) from = rev(c, "HEAD");
      run_job({"git", "update-ref", "refs/heads/" + wpb, from, ""}, -1, 60, c.repo, 0);
    }
    if (!rev(c, "refs/heads/" + c.merge_target).empty()) {
      if (auto e = merge_branch(c, wpb, c.merge_target, "sync " + wpb + " with " + c.merge_target); !e.empty()) db.event(id, "sync_conflict", e);
    }
    sh(c.repo, {"git", "worktree", "prune"});
    // Resume from wip/<id> if a checkpoint exists, else from t/<id> (merged with the package branch), else the package branch.
    std::string base = !rev(c, "refs/heads/wip/" + id).empty() ? "wip/" + id : !rev(c, "refs/heads/" + branch).empty() ? branch : wpb;
    if (base != wpb) {   // a retry or a reopened task also takes in what its package merged since
      if (auto e = merge_branch(c, base, wpb, "refresh " + id + " from " + wpb); !e.empty()) db.event(id, "refresh_conflict", e);
    }
    sh(c.repo, {"git", "worktree", "add", "--no-checkout", "-B", branch, wt.string(), base});
    std::vector<std::string> sp{"git", "sparse-checkout", "set", "--no-cone"};
    for (auto& p : c.wt_sparse) sp.push_back(p);
    sh(wt.string(), sp); sh(wt.string(), {"git", "checkout", "-q"});
    if (!fs::exists(wt / ".git")) { db.event(id, "worktree_failed", base); out.r = Result::infra; return out; }
  }
  // 2. Layered prompt: global rules, role template, contract list, unit source, then the variable tail.
  fs::path plan = fs::path(c.repo) / "docs/plan";
  const size_t ctx_chars = size_t(c.ctx_per_slot) * 4;
  std::string global, agents;
  {
    std::string all = slurp(plan / "AGENTS.md", 400000);
    auto block = [&](const std::string& marker) {
      auto a = all.find(marker); if (a == std::string::npos) return std::string();
      return all.substr(a, all.find("```", a) - a);
    };
    global = block("You are one agent in the PBSD build.");
    agents = block("Role: " + role + ".");
  }
  std::vector<std::string> headers;
  if (fs::exists(wt / "pbsd/kernel/include"))
    for (auto& e : fs::recursive_directory_iterator(wt / "pbsd/kernel/include", fs::directory_options::skip_permission_denied))
      if (e.is_regular_file()) headers.push_back(e.path().lexically_relative(wt).string());
  std::sort(headers.begin(), headers.end());   // byte-stable for the prompt cache
  std::string contracts; for (auto& h : headers) contracts += h + "\n";
  std::string source = load_source(t, c, ctx_chars / 4);
  json task = t; task.erase("_attempt"); task.erase("deps_one_day");
  std::string inbox; for (auto& m : db.messages_for(id, wp)) inbox += m.dump() + "\n";
  json msgs = json::array({
    {{"role", "system"}, {"content", "PBSD agent rules:\n" + global}},
    {{"role", "system"}, {"content", "Role template:\n" + agents + "\nYour role: " + role}},
    {{"role", "system"}, {"content", "Frozen contract headers (read_file to open):\n" + contracts.substr(0, ctx_chars / 16)}},
    {{"role", "user"}, {"content", "Unit source:\n" + source}},
    {{"role", "user"}, {"content", "Task:\n" + task.dump(2) + "\nOwned paths (the only ones you may write): " + json(owns).dump() +
      "\nAttempt " + std::to_string(attempt) + (inbox.empty() ? "" : "\nMessages for you:\n" + inbox) +
      (attempt >= 5 ? "\nThis task has failed repeatedly: if it is too large, post a handoff message proposing a split." : "")}}});
  // Conversation state saved for resume (fresh context from attempt 3).
  fs::path state = fs::path(c.state_dir) / "conv" / (id + ".json");
  fs::create_directories(state.parent_path());
  if (attempt < 3 && fs::exists(state)) { json s = json::parse(slurp(state, 64 << 20), nullptr, false); if (s.is_array()) msgs = s; }

  auto [kind, node] = gate_slot(role, c);
  const int max_turns = c.role_max_turns.count(role) ? c.role_max_turns.at(role) : c.max_turns;
  double last_ck = now(), task_start = now();
  bool gate_ok = false;
  json tools = tools_schema(role, false);
  if (role == "prover" && getenv("PRISM_PROVER_FPK")) tools = tools_schema(role, true);
  else tools.erase(std::remove_if(tools.begin(), tools.end(), [](const json& x) { return x["function"]["name"] == "lean_prove"; }), tools.end());
  auto commit = [&](std::string m) {
    if (m.ends_with(" gate")) {   // the proof gate checks annotation-only diffs against this base
      std::string base = trim(sh(wt.string(), {"git", "merge-base", "HEAD", "refs/heads/wp/" + wp}));
      if (!base.empty()) m += "\n\nPBSD-Base: " + base;
    }
    // Stage only owned paths, one at a time: one missing path must not stop the others.
    for (auto& p : owns) sh(wt.string(), {"git", "add", "-A", "--", p.ends_with(".*") ? ":(glob)" + p : p});
    sh(wt.string(), {"git", "-c", "user.name=pbsd-agent", "-c", "user.email=agent@pbsd.local", "commit", "-q", "--allow-empty", "-m", m});
    return trim(sh(wt.string(), {"git", "rev-parse", "HEAD"}));
  };
  auto finish_state = [&] {
    sh(wt.string(), {"git", "update-ref", "refs/heads/wip/" + id, "HEAD"});
    std::ofstream(state) << msgs.dump();
  };
  bool compacted_hard = false;
  // Progress nudges. GLM-5.3-Flash on the first real dry run (P0-01-IF, 6 Nov 2026) made 87 read-only calls over 39 turns, wrote nothing and
  // never ran the gate, twice. A task that has written nothing by a third of its turns, or has no passing gate by two thirds, is told so.
  bool wrote_any = false, nudged_write = false, nudged_gate = false;
  // Third rehearsal (6 Nov 2026): P0-02-IF received both nudges and kept reading for all 40 turns on all 3 attempts (51 read-only calls, no
  // write, no gate run). After the write nudge the read-only tools are therefore withdrawn until the agent has written a file.
  bool reads_off = false;
  auto without_reads = [](const json& all) {
    json out = json::array();
    for (auto& tl : all) { const std::string nm = tl["function"].value("name", ""); if (nm != "read_file" && nm != "list_dir" && nm != "grep") out.push_back(tl); }
    return out;
  };
  // Circuit breaker. In the first rehearsal (6 Nov 2026) one proof task ran its gate 338 times on three attempts, repeating the same failure,
  // and the prover tasks took 77% of the spend. The same failing gate output (digits ignored) three times in a row gets a warning; six ends the attempt.
  size_t last_fail_hash = 0; int same_fail = 0;
  for (int turn = 0; turn < max_turns; ++turn) {
    db.heartbeat(id, c.lease_s);
    if (!wrote_any && !nudged_write && turn >= std::max(3, max_turns / 3)) {
      nudged_write = true; reads_off = true;
      msgs.push_back({{"role", "user"}, {"content", std::format("You have used {} of {} turns and have written no file. Stop exploring: everything the task needs is in the task text and the files already read. Write the owned files now with write_file, with their real content: the gate rejects a file that holds only a comment, a stub or a placeholder, so a stub only wastes a turn. Then call run_gate and submit. read_file, list_dir and grep are switched off until you have written a file.", turn, max_turns)}});
    }
    if (!gate_ok && !nudged_gate && turn >= std::max(5, max_turns * 2 / 3)) {
      nudged_gate = true;
      msgs.push_back({{"role", "user"}, {"content", std::format("{} of {} turns used and the gate has not passed. Call run_gate now. If it fails, fix what the log says and run it again. The attempt fails if submit is not called before turn {}.", turn, max_turns, max_turns)}});
    }
    Usage u;
    const int reply = c.backend == "api" ? c.api_max_tokens : c.max_tokens;
    size_t budget = size_t(std::max(4096, c.ctx_per_slot - reply - 1024));
    if (compacted_hard) budget /= 2;
    if (int n = compact_history(msgs, budget)) db.event(id, "compacted", n);
    if (wrote_any) reads_off = false;
    const json tools_now = reads_off ? without_reads(tools) : tools;
    auto r = api.chat(msgs, tools_now, u, role, slot, wp, tier, attempt >= c.escalate_at ? c.escalate_reasoning_effort : std::string());
    out.usage.push_back(u);
    if (u.hit + u.miss + u.out > 0) db.cost(id, u.hit, u.miss, u.out, u.usd(), u.provider);   // billed as it happens
    if (!r) {
      db.event(id, "api_error", r.error());
      if (r.error().starts_with("402")) throw std::runtime_error(r.error());
      if (r.error().starts_with("400") && !compacted_hard) { compacted_hard = true; continue; }   // context too long: compact harder once
      finish_state(); out.r = r.error().starts_with("400") ? Result::failed : Result::infra; return out;
    }
    json m = (*r)["choices"][0]["message"];
    if (m.contains("reasoning_content")) m.erase("reasoning_content");   // not resent: saves input tokens
    msgs.push_back(m);
    if (!m.contains("tool_calls") || m["tool_calls"].empty()) { msgs.push_back({{"role", "user"}, {"content", "Use the tools; call submit when the gate passes."}}); continue; }
    bool submitted = false;
    for (auto& call : m["tool_calls"]) {
      std::string fnm = call["function"].value("name", "");
      json a = json::parse(call["function"].value("arguments", "{}"), nullptr, false);
      if (!a.is_object()) a = json::object();
      std::string res;
      auto rd = [&](const std::string& p) -> std::optional<fs::path> {   // ref:<tree>/..., worktree, then main checkout
        if (p.starts_with("ref:")) {
          auto sl = p.find('/', 4); auto it = c.ref_roots.find(p.substr(4, sl == std::string::npos ? std::string::npos : sl - 4));
          if (it == c.ref_roots.end()) return std::nullopt;
          return in_tree(it->second, sl == std::string::npos ? "." : p.substr(sl + 1));
        }
        auto w = in_tree(wt, p.empty() ? "." : p); if (w && fs::exists(*w)) return w;
        auto mc = in_tree(c.repo, p.empty() ? "." : p); if (mc && fs::exists(*mc)) return mc;
        return w;
      };
      if (fnm == "read_file") {
        auto p = rd(a.value("path", ""));
        if (!p || !fs::is_regular_file(*p)) res = "not found or not allowed: " + a.value("path", "");
        else {
          std::string s = slurp(*p, 8u << 20);
          int lo = a.value("start_line", 0), hi = a.value("end_line", 0);
          if (lo > 0 || hi > 0) {
            std::istringstream is(s); std::string l, o; int n = 0;
            while (std::getline(is, l)) { ++n; if (n >= std::max(lo, 1) && (hi <= 0 || n <= hi)) o += l + "\n"; }
            s = o;
          }
          if (s.size() > size_t(c.tool_result_chars)) s = s.substr(0, c.tool_result_chars) + "\n[truncated: use start_line/end_line]";
          res = s;
        }
      } else if (fnm == "list_dir") {
        auto p = rd(a.value("path", "."));
        if (!p || !fs::is_directory(*p)) res = "not found or not allowed";
        else { std::vector<std::string> v; for (auto& e : fs::directory_iterator(*p, fs::directory_options::skip_permission_denied)) v.push_back(e.path().filename().string() + (e.is_directory() ? "/" : ""));
               std::sort(v.begin(), v.end()); for (auto& x : v) res += x + "\n"; if (res.size() > size_t(c.tool_result_chars)) res.resize(c.tool_result_chars); }
      } else if (fnm == "grep") {
        auto p = rd(a.value("path", "."));
        if (!p) res = "not allowed";
        else { auto g = run_job({"grep", "-rnIE", "--max-count=20", "-e", a.value("pattern", ""), p->string()}, -1, 60, wt.string(), 0);
               res = g.tail.substr(0, std::min<size_t>(g.tail.size(), size_t(c.tool_result_chars))); if (res.empty()) res = "no matches"; }
      } else if (fnm == "write_file" || fnm == "edit_file") {
        std::string p = a.value("path", "");
        auto dst = in_tree(wt, p);
        if (!owned(t, p) || !dst) res = "refused: " + p + " is outside this task's owned paths " + json(owns).dump();
        else if (fnm == "write_file") {
          fs::create_directories(dst->parent_path());
          std::ofstream f(*dst, a.value("append", false) ? std::ios::app : std::ios::trunc); f << a.value("content", "");
          res = "written"; gate_ok = false;
        } else {
          std::string s = slurp(*dst, 64u << 20), o = a.value("old_text", ""), nw = a.value("new_text", "");
          size_t at = o.empty() ? std::string::npos : s.find(o);
          if (at == std::string::npos) res = "old_text not found";
          else if (s.find(o, at + 1) != std::string::npos) res = "old_text occurs more than once; include more context";
          else { s.replace(at, o.size(), nw); std::ofstream(*dst, std::ios::trunc) << s; res = "edited"; gate_ok = false; }
        }
      } else if (fnm == "run_gate") {
        while (mem_used_frac() > c.ram_pause) std::this_thread::sleep_for(std::chrono::seconds(10));
        const std::string sha = commit(id + " gate");
        fs::path gate = fs::path(c.gates_dir) / (role + ".sh");
        JobResult g{3, "", false};
        for (int tries = 0; tries < std::max(1, c.gate_infra_retries); ++tries) {
          GateLease gl = gate_acquire(kind, c);
          std::vector<std::string> argv{"bash", gate.string(), id, wt.string()};
          std::vector<std::string> env; const std::vector<std::string>* envp = nullptr;
          if (!gl.host.empty()) {   // cloud worker: same gate on the same commit, fetched from the mirror
            sh(wt.string(), {"git", "push", "-q", "-f", c.mirror_url, "HEAD:refs/gates/" + id});
            argv = {"ssh", "-o", "BatchMode=yes", "-o", "ConnectTimeout=20", "-o", "ServerAliveInterval=30", gl.host, "--", c.remote_gate, role, id, sha,
                    t.value("proof_tier", std::string("-"))};
          } else {
            if (gl.burst) argv.insert(argv.begin(), c.burst_cmd.begin(), c.burst_cmd.end());
            else if (!c.gate_wrap.empty()) argv.insert(argv.begin(), c.gate_wrap.begin(), c.gate_wrap.end());
            if (c.gate_scrub_env && !gl.burst) { env = gate_env((fs::path(wtroot ? wtroot : "/run/pbsd/wt") / ".build" / id).string()); envp = &env; }
            if (t.contains("proof_tier")) {   // the proof gate takes its tier from the task, never from the tree
              if (!envp) { for (char** e = environ; *e; ++e) env.push_back(*e); envp = &env; }
              env.push_back("PBSD_PROOF_TIER=" + t["proof_tier"].get<std::string>());
            }
          }
          g = run_job(argv, gl.burst ? -1 : node, gate_timeout(t, kind, c), wt.string(), 500, envp);
          gate_release(gl);
          // ssh exits 255 when the worker is gone (spot reclaim): same as an infrastructure error.
          if (g.code != 3 && g.code != 255) break;
          db.event(id, "gate_infra", json{{"code", g.code}, {"host", gl.host}});
        }
        if (g.code == 3 || g.code == 255) { finish_state(); out.r = Result::infra; return out; }
        gate_ok = g.code == 0; if (gate_ok) out.gate_sha = sha;
        db.event(id, "gate", json{{"code", g.code}, {"timeout", g.timed_out}, {"sha", sha}});
        std::string tail = g.tail.size() > size_t(c.tool_result_chars) ? g.tail.substr(g.tail.size() - c.tool_result_chars) : g.tail;
        res = std::format("exit {}{} on {}\n{}", g.code, g.timed_out ? " (watchdog timeout)" : "", sha.substr(0, 12), tail);
        if (gate_ok) { same_fail = 0; last_fail_hash = 0; }
        else {
          std::string norm; for (char ch : g.tail) if (!std::isdigit(static_cast<unsigned char>(ch)) && !std::isxdigit(static_cast<unsigned char>(ch))) norm += ch;
          const size_t h = std::hash<std::string>{}(norm);
          same_fail = (h == last_fail_hash) ? same_fail + 1 : 1; last_fail_hash = h;
          if (same_fail == 3) res += "\n\nNOTE: this is the same failure three times in a row. Rerunning the gate will not change it: change the files, or post_message(kind=proof_failure or test_failure, to=impl) if the problem is in another task's files.";
          if (same_fail >= 6) {
            db.event(id, "stuck", json{{"same_failure", same_fail}});
            msgs.push_back({{"role", "tool"}, {"tool_call_id", call.value("id", "")}, {"content", res}});
            finish_state(); out.r = Result::failed; return out;
          }
        }
      } else if (fnm == "post_message") {
        std::string to = resolve_to(t, a.value("to", "")), k = a.value("kind", "note");
        db.post(to, id, k, a.value("body", ""));
        // A contract change request reopens the package's -IF task (it owns the package contracts) so
        // requests are applied in a batch; dependants whose build breaks are retried by their gates.
        if (k == "contract_change_request" && db.state_of(to) == "passed") db.reopen({to}, {});
        res = "posted to " + to;
      } else if (fnm == "lean_prove") {
        std::string p = a.value("file", ""), stmt = a.value("theorem", "");
        auto dst = in_tree(wt, p);
        if (!getenv("PRISM_PROVER_FPK")) res = "no prover model configured (PRISM_PROVER_FPK)";
        else if (!owned(t, p) || !dst || !p.ends_with(".lean")) res = "refused: " + p + " is not an owned .lean file";
        else { res = lean_prove(c, t, wt, *dst, stmt, db); if (res.starts_with("proved")) gate_ok = false; }
      } else if (fnm == "read_messages") {
        for (auto& mm : db.messages_for(id, wp)) res += mm.dump() + "\n";
        if (res.empty()) res = "no messages";
      } else if (fnm == "submit") {
        submitted = true;
        if (!gate_ok) res = "rejected: the gate has not passed on the current tree (run_gate after your last write)";
        else if (role == "reviewer" && a.value("verdict", "") == "changes") {
          // Reopen the unit's tasks the findings name (impl by default); this review waits for them.
          std::set<std::string> targets;
          for (auto& f : a.value("findings", json::array())) targets.insert(f.value("target", "impl"));
          if (targets.empty()) targets.insert("impl");
          std::vector<std::string> ready, block{id};
          auto add = [&](const std::string& x) { ready.push_back(resolve_to(t, x)); };
          for (auto& x : targets) add(x);
          if (targets.count("spec")) { add("impl"); add("test"); }
          if (targets.count("impl") || targets.count("spec")) block.push_back(resolve_to(t, "proof"));
          std::sort(ready.begin(), ready.end()); ready.erase(std::unique(ready.begin(), ready.end()), ready.end());
          for (auto& r2 : ready) db.post(r2, id, "review_findings", a.dump());
          db.reopen(ready, block);
          res = "changes requested; reopened " + json(ready).dump();
          commit(id + " review: changes"); finish_state();
          out.r = Result::changes; return out;
        } else if (role == "reviewer" && a.value("verdict", "") != "approve") {
          submitted = false; res = "rejected: verdict must be approve or changes";
        } else res = "accepted";
      } else res = "unknown tool " + fnm;
      if ((fnm == "write_file" || fnm == "edit_file") && !res.starts_with("refused")) wrote_any = true;
      msgs.push_back({{"role", "tool"}, {"tool_call_id", call.value("id", "")}, {"content", res}});
    }
    // 3. Atomic commit at turn end; WIP checkpoint every checkpoint_s.
    commit(id + " turn " + std::to_string(turn));
    if (now() - last_ck > c.checkpoint_s) { finish_state(); last_ck = now(); }
    if (submitted && gate_ok) { out.r = Result::passed; break; }
    if (now() - task_start > double(c.agent_watchdog_s) * max_turns) break;   // whole-task watchdog
  }
  finish_state();
  if (out.r != Result::passed) return out;
  // 5. Deterministic merges. A conflict sends the task back with the conflict as a message.
  std::string e = merge_branch(c, "wp/" + wp, branch, "merge " + id);
  if (e.empty() && (role == "integrator" || id.ends_with("-IF")))
    e = merge_branch(c, c.merge_target, "wp/" + wp, "integrate " + wp + " (" + id + ")");
  if (!e.empty()) { db.post(id, "runner", "test_failure", e); db.event(id, "merge_failed", e); out.r = Result::failed; return out; }
  db.event(id, "merged", out.gate_sha);
  sh(c.repo, {"git", "worktree", "remove", "--force", wt.string()});
  fs::remove(state);
  return out;
}
}  // namespace pbsd
