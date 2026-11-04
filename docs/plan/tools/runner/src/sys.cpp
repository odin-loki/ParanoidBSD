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
  c.tasks_json = env("PBSD_TASKS", "docs/plan/tasks.json");
  c.repo = env("PBSD_REPO", ".");
  for (auto k : {"DEEPSEEK_API_KEY", "DEEPSEEK_API_KEY_2"}) if (auto v = env(k); !v.empty()) c.keys.push_back(v);
  c.state_dir = env("PBSD_STATE", "/run/pbsd/state");   // tmpfs: the run lives in RAM
  c.checkpoint_dir = env("PBSD_CHECKPOINT", "");         // e.g. /mnt/d/pbsd-ckpt (the HDD)
  // Built-in GLM-5.3-Flash providers; each is used only when its key variable is set. Limits are
  // conservative starting caps: set them to what each account's console shows (AGENTS.md 8.2).
  // Model ids other than Z.ai's and OpenRouter's are unverified: check each provider's catalog.
  auto prov = [&](std::string name, std::string url, std::string model, std::string key_env, int conc, int rpm, int64_t tpm) {
    Provider p; p.name = name; p.url = url; p.model = model; p.key_env = key_env; p.concurrency = conc; p.rpm = rpm; p.tpm = tpm;
    return p;
  };
  std::vector<Provider> builtin{
      prov("zai", "https://api.z.ai/api/paas/v4/chat/completions", "glm-5.3-flash", "ZAI_API_KEY", 8, 0, 0),
      prov("deepinfra", "https://api.deepinfra.com/v1/openai/chat/completions", "zai-org/GLM-5.3-Flash", "DEEPINFRA_API_KEY", 64, 0, 0),
      prov("fireworks", "https://api.fireworks.ai/inference/v1/chat/completions", "accounts/fireworks/models/glm-5p3-flash", "FIREWORKS_API_KEY", 32, 0, 2'000'000),
      prov("together", "https://api.together.xyz/v1/chat/completions", "zai-org/GLM-5.3-Flash", "TOGETHER_API_KEY", 32, 0, 0),
      prov("openrouter", "https://openrouter.ai/api/v1/chat/completions", "z-ai/glm-5.3-flash", "OPENROUTER_API_KEY", 32, 0, 0)};
  auto keys_of = [&](Provider& p) {
    p.keys.clear();
    if (auto v = env(p.key_env.c_str()); !v.empty()) p.keys.push_back(v);
    for (int i = 2; i < 10; ++i) if (auto v = env((p.key_env + "_" + std::to_string(i)).c_str()); !v.empty()) p.keys.push_back(v);
  };
  for (auto& p : builtin) { keys_of(p); if (!p.keys.empty()) c.providers.push_back(p); }
  if (!c.keys.empty()) {   // DeepSeek Flash kept as a last-resort provider (Appendix C prices, off-peak only)
    Provider d = prov("deepseek", c.api_url, c.model, "DEEPSEEK_API_KEY", 64, 0, 0);
    d.keys = c.keys; d.price_in = 0.15; d.price_hit = 0.003; d.price_out = 0.60; d.peak_double = true; d.offpeak_only = true;
    c.providers.push_back(d);
  }
  c.backend = env("PBSD_BACKEND", c.backend.c_str());
  c.local_url = env("PBSD_LOCAL_URL", c.local_url.c_str());
  // PBSD_CONFIG, else $PBSD_STATE/config.json, else the HDD checkpoint copy (tmpfs is empty after a reboot).
  std::string cfg = env("PBSD_CONFIG", "");
  if (cfg.empty()) cfg = c.state_dir + "/config.json";
  if (!std::ifstream(cfg) && !c.checkpoint_dir.empty()) cfg = c.checkpoint_dir + "/config.json";
  std::ifstream f(cfg);
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
      c.api_max_tokens = j.value("api_max_tokens", c.api_max_tokens); c.api_timeout_s = j.value("api_timeout_s", c.api_timeout_s);
      c.expected_out_tokens = j.value("expected_out_tokens", c.expected_out_tokens);
      c.checkpoint_dir = j.value("checkpoint_dir", c.checkpoint_dir); c.checkpoint_repo = j.value("checkpoint_repo", c.checkpoint_repo);
      c.checkpoint_s = j.value("checkpoint_s", c.checkpoint_s); c.burst_slots = j.value("burst_slots", c.burst_slots);
      c.lease_s = j.value("lease_s", c.lease_s);
      if (j.contains("burst_cmd")) c.burst_cmd = j["burst_cmd"].get<std::vector<std::string>>();
      // "providers" replaces the built-in list. Fields: name, url, model, key_env, concurrency, rpm,
      // tpm, price_in, price_hit, price_out, peak_double, offpeak_only, extra.
      if (j.contains("providers") && j["providers"].is_array()) {
        c.providers.clear();
        for (auto& v : j["providers"]) {
          Provider p; p.name = v.value("name", "provider"); p.url = v.value("url", ""); p.model = v.value("model", p.model);
          p.key_env = v.value("key_env", ""); p.concurrency = v.value("concurrency", p.concurrency); p.rpm = v.value("rpm", 0);
          p.tpm = v.value("tpm", int64_t(0)); p.price_in = v.value("price_in", p.price_in); p.price_hit = v.value("price_hit", p.price_hit);
          p.price_out = v.value("price_out", p.price_out); p.peak_double = v.value("peak_double", false);
          p.offpeak_only = v.value("offpeak_only", false); if (v.contains("extra")) p.extra = v["extra"];
          keys_of(p);
          if (v.contains("keys")) for (auto& k : v["keys"]) p.keys.push_back(k.get<std::string>());   // tests only; use key_env
          if (!p.keys.empty() && !p.url.empty()) c.providers.push_back(p);
        }
      }
      if (j.contains("role_backend")) c.role_backend = j["role_backend"].get<std::map<std::string, std::string>>();
    }
  }
  return c;
}
}  // namespace pbsd
