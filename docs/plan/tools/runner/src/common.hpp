// PBSD runner: shared declarations. See docs/plan/AGENTS.md sections 5, 8 and 9.
#pragma once
#include <atomic>
#include <chrono>
#include <cstdint>
#include <expected>
#include <map>
#include <mutex>
#include <optional>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include <sqlite3.h>

namespace pbsd {
using json = nlohmann::json;
using Clock = std::chrono::system_clock;
double now();                                      // seconds since epoch

struct Slot { int count; int cores; double ram_gb; int node; };   // node -1 = any
struct Config {
  std::string state_dir, tasks_json, repo, api_url = "https://api.deepseek.com/chat/completions",
              model = "deepseek-flash";
  std::vector<std::string> keys;
  std::map<std::string, Slot> slots{
      {"agent", {48, 0, 0.3, -1}}, {"compile", {12, 1, 2, 0}}, {"qemu", {3, 2, 4, 0}}, {"proof", {6, 2, 12, 1}}};
  double load_max = 30, ram_admit = 0.85, ram_pause = 0.92, io_some_max = 40;
  int lease_s = 45 * 60, agent_watchdog_s = 20 * 60, build_watchdog_s = 30 * 60, qemu_watchdog_s = 60 * 60;
  int checkpoint_s = 600, max_attempts = 8, quarantine_after = 3, max_turns = 40;
  static Config load();                            // env + optional $PBSD_STATE/config.json
};

// ---- task database (SQLite WAL, synchronous=FULL)
class Db {
 public:
  explicit Db(const std::string& path);
  ~Db();
  Db(const Db&) = delete;
  void exec(const std::string& sql);
  void init_schema();
  size_t load_tasks(const json& tasks);           // idempotent
  int recover();                                   // expired leases -> ready; returns count
  bool integrity_ok();
  void unblock();
  std::vector<json> lease(int n, int lease_s);
  void finish(const std::string& id, const std::string& result, const Config& c);
  void event(const std::string& task, const std::string& kind, const json& data = {});
  void cost(const std::string& task, int64_t hit, int64_t miss, int64_t out, double usd);
  void heartbeat(const std::string& id, int lease_s);
  struct Snapshot {
    std::map<std::string, int64_t> states;
    std::vector<std::tuple<int, int64_t, int64_t>> phases;   // phase, total, passed
    double cost = 0; int64_t passed_24h = 0; double first_pass = 0;
  };
  Snapshot snapshot();
  int64_t remaining();
 private:
  sqlite3* h_ = nullptr;
  std::mutex m_;
};

// ---- DeepSeek client with retry, backoff, Retry-After, breaker, key rotation
struct Usage { int64_t hit = 0, miss = 0, out = 0; bool peak = false; double usd() const; };
class Api {
 public:
  explicit Api(const Config& c);
  std::expected<json, std::string> chat(const json& messages, const json& tools, Usage& u);
  static bool peak_now();
 private:
  const Config& c_;
  std::mutex m_;
  std::vector<double> fails_;
  std::atomic<double> open_until_{0};
  size_t key_ = 0;
};

// ---- machine pressure and jobs
double mem_used_frac();
double io_pressure_some10();
double load1();
struct JobResult { int code; std::string tail; bool timed_out; };
JobResult run_job(const std::vector<std::string>& argv, int node, int timeout_s, const std::string& cwd,
                  int oom_adj = 500);

// ---- harness: one task end to end
enum class Result { passed, failed, crash };
struct TaskOutcome { Result r; std::vector<Usage> usage; };
TaskOutcome run_task(const json& task, Api& api, const Config& c, Db& db);

// ---- progress
std::string render(const Db::Snapshot& s, const std::map<std::string, int>& running, const Config& c,
                   double start_time, int width = 40);

// ---- scheduler
int run(const Config& c);
}  // namespace pbsd
