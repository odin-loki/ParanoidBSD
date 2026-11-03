// pbsd_run: PBSD runner and scheduler. Usage: pbsd_run init|run|status|selftest
#include "common.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>

using namespace pbsd;
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
    if (cmd == "selftest") {   // watchdog and NUMA job path
      auto r = run_job({"sh", "-c", "echo ok; sleep 5"}, -1, 1, "");
      std::cout << "watchdog " << (r.timed_out ? "ok" : "FAILED") << "\n";
      auto r2 = run_job({"sh", "-c", "echo hello"}, 0, 5, "");
      std::cout << "job " << (r2.code == 0 && r2.tail.find("hello") != std::string::npos ? "ok" : "FAILED") << "\n";
      return (r.timed_out && r2.code == 0) ? 0 : 1;
    }
  } catch (const std::exception& e) { std::cerr << "error: " << e.what() << "\n"; return 1; }
  std::cerr << "usage: pbsd_run init|run|status|selftest\n"; return 2;
}
