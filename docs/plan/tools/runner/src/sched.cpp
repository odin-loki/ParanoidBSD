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
  std::signal(SIGINT, on_sig); std::signal(SIGTERM, on_sig);
  std::mutex m; std::map<std::string, int> running{{"agent", 0}};
  std::atomic<bool> fatal{false};
  bool tty = isatty(1); double start = now(), last = 0;
  while (!g_stop && !fatal) {
    db.unblock();
    double ram = mem_used_frac();
    int free_slots; { std::lock_guard g(m); free_slots = c.slots.at("agent").count - running["agent"]; }
    bool admit = ram < c.ram_admit && load1() < c.load_max && io_pressure_some10() < c.io_some_max && !Api::peak_now();
    if (ram > c.ram_pause) admit = false;       // jobs themselves are paused by run_job's caller (harness)
    if (admit && free_slots > 0) {
      for (auto& t : db.lease(free_slots, c.lease_s)) {
        { std::lock_guard g(m); ++running["agent"]; }
        std::thread([&, t] {
          Db tdb(c.state_dir + "/tasks.db");
          std::string id = t["id"];
          std::string res = "crash";
          try {
            auto out = run_task(t, api, c, tdb);
            for (auto& u : out.usage) tdb.cost(id, u.hit, u.miss, u.out, u.usd());
            res = out.r == Result::passed ? "passed" : out.r == Result::failed ? "failed" : "crash";
          } catch (const std::bad_alloc&) { res = "crash";
          } catch (const std::exception& e) {
            tdb.event(id, "error", e.what());
            if (std::string(e.what()).starts_with("402")) fatal = true;
          }
          tdb.finish(id, res, c);
          std::lock_guard g(m); --running["agent"];
        }).detach();
      }
    }
    if (now() - last > 2) {
      last = now();
      std::map<std::string, int> r; { std::lock_guard g(m); r = running; }
      std::string s = render(db.snapshot(), r, c, start);
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
