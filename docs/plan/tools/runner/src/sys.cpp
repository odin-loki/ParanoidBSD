#include "common.hpp"
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <csignal>
#include <fcntl.h>
#include <poll.h>
#include <sys/wait.h>
#include <unistd.h>
#ifdef PBSD_HAVE_NUMA
#include <numa.h>
#endif

namespace pbsd {
double now() { return std::chrono::duration<double>(Clock::now().time_since_epoch()).count(); }

double mem_used_frac() {
  std::ifstream f("/proc/meminfo"); std::string k; long v; std::string u; long tot = 0, avail = 0;
  while (f >> k >> v >> u) { if (k == "MemTotal:") tot = v; else if (k == "MemAvailable:") avail = v; }
  return tot ? 1.0 - double(avail) / double(tot) : 0.0;
}
double io_pressure_some10() {
  std::ifstream f("/proc/pressure/io"); std::string w;
  while (f >> w) if (w.rfind("avg10=", 0) == 0) return std::stod(w.substr(6));
  return 0;
}
double load1() { double l[1]{}; return getloadavg(l, 1) == 1 ? l[0] : 0; }

// Fork, pin to NUMA node (libnuma, else numactl), raise oom_score_adj so jobs die before the
// database or git server, kill the whole process group on watchdog timeout.
JobResult run_job(const std::vector<std::string>& argv, int node, int timeout_s, const std::string& cwd, int oom_adj) {
  int fd[2]; if (pipe(fd) != 0) return {-1, "pipe failed", false};
  pid_t pid = fork();
  if (pid < 0) return {-1, "fork failed", false};
  if (pid == 0) {
    setpgid(0, 0);
    dup2(fd[1], 1); dup2(fd[1], 2); close(fd[0]); close(fd[1]);
    if (!cwd.empty() && chdir(cwd.c_str()) != 0) _exit(127);
    if (FILE* o = fopen("/proc/self/oom_score_adj", "w")) { fprintf(o, "%d", oom_adj); fclose(o); }
    std::vector<std::string> a = argv;
#ifdef PBSD_HAVE_NUMA
    if (node >= 0 && numa_available() >= 0) { numa_run_on_node(node); numa_set_preferred(node); }
#else
    if (node >= 0 && access("/usr/bin/numactl", X_OK) == 0)
      a.insert(a.begin(), {"/usr/bin/numactl", "--cpunodebind=" + std::to_string(node), "--membind=" + std::to_string(node)});
#endif
    std::vector<char*> c; for (auto& s : a) c.push_back(s.data()); c.push_back(nullptr);
    execvp(c[0], c.data()); _exit(127);
  }
  close(fd[1]);
  std::string out; char buf[8192]; double end = now() + timeout_s; bool to = false;
  pollfd p{fd[0], POLLIN, 0};
  for (;;) {
    int ms = int((end - now()) * 1000);
    if (ms <= 0) { to = true; kill(-pid, SIGKILL); break; }
    int r = poll(&p, 1, std::min(ms, 1000));
    if (r > 0) { ssize_t n = read(fd[0], buf, sizeof buf); if (n <= 0) break; out.append(buf, n);
                 if (out.size() > 200000) out.erase(0, out.size() - 100000); }
  }
  close(fd[0]);
  int st = 0; waitpid(pid, &st, 0);
  if (out.size() > 20000) out = out.substr(out.size() - 20000);
  return {to ? 124 : (WIFEXITED(st) ? WEXITSTATUS(st) : 128 + WTERMSIG(st)), to ? out + "\n[watchdog timeout]" : out, to};
}

Config Config::load() {
  Config c;
  auto env = [](const char* k, const char* d = "") { const char* v = getenv(k); return std::string(v ? v : d); };
  std::string home = env("HOME", ".");
  c.state_dir = env("PBSD_STATE", (home + "/.pbsd").c_str());
  c.tasks_json = env("PBSD_TASKS", "docs/plan/tasks.json");
  c.repo = env("PBSD_REPO", ".");
  for (auto k : {"DEEPSEEK_API_KEY", "DEEPSEEK_API_KEY_2"}) if (auto v = env(k); !v.empty()) c.keys.push_back(v);
  c.backend = env("PBSD_BACKEND", c.backend.c_str());
  c.local_url = env("PBSD_LOCAL_URL", c.local_url.c_str());
  std::ifstream f(c.state_dir + "/config.json");
  if (f) {
    json j = json::parse(f, nullptr, false);
    if (j.is_object()) {
      if (j.contains("slots")) for (auto& [k, v] : j["slots"].items())
        c.slots[k] = {v.value("count", 1), v.value("cores", 1), v.value("ram_gb", 1.0), v.value("node", -1)};
      c.load_max = j.value("load_max", c.load_max); c.ram_admit = j.value("ram_admit", c.ram_admit);
      c.ram_pause = j.value("ram_pause", c.ram_pause); c.model = j.value("model", c.model);
      c.backend = j.value("backend", c.backend); c.local_url = j.value("local_url", c.local_url);
      c.local_model = j.value("local_model", c.local_model); c.local_slots = j.value("local_slots", c.local_slots);
      c.ctx_per_slot = j.value("ctx_per_slot", c.ctx_per_slot); c.max_tokens = j.value("max_tokens", c.max_tokens);
      c.tool_result_chars = j.value("tool_result_chars", c.tool_result_chars);
      c.request_timeout_s = j.value("request_timeout_s", c.request_timeout_s);
      c.health_every_s = j.value("health_every_s", c.health_every_s); c.load_timeout_s = j.value("load_timeout_s", c.load_timeout_s);
      c.restart_max = j.value("restart_max", c.restart_max); c.fallback_api = j.value("fallback_api", c.fallback_api);
      c.compact_prompt = j.value("compact_prompt", c.compact_prompt);
      c.aux_url = j.value("aux_url", c.aux_url); c.aux_model = j.value("aux_model", c.aux_model);
      c.max_turns = j.value("max_turns", c.max_turns); c.lease_s = j.value("lease_s", c.lease_s);
      c.agent_watchdog_s = j.value("agent_watchdog_s", c.agent_watchdog_s);
      if (j.contains("server_cmd")) c.server_cmd = j["server_cmd"].get<std::vector<std::string>>();
      if (j.contains("role_backend")) c.role_backend = j["role_backend"].get<std::map<std::string, std::string>>();
    }
  }
  return c;
}
}  // namespace pbsd
