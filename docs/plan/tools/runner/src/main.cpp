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

// Stand-in for llama-server used by selftest: /health and /v1/chat/completions, one request at a time.
static int fake_server(int port, bool limited) {
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
    if (cmd == "fake-server" && argc > 2) return fake_server(std::atoi(argv[2]), argc > 3 && std::string(argv[3]) == "429");
    if (cmd == "health") {
      Api api(c); bool h = api.local()->healthy();
      std::cout << c.local_url << (h ? " healthy" : " not healthy") << "\n"; return h ? 0 : 1;
    }
  } catch (const std::exception& e) { std::cerr << "error: " << e.what() << "\n"; return 1; }
  std::cerr << "usage: pbsd_run init|run|status|health|selftest|cpath|estimate [k=v ...]\n"; return 2;
}
