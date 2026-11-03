// PBSD runner: shared declarations. See docs/plan/AGENTS.md sections 5, 8 and 9.
#pragma once
#include <atomic>
#include <chrono>
#include <cstdint>
#include <expected>
#include <map>
#include <memory>
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
  // Backend. "local" (default): llama-server's OpenAI-compatible API on this PC. "api": DeepSeek.
  // fallback_api: when the local server stays down after restart_max restarts and a key is set,
  // send turns to the API until the local server is healthy again.
  std::string backend = "local", local_url = "http://127.0.0.1:8080/v1", local_model = "GLM-5.3-Flash";
  std::vector<std::string> server_cmd;           // how to (re)start llama-server; empty = never restart
  int local_slots = 4;                           // must equal llama-server --parallel
  int ctx_per_slot = 32768;                      // llama-server --ctx-size / --parallel
  int max_tokens = 2048;                         // output cap per turn
  int tool_result_chars = 24000;                 // about 6k tokens per tool result
  int request_timeout_s = 3600;                  // a slow CPU turn can take tens of minutes
  int health_every_s = 30, load_timeout_s = 3600, restart_max = 5;
  bool fallback_api = false, compact_prompt = true;
  std::map<std::string, std::string> role_backend;   // e.g. {"reviewer": "aux"}; default = backend
  std::string aux_url, aux_model;                // optional second local model (Qwen 3.5 9B)
  std::map<std::string, Slot> slots{
      {"agent", {4, 0, 0.3, -1}}, {"compile", {4, 1, 2, 0}}, {"qemu", {1, 2, 4, 0}}, {"proof", {2, 2, 12, 1}}};
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
struct Usage { int64_t hit = 0, miss = 0, out = 0; bool peak = false, local = false; double usd() const; };
struct Http { long code; std::string body; double retry_after; bool conn_error; };
Http http_post(const std::string& url, const std::string& payload, const std::string& bearer, long timeout_s);
Http http_get(const std::string& url, long timeout_s);

// llama-server supervision: health check (GET /health), restart with server_cmd when it is down
// or hung, wait for the model to load. One instance shared by all agent threads.
class LocalServer {
 public:
  LocalServer(std::string base_url, std::vector<std::string> cmd, int load_timeout_s, int restart_max);
  ~LocalServer();
  bool healthy();                                 // 200 from /health
  bool ensure_up();                               // restart if needed; false once restart_max is spent
  int restarts() const { return restarts_; }
  bool given_up() const { return given_up_; }
 private:
  std::string base_, health_;
  std::vector<std::string> cmd_;
  int load_timeout_s_, restart_max_;
  std::atomic<int> restarts_{0};
  std::atomic<bool> given_up_{false};
  pid_t pid_ = -1;
  std::mutex m_;
};

class Api {
 public:
  explicit Api(const Config& c);
  // slot: llama-server slot to pin this conversation to, so its KV prefix cache is reused.
  std::expected<json, std::string> chat(const json& messages, const json& tools, Usage& u,
                                        const std::string& role = "", int slot = -1);
  static bool peak_now();
  LocalServer* local() { return local_.get(); }
  std::string status_line();                      // backend, tok/s, restarts
 private:
  std::expected<json, std::string> chat_api(const std::string& payload, Usage& u);
  std::expected<json, std::string> chat_local(const std::string& url, json body, Usage& u, int slot,
                                              LocalServer* srv);
  const Config& c_;
  std::mutex m_;
  std::vector<double> fails_;
  std::atomic<double> open_until_{0};
  size_t key_ = 0;
  std::unique_ptr<LocalServer> local_, aux_;
  std::atomic<int64_t> out_tok_{0}, prompt_tok_{0};
  std::atomic<double> gen_s_{0};
  std::atomic<int64_t> fallback_turns_{0};
};

// Keep a conversation inside the slot's context: older tool results are elided first.
size_t approx_tokens(const json& msgs);
int compact_history(json& msgs, size_t budget_tokens);

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
TaskOutcome run_task(const json& task, Api& api, const Config& c, Db& db, int slot = -1);

// ---- progress
std::string render(const Db::Snapshot& s, const std::map<std::string, int>& running, const Config& c,
                   double start_time, int width = 40);

// ---- scheduler
int run(const Config& c);
int selftest(const Config& c);
}  // namespace pbsd
