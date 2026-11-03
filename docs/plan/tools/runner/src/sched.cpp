#include "common.hpp"
#include <csignal>
#include <cstdio>
#include <iostream>
#include <thread>
#include <unistd.h>

namespace pbsd {
static std::atomic<bool> g_stop{false};
static void on_sig(int) { g_stop = true; }

int run(const Config& c) {
  Db db(c.state_dir + "/tasks.db");
  if (!db.integrity_ok()) { std::cerr << "task database integrity check failed\n"; return 2; }
  if (int n = db.recover()) db.event("", "recovered", n);
  Api api(c);
  const bool local = c.backend == "local";
  // Agents = llama-server slots when local (one conversation per slot keeps its KV prefix).
  const int agent_slots = local ? c.local_slots : c.slots.at("agent").count;
  std::vector<bool> slot_busy(agent_slots, false);
  std::atomic<bool> server_ok{true};
  if (local) {
    std::cout << "checking llama-server at " << c.local_url << " ...\n" << std::flush;
    server_ok = api.local()->ensure_up();
    if (!server_ok && !(c.fallback_api && !c.keys.empty())) {
      std::cerr << "llama-server is not healthy and cannot be restarted (set server_cmd or start it: docs/plan/tools/llama/)\n";
      return 4;
    }
    std::thread([&] {   // supervisor: health check every health_every_s, restart on failure
      while (!g_stop) {
        std::this_thread::sleep_for(std::chrono::seconds(c.health_every_s));
        bool ok = api.local()->healthy() || api.local()->ensure_up();
        if (ok != server_ok) db.event("", ok ? "server_up" : "server_down", api.local()->restarts());
        server_ok = ok;
      }
    }).detach();
  }
  std::signal(SIGINT, on_sig); std::signal(SIGTERM, on_sig);
  std::mutex m; std::map<std::string, int> running{{"agent", 0}};
  std::atomic<bool> fatal{false};
  bool tty = isatty(1); double start = now(), last = 0;
  while (!g_stop && !fatal) {
    db.unblock();
    double ram = mem_used_frac();
    int free_slots; { std::lock_guard g(m); free_slots = agent_slots - running["agent"]; }
    bool admit = ram < c.ram_admit && load1() < c.load_max && io_pressure_some10() < c.io_some_max && (local ? (server_ok || (c.fallback_api && !c.keys.empty())) : !Api::peak_now());
    if (ram > c.ram_pause) admit = false;       // jobs themselves are paused by run_job's caller (harness)
    if (admit && free_slots > 0) {
      for (auto& t : db.lease(free_slots, c.lease_s)) {
        int slot = -1;
        { std::lock_guard g(m); ++running["agent"];
          for (int i = 0; i < agent_slots; ++i) if (!slot_busy[i]) { slot_busy[i] = true; slot = i; break; } }
        std::thread([&, t, slot] {
          Db tdb(c.state_dir + "/tasks.db");
          std::string id = t["id"];
          std::string res = "crash";
          try {
            auto out = run_task(t, api, c, tdb, local ? slot : -1);
            for (auto& u : out.usage) tdb.cost(id, u.hit, u.miss, u.out, u.usd());
            res = out.r == Result::passed ? "passed" : out.r == Result::failed ? "failed" : "crash";
          } catch (const std::bad_alloc&) { res = "crash";
          } catch (const std::exception& e) {
            tdb.event(id, "error", e.what());
            if (std::string(e.what()).starts_with("402")) fatal = true;
          }
          tdb.finish(id, res, c);
          std::lock_guard g(m); --running["agent"]; if (slot >= 0) slot_busy[slot] = false;
        }).detach();
      }
    }
    if (now() - last > 2) {
      last = now();
      std::map<std::string, int> r; { std::lock_guard g(m); r = running; }
      std::string s = render(db.snapshot(), r, c, start) + "  model " + api.status_line() + "\n";
      std::cout << (tty ? "\033[H\033[J" : "") << s << std::flush;
    }
    int busy; { std::lock_guard g(m); busy = running["agent"]; }
    if (db.remaining() == 0 && busy == 0) { std::cout << render(db.snapshot(), {}, c, start) << "run complete\n"; return 0; }
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  std::cout << "\nstopping: leases will expire and tasks resume on next run\n";
  return fatal ? 3 : 0;
}
}  // namespace pbsd
