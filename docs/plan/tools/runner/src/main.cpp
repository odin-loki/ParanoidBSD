// pbsd_run: PBSD runner and scheduler. Usage: pbsd_run init|run|status|health|selftest|cpath|estimate
#include "common.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <csignal>
#include <sys/wait.h>

using namespace pbsd;

// Stand-in for llama-server and API providers used by selftest: /health and /v1/chat/completions,
// one request at a time. mode "429" answers rate-limited; mode "script <file>" answers with the
// assistant messages in <file> (a JSON array) in order and saves each request body as <file>.req.<n>.
static int fake_server(int port, bool limited, const std::string& script = "") {
  json steps = json::array(); size_t step = 0;
  if (!script.empty()) { std::ifstream f(script); steps = json::parse(f); }
  int fd = socket(AF_INET, SOCK_STREAM, 0), one = 1;
  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof one);
  sockaddr_in a{}; a.sin_family = AF_INET; a.sin_port = htons(port); a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  if (bind(fd, (sockaddr*)&a, sizeof a) != 0 || listen(fd, 16) != 0) return 1;
  for (;;) {
    int c = accept(fd, nullptr, nullptr); if (c < 0) continue;
    std::string req; char b[65536];
    for (;;) {   // headers, then Content-Length bytes
      ssize_t n = read(c, b, sizeof b); if (n <= 0) break; req.append(b, n);
      auto h = req.find("\r\n\r\n"); if (h == std::string::npos) continue;
      auto cl = req.find("Content-Length: "); size_t len = cl == std::string::npos ? 0 : std::stoul(req.substr(cl + 16));
      if (req.size() >= h + 4 + len) break;
    }
    std::string body = req.starts_with("GET /health") ? R"({"status":"ok"})"
      : limited ? R"({"error":{"code":"1302","message":"Rate limit reached for requests"}})"
      : R"({"choices":[{"message":{"role":"assistant","content":"pong"}}],"usage":{"prompt_tokens":12,"completion_tokens":1,"prompt_tokens_details":{"cached_tokens":10}}})";
    if (!script.empty() && !req.starts_with("GET")) {
      auto h = req.find("\r\n\r\n");
      std::ofstream(script + ".req." + std::to_string(step)) << (h == std::string::npos ? "" : req.substr(h + 4));
      json msg = step < steps.size() ? steps[step] : json{{"role", "assistant"}, {"content", "done"}};
      ++step;
      body = json{{"choices", json::array({{{"message", msg}}})},
                  {"usage", {{"prompt_tokens", 1000}, {"completion_tokens", 100}, {"prompt_tokens_details", {{"cached_tokens", 800}}}}}}.dump();
    }
    std::string r = std::string(limited ? "HTTP/1.1 429 Too Many Requests\r\nRetry-After: 30" : "HTTP/1.1 200 OK") +
                    "\r\nContent-Type: application/json\r\nContent-Length: " + std::to_string(body.size()) +
                    "\r\nConnection: close\r\n\r\n" + body;
    (void)!write(c, r.data(), r.size()); close(c);
  }
}

int pbsd::selftest(const Config& base) {
  bool ok = true;
  auto check = [&](const char* name, bool v) { std::cout << name << (v ? " ok" : " FAILED") << "\n"; ok = ok && v; };
  auto r = run_job({"sh", "-c", "echo ok; sleep 5"}, -1, 1, "");
  check("watchdog", r.timed_out);
  auto r2 = run_job({"sh", "-c", "echo hello"}, 0, 5, "");
  check("job", r2.code == 0 && r2.tail.find("hello") != std::string::npos);
  // Local backend: server down, supervisor restarts it with server_cmd, chat goes through.
  Config c = base;
  int port = 18000 + getpid() % 1000;
  c.backend = "local"; c.local_url = "http://127.0.0.1:" + std::to_string(port) + "/v1";
  c.server_cmd = {"/proc/self/exe", "fake-server", std::to_string(port)};
  c.load_timeout_s = 20; c.restart_max = 2;
  char self[4096]; ssize_t n = readlink("/proc/self/exe", self, sizeof self - 1);
  if (n > 0) { self[n] = 0; c.server_cmd[0] = self; }
  Api api(c);
  check("health check (down)", !api.local()->healthy());
  check("restart llama-server", api.local()->ensure_up() && api.local()->restarts() == 1);
  Usage u; auto j = api.chat(json::array({{{"role", "user"}, {"content", "ping"}}}), json::array(), u, "implementer", 0);
  check("local chat", j && (*j)["choices"][0]["message"]["content"] == "pong" && u.out == 1 && u.usd() == 0);
  json h = json::array({{{"role", "system"}, {"content", "s"}}});
  for (int i = 0; i < 20; ++i) h.push_back({{"role", "tool"}, {"content", std::string(4000, 'x')}});
  compact_history(h, 4000);
  check("context compaction", approx_tokens(h) <= 8000 && h[0]["content"] == "s");
  std::system(("pkill -f 'fake-server " + std::to_string(port) + "'").c_str());

  // Rate limiter: in-flight cap, RPM bucket refill, TPM reservation and refund.
  RateLimiter l1(2, 2, 0);
  bool a1 = l1.try_acquire(1, 100), a2 = l1.try_acquire(1, 100), a3 = l1.try_acquire(1, 100);
  check("limiter in-flight cap", a1 && a2 && !a3);
  l1.release(1, 1, 100); l1.release(1, 1, 100);
  bool b1 = l1.try_acquire(1, 100); l1.release(1, 1, 100);
  bool b2 = l1.try_acquire(1, 130); l1.release(1, 1, 130);
  check("limiter RPM bucket", !b1 && b2);
  RateLimiter l2(0, 0, 1000);
  bool t1 = l2.try_acquire(800, 200), t2 = l2.try_acquire(800, 200);
  l2.release(800, 100, 200);
  bool t3 = l2.try_acquire(800, 200);
  check("limiter TPM bucket and refund", t1 && !t2 && t3);

  // Failover: provider "a" answers 429, "b" answers; the turn is billed at b's prices with cache hits.
  int pa = port + 1, pb = port + 2;
  pid_t fa = fork(); if (fa == 0) { execl(self, self, "fake-server", std::to_string(pa).c_str(), "429", (char*)nullptr); _exit(127); }
  pid_t fb = fork(); if (fb == 0) { execl(self, self, "fake-server", std::to_string(pb).c_str(), (char*)nullptr); _exit(127); }
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  Config ca = base; ca.backend = "api"; ca.providers.clear();
  for (auto [nm, p, pin] : {std::tuple{"a", pa, 0.15}, std::tuple{"b", pb, 0.10}}) {
    Provider pr; pr.name = nm; pr.url = "http://127.0.0.1:" + std::to_string(p) + "/v1/chat/completions";
    pr.keys = {"test"}; pr.concurrency = 4; pr.price_in = pin; ca.providers.push_back(pr);
  }
  Api api2(ca); Usage u2;
  auto j2 = api2.chat(json::array({{{"role", "user"}, {"content", "ping"}}}), json::array(), u2, "implementer", -1, "");
  check("API failover on 429", j2 && u2.provider == "b" && u2.hit == 10 && u2.miss == 2 && u2.usd() > 0);
  kill(fa, SIGKILL); kill(fb, SIGKILL); waitpid(fa, nullptr, 0); waitpid(fb, nullptr, 0);

  // Gate admission: one local compile slot, then the cloud burst slot.
  Config cg = base; cg.slots["compile"] = {1, 1, 2, 0}; cg.burst_cmd = {"env"}; cg.burst_slots = 1;
  auto g1 = gate_acquire("compile", cg), g2 = gate_acquire("compile", cg);
  check("gate slots and burst", !g1.burst && g2.burst);
  gate_release(g1); gate_release(g2);
  // Worker list: the burst gate goes to the least-busy registered worker; comments and bad lines skipped.
  std::string wf = (std::filesystem::temp_directory_path() / ("pbsd-workers-" + std::to_string(getpid()) + ".txt")).string();
  std::ofstream(wf) << "# host slots\nw-a 1\nbroken\nw-b 2\n";
  Config cw = base; cw.slots["compile"] = {1, 1, 2, 0}; cw.workers_file = wf;
  auto w0 = gate_acquire("compile", cw), w1 = gate_acquire("compile", cw), w2 = gate_acquire("compile", cw);
  check("worker list", load_workers(wf).size() == 2 && !w0.burst && w1.host == "w-a" && w2.host == "w-b");
  gate_release(w0); gate_release(w1); gate_release(w2);
  // QEMU gates go only to workers marked kvm.
  std::ofstream(wf) << "w-a 4\nw-k 1 kvm\n";
  Config cq = base; cq.slots["qemu"] = {0, 2, 4, 0}; cq.workers_file = wf;
  auto q1 = gate_acquire("qemu", cq);
  check("qemu gates only on KVM workers", q1.host == "w-k" && load_workers(wf).at(1).kvm && !load_workers(wf).at(0).kvm);
  gate_release(q1);

  // Provider order: weight-0 providers only after the weighted ones; the start follows the weights.
  {
    std::vector<Provider> ps(3);
    ps[0].name = "cheap"; ps[0].weight = 3; ps[1].name = "cheap2"; ps[1].weight = 1; ps[2].name = "official"; ps[2].weight = 0;
    int first_cheap = 0, official_first = 0;
    for (int k = 0; k < 400; ++k) {
      auto o = provider_order(ps, "P" + std::to_string(k), "flash");
      if (o.size() != 3 || o.back() != 2) official_first++;
      if (o.front() == 0) first_cheap++;
    }
    check("provider weights and overflow order", official_first == 0 && first_cheap > 240 && first_cheap < 360);
  }
  // Ownership parsing: comma-separated string, braces, free text and "-" dropped.
  {
    auto o = owns_list(json{{"owns", "pbsd/kernel/include/{sched,ring}/, docs/x.md, toolchain flags, -"}});
    check("owns parsing", o == std::vector<std::string>{"pbsd/kernel/include/sched/", "pbsd/kernel/include/ring/", "docs/x.md"});
    Config ct = base; ct.role_tier = {{"implementer", "flash"}}; ct.default_tier = "frontier"; ct.escalate_tier = "frontier"; ct.escalate_at = 3;
    check("role tiers and escalation", role_tier(json{{"role", "implementer"}, {"_attempt", 1}}, ct) == "flash" &&
          role_tier(json{{"role", "prover"}, {"_attempt", 1}}, ct) == "frontier" &&
          role_tier(json{{"role", "implementer"}, {"_attempt", 3}}, ct) == "frontier");
  }

  // End to end on a scratch repository with a scripted model: path escapes refused, a write after a
  // passing gate invalidates it, the task passes only after a fresh gate, and t/<id> is merged into wp/<WP>.
  namespace fs = std::filesystem;
  fs::path tmp = fs::temp_directory_path() / ("pbsd-e2e-" + std::to_string(getpid()));
  fs::remove_all(tmp); fs::create_directories(tmp / "repo/docs/plan"); fs::create_directories(tmp / "repo/tools/gates");
  fs::create_directories(tmp / "repo/pbsd/kernel/include"); fs::create_directories(tmp / "state"); fs::create_directories(tmp / "wt");
  std::ofstream(tmp / "repo/docs/plan/AGENTS.md") << "```text\nYou are one agent in the PBSD build. Rules.\n```\n```text\nRole: implementer. Port it.\n```\n```text\nRole: reviewer. Review it.\n```\n";
  std::ofstream(tmp / "repo/pbsd/kernel/include/x.h") << "// contract\n";
  std::ofstream(tmp / "repo/tools/gates/implementer.sh") << "grep -q ok \"$2/pbsd/kernel/x/a.cpp\" && ! env | grep -q E2E_SECRET\n";
  std::ofstream(tmp / "repo/tools/gates/reviewer.sh") << "exit 0\n";
  std::ofstream(tmp / "secret") << "do not read";
  auto git = [&](std::vector<std::string> a) { a.insert(a.begin(), {"git", "-c", "user.name=t", "-c", "user.email=t@t"}); return run_job(a, -1, 60, (tmp / "repo").string(), 0); };
  git({"init", "-q", "-b", "main"}); git({"add", "-A"}); git({"commit", "-q", "-m", "base"});
  git({"checkout", "-q", "--detach"});
  auto call = [](const char* id, const char* fn, json args) {
    return json{{"id", id}, {"type", "function"}, {"function", {{"name", fn}, {"arguments", args.dump()}}}};
  };
  auto turn = [](std::vector<json> calls) { return json{{"role", "assistant"}, {"content", nullptr}, {"tool_calls", calls}}; };
  json script = json::array({
    turn({call("1", "read_file", {{"path", "/etc/passwd"}}), call("2", "read_file", {{"path", "../secret"}}),
          call("3", "write_file", {{"path", "pbsd/other/a.cpp"}, {"content", "x"}}),
          call("4", "write_file", {{"path", "pbsd/kernel/x/a.cpp"}, {"content", "int a; // bad\n"}})}),
    turn({call("5", "run_gate", json::object())}),
    turn({call("6", "edit_file", {{"path", "pbsd/kernel/x/a.cpp"}, {"old_text", "bad"}, {"new_text", "ok"}})}),
    turn({call("7", "run_gate", json::object())}),
    turn({call("8", "edit_file", {{"path", "pbsd/kernel/x/a.cpp"}, {"old_text", "ok"}, {"new_text", "ok2"}})}),
    turn({call("9", "submit", {{"summary", "early"}})}),
    turn({call("10", "run_gate", json::object())}),
    turn({call("11", "submit", {{"summary", "done"}})})});
  std::ofstream(tmp / "script.json") << script.dump();
  int pe = port + 3;
  pid_t fe = fork(); if (fe == 0) { execl(self, self, "fake-server", std::to_string(pe).c_str(), "script", (tmp / "script.json").c_str(), (char*)nullptr); _exit(127); }
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  Config ce = base; ce.backend = "api"; ce.providers.clear(); ce.repo = (tmp / "repo").string(); ce.state_dir = (tmp / "state").string();
  ce.gates_dir = (tmp / "repo/tools/gates").string(); ce.workers_file.clear(); ce.burst_cmd.clear(); ce.gate_wrap.clear();
  ce.slots["compile"] = {2, 1, 1, -1}; ce.checkpoint_s = 1; ce.ram_pause = 1.01;
  { Provider pr; pr.name = "e2e"; pr.url = "http://127.0.0.1:" + std::to_string(pe) + "/v1/chat/completions"; pr.keys = {"k"}; pr.tier = "frontier"; ce.providers.push_back(pr); }
  setenv("PBSD_WT", (tmp / "wt").c_str(), 1); setenv("E2E_SECRET_KEY", "leak", 1);
  {
    Db edb((tmp / "state/tasks.db").string()); edb.init_schema();
    json task{{"id", "P9-01-U001-IMPL"}, {"wp", "P9-01"}, {"phase", 9}, {"role", "implementer"}, {"owns", "pbsd/kernel/x/, docs/x.md"},
              {"deps", json::array()}, {"title", "e2e"}, {"source", "pbsd/kernel/include/x.h"}};
    edb.load_tasks(json::array({task}));
    auto leased = edb.lease(1, 600);
    Api ae(ce);
    auto out = run_task(leased.at(0), ae, ce, edb, -1);
    std::string last; for (int i = 20; i >= 0 && last.empty(); --i) { std::ifstream f(tmp / ("script.json.req." + std::to_string(i))); if (f) last.assign(std::istreambuf_iterator<char>(f), {}); }
    check("e2e: path escapes refused", last.find("not found or not allowed: /etc/passwd") != std::string::npos &&
          last.find("not found or not allowed: ../secret") != std::string::npos && last.find("refused: pbsd/other/a.cpp") != std::string::npos);
    check("e2e: write after a passing gate invalidates it", last.find("rejected: the gate has not passed") != std::string::npos);
    check("e2e: gates run without API keys, task passes", out.r == Result::passed && !out.gate_sha.empty());
    auto shown = run_job({"git", "show", "wp/P9-01:pbsd/kernel/x/a.cpp"}, -1, 30, ce.repo, 0);
    check("e2e: merged into wp/<WP>", shown.code == 0 && shown.tail.find("ok2") != std::string::npos);
    // Merges without a working tree: a second branch is merged with a merge commit; a conflict is reported.
    git({"branch", "x2", "main"}); git({"branch", "x3", "main"});
    fs::path w2 = tmp / "w2"; git({"worktree", "add", "-q", w2.string(), "x2"});
    std::ofstream(w2 / "b.txt") << "b"; run_job({"git", "-c", "user.name=t", "-c", "user.email=t@t", "commit", "-qam", "b", "--allow-empty"}, -1, 30, w2.string(), 0);
    run_job({"git", "add", "b.txt"}, -1, 30, w2.string(), 0); run_job({"git", "-c", "user.name=t", "-c", "user.email=t@t", "commit", "-qm", "b"}, -1, 30, w2.string(), 0);
    fs::path w3 = tmp / "w3"; git({"worktree", "add", "-q", w3.string(), "x3"}); fs::create_directories(w3 / "pbsd/kernel/x");
    std::ofstream(w3 / "pbsd/kernel/x/a.cpp") << "conflict\n";
    run_job({"git", "add", "-A"}, -1, 30, w3.string(), 0); run_job({"git", "-c", "user.name=t", "-c", "user.email=t@t", "commit", "-qm", "c"}, -1, 30, w3.string(), 0);
    std::string m2 = merge_branch(ce, "wp/P9-01", "x2", "merge x2"), m3 = merge_branch(ce, "wp/P9-01", "x3", "merge x3");
    auto both = run_job({"git", "ls-tree", "-r", "--name-only", "wp/P9-01"}, -1, 30, ce.repo, 0);
    check("merge without worktree; conflict reported", m2.empty() && both.tail.find("b.txt") != std::string::npos &&
          both.tail.find("pbsd/kernel/x/a.cpp") != std::string::npos && m3.find("conflict") != std::string::npos);
    // Reviewer verdict "changes" reopens the implementer and blocks the review until it passes again.
    json impl2{{"id", "P9-01-U002-IMPL"}, {"wp", "P9-01"}, {"phase", 9}, {"role", "implementer"}, {"owns", "pbsd/kernel/y/"}, {"deps", json::array()}};
    json proof2{{"id", "P9-01-U002-PROOF"}, {"wp", "P9-01"}, {"phase", 9}, {"role", "prover"}, {"owns", "proofs/P9-01/U002/"}, {"deps", {"P9-01-U002-IMPL"}}};
    json rev2{{"id", "P9-01-U002-REVIEW"}, {"wp", "P9-01"}, {"phase", 9}, {"role", "reviewer"}, {"owns", "-"}, {"deps", {"P9-01-U002-IMPL", "P9-01-U002-PROOF"}}};
    edb.load_tasks(json::array({impl2, proof2, rev2}));
    edb.exec("UPDATE task SET state='passed' WHERE id IN ('P9-01-U002-IMPL','P9-01-U002-PROOF')");
    edb.unblock();
    json finding = {{"target", "impl"}, {"file", "pbsd/kernel/y/y.cpp"}, {"line", 3}, {"issue", "raw pointer"}};
    json verdict = {{"summary", "needs work"}, {"verdict", "changes"}, {"findings", json::array({finding})}};
    json rscript = json::array();
    rscript.push_back(turn({call("r1", "run_gate", json::object())}));
    rscript.push_back(turn({call("r2", "submit", verdict)}));
    std::ofstream(tmp / "rscript.json") << rscript.dump();
    int pr2 = port + 4;
    pid_t fr = fork(); if (fr == 0) { execl(self, self, "fake-server", std::to_string(pr2).c_str(), "script", (tmp / "rscript.json").c_str(), (char*)nullptr); _exit(127); }
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    ce.providers[0].url = "http://127.0.0.1:" + std::to_string(pr2) + "/v1/chat/completions";
    Api ar(ce);
    auto rl = edb.lease(5, 600);
    json rtask; for (auto& x : rl) if (x["id"] == "P9-01-U002-REVIEW") rtask = x;
    auto rout = rtask.is_null() ? TaskOutcome{Result::crash, {}, ""} : run_task(rtask, ar, ce, edb, -1);
    edb.finish("P9-01-U002-REVIEW", "changes", ce);
    edb.unblock();
    auto inbox = edb.messages_for("P9-01-U002-IMPL", "P9-01");
    check("review changes reopen the unit", rout.r == Result::changes && edb.state_of("P9-01-U002-IMPL") == "ready" &&
          edb.state_of("P9-01-U002-PROOF") == "blocked" && edb.state_of("P9-01-U002-REVIEW") == "blocked" &&
          !inbox.empty() && inbox.back()["kind"] == "review_findings");
    // An infrastructure failure does not charge an attempt.
    edb.exec("UPDATE task SET state='leased', attempts=2 WHERE id='P9-01-U002-IMPL'");
    edb.finish("P9-01-U002-IMPL", "infra", ce);
    check("infra failure charges no attempt", edb.state_of("P9-01-U002-IMPL") == "ready");
    // A task that reported a proof_failure to a task that is not passed waits (blocked) instead of retrying on the same spec, and resumes when it passes.
    edb.exec("UPDATE task SET state='ready' WHERE id='P9-01-U002-IMPL'");
    edb.exec("UPDATE task SET state='leased', attempts=1 WHERE id='P9-01-U002-PROOF'");
    edb.post("P9-01-U002-IMPL", "P9-01-U002-PROOF", "proof_failure", "statement is false");
    edb.finish("P9-01-U002-PROOF", "failed", ce);
    const bool held = edb.state_of("P9-01-U002-PROOF") == "blocked";
    edb.exec("UPDATE task SET state='passed' WHERE id='P9-01-U002-IMPL'");
    edb.unblock();
    check("a proof_failure to an unfinished task blocks the sender until it passes", held && edb.state_of("P9-01-U002-PROOF") == "ready");
    kill(fr, SIGKILL); waitpid(fr, nullptr, 0);
  }
  unsetenv("E2E_SECRET_KEY");
  kill(fe, SIGKILL); waitpid(fe, nullptr, 0);
  if (!getenv("PBSD_SELFTEST_KEEP")) fs::remove_all(tmp); else std::cout << "kept " << tmp << "\n";
  return ok ? 0 : 1;
}
int main(int argc, char** argv) {
  std::string cmd = argc > 1 ? argv[1] : "status";
  Config c = Config::load();
  std::filesystem::create_directories(c.state_dir);
  std::string dbp = c.state_dir + "/tasks.db";
  // After a reboot the tmpfs is empty: restore the last HDD checkpoint of the database.
  if (!std::filesystem::exists(dbp) && !c.checkpoint_dir.empty() && std::filesystem::exists(c.checkpoint_dir + "/tasks.db")) {
    std::filesystem::copy_file(c.checkpoint_dir + "/tasks.db", dbp);
    std::cout << "restored task database from " << c.checkpoint_dir << "\n";
  }
  try {
    if (cmd == "init" || cmd == "run") {
      Db db(dbp); db.init_schema();
      std::ifstream f(c.tasks_json);
      if (!f) { std::cerr << "cannot read " << c.tasks_json << "\n"; return 1; }
      size_t n = db.load_tasks(json::parse(f));
      std::cout << n << " tasks in " << dbp << "\n";
      if (cmd == "run") return run(c);
      return 0;
    }
    if (cmd == "status") { Db db(dbp); db.init_schema(); std::cout << render(db.snapshot(), {}, c, now()); return 0; }
    if (cmd == "selftest") return selftest(c);
    if (cmd == "cpath") return cpath(c);
    if (cmd == "estimate") return estimate(c, argc, argv);
    if (cmd == "fake-server" && argc > 2)
      return fake_server(std::atoi(argv[2]), argc > 3 && std::string(argv[3]) == "429",
                         argc > 4 && std::string(argv[3]) == "script" ? argv[4] : "");
    if (cmd == "health") {
      Api api(c); bool h = api.local()->healthy();
      std::cout << c.local_url << (h ? " healthy" : " not healthy") << "\n"; return h ? 0 : 1;
    }
  } catch (const std::exception& e) { std::cerr << "error: " << e.what() << "\n"; return 1; }
  std::cerr << "usage: pbsd_run init|run|status|health|selftest|cpath|estimate [k=v ...]\n"; return 2;
}
