// pbsd_run: PBSD runner and scheduler. Usage: pbsd_run init|run|status|health|selftest
#include "common.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace pbsd;

// Stand-in for llama-server used by selftest: /health and /v1/chat/completions, one request at a time.
static int fake_server(int port) {
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
      : R"({"choices":[{"message":{"role":"assistant","content":"pong"}}],"usage":{"prompt_tokens":12,"completion_tokens":1}})";
    std::string r = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " + std::to_string(body.size()) +
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
  return ok ? 0 : 1;
}
int main(int argc, char** argv) {
  std::string cmd = argc > 1 ? argv[1] : "status";
  Config c = Config::load();
  std::filesystem::create_directories(c.state_dir);
  std::string dbp = c.state_dir + "/tasks.db";
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
    if (cmd == "fake-server" && argc > 2) return fake_server(std::atoi(argv[2]));
    if (cmd == "health") {
      Api api(c); bool h = api.local()->healthy();
      std::cout << c.local_url << (h ? " healthy" : " not healthy") << "\n"; return h ? 0 : 1;
    }
  } catch (const std::exception& e) { std::cerr << "error: " << e.what() << "\n"; return 1; }
  std::cerr << "usage: pbsd_run init|run|status|health|selftest\n"; return 2;
}
