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
// One OpenAI-compatible endpoint serving one GLM model (Z.ai, DeepInfra, Fireworks, Together,
// OpenRouter, ...). Limits are client-side caps set at or below what the provider grants the account;
// 0 = no cap of that kind. Prices are USD per 1M tokens. `tier` says which model it serves:
// "frontier" (the flagship GLM) or "flash" (GLM-5.3-Flash); roles pick a tier (Config::role_tier).
struct Provider {
  std::string name, url, model = "glm-5.3-flash", key_env, tier = "flash";
  std::vector<std::string> keys;                 // from key_env, key_env_2, ... ; rotated on 401
  int concurrency = 8, rpm = 0;                  // in-flight cap, requests per minute
  double weight = 1;                             // share of packages that start here; 0 = overflow and failover only
  int64_t tpm = 0;                               // input + output tokens per minute
  double price_in = 0.15, price_hit = 0.03, price_out = 0.50;
  bool peak_double = false, offpeak_only = false;   // DeepSeek-style peak pricing
  json extra = json::object();                   // merged into the request body (reasoning, routing)
};
struct Config {
  std::string state_dir, tasks_json, repo, api_url = "https://api.deepseek.com/chat/completions",
              model = "deepseek-flash";
  std::vector<std::string> keys;                 // legacy DeepSeek keys (provider "deepseek" if set)
  // Backend. "api" (default): GLM-5.3-Flash over the providers below, with per-provider token
  // buckets and failover. "local": llama-server on this PC (appendix plan). fallback_api: a local
  // run sends turns to the API providers while llama-server is down.
  std::vector<Provider> providers;
  std::vector<Provider> catalog;                 // every configured provider, with or without a key (for estimate)
  int api_max_tokens = 16384;                    // output cap per API turn (reasoning included)
  int api_timeout_s = 900;
  int expected_out_tokens = 4000;                // TPM reservation per request before usage is known
  std::string checkpoint_dir;                    // HDD copy of the DB and the git mirror (empty = off)
  std::string checkpoint_repo;                   // bare repo on the HDD, pushed with --mirror
  std::vector<std::string> burst_cmd;            // prefix for gates on cloud CI, e.g. ["ssh","ci1","--"]
  int burst_slots = 0;                           // gates that may run through burst_cmd at once
  // Cloud gate workers (replaces the single burst_cmd host). One line per worker in workers_file:
  // "<ssh-host> <slots>"; re-read every 30 s so bootstrap can register workers mid-run. A burst gate
  // pushes the task commit to mirror_url as refs/gates/<id>, then runs
  // ssh <host> -- /opt/pbsd/bin/pbsd-gate <role> <id> <sha> on the least-busy worker.
  std::string workers_file;                      // e.g. ~/.pbsd/workers.txt (empty = off)
  std::string mirror_url;                        // bare repo the workers fetch from, e.g. /srv/pbsd/pbsd.git
  std::string remote_gate = "/opt/pbsd/bin/pbsd-gate";
  std::string backend = "api", local_url = "http://127.0.0.1:8080/v1", local_model = "GLM-5.3-Flash";
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
      {"agent", {32, 0, 0.3, -1}}, {"compile", {8, 1, 3, 0}}, {"qemu", {2, 2, 4, 0}}, {"proof", {9, 2, 12, -1}}};
  double load_max = 30, ram_admit = 0.85, ram_pause = 0.92, io_some_max = 40;
  int lease_s = 45 * 60, agent_watchdog_s = 20 * 60, build_watchdog_s = 30 * 60, qemu_watchdog_s = 60 * 60;
  int checkpoint_s = 600, max_attempts = 8, quarantine_after = 3, max_turns = 40;
  // ---- model tiers. A role runs on role_tier[role] (else default_tier); from attempt escalate_at
  // every role moves to escalate_tier. Providers of a tier fail over to each other only.
  std::map<std::string, std::string> role_tier;
  std::string default_tier = "flash", escalate_tier = "flash";   // GLM-5.3-Flash everywhere unless configured
  int escalate_at = 3;
  std::map<std::string, int> role_max_turns{{"decomposer", 30}, {"implementer", 60}, {"tester", 60},
                                            {"prover", 100}, {"reviewer", 24}, {"integrator", 80}};
  // ---- worktrees, references, gates and merges
  // Sparse pattern for task worktrees (and gate checkouts): everything except the big read-only
  // reference trees, which agents read through read_file("ref:<name>/...") or the main checkout.
  std::vector<std::string> wt_sparse{"/*", "!/hbsd/", "!/kde/", "!/linux/", "!/docs/migration/", "!/docs/verify/", "!/docs/PORT_PLAN.md",
                                      "!/docs/plan/tasks.yaml", "!/docs/plan/tasks.json"};   // the 10 MB task list: agents read it for 20 turns instead of writing
  std::map<std::string, std::string> ref_roots;  // e.g. {"linux": "/srv/ref/linux", "wine": "/srv/ref/wine"}
  std::string gates_dir;                         // trusted gate scripts; default $PBSD_REPO/tools/gates
  std::vector<std::string> gate_wrap;            // sandbox prefix for local gates, e.g. ["bwrap", ...]
  bool gate_scrub_env = true;                    // local gates get no API keys or cloud credentials
  int gate_infra_retries = 3;                    // a gate exiting 3 is retried, on another worker if any
  std::string merge_target = "pbsd-cutover";     // integrators and -IF tasks merge into this branch
  std::vector<std::string> backup_cmd;           // e.g. ["gsutil","-m","rsync","-r","/srv/pbsd/ckpt","gs://..."]
  int backup_every_s = 3600;
  // ---- Lean proofs: `prism prove FILE.lean THEOREM` (PRISM on FORGE with the Goedel prover model,
  // PRISM_PROVER_FPK). The lean_prove tool is offered to prover tasks when PRISM_PROVER_FPK is set.
  std::string prism_bin = "prism";
  int prove_timeout_s = 1800;
  // `prism prove` loads its own copy of the prover model on the GPU (8B about 6.7 GB, 32B about 22 GB of a 24 GB card), so the number
  // running at once is limited here, not by the agent count. PBSD_PROVER_SLOTS / "prover_slots" in the config.
  int prover_slots = 1;
  // GLM-5.3-Flash is a reasoning model whose effort defaults to the maximum. Measured on Morph (6 Nov 2026): the same short Lean request spent
  // 8000 tokens (the whole cap) on reasoning in 97 s and returned no answer at the default, and answered in 3.7 s with 392 tokens at "low".
  // At the default a turn can take 45 minutes and an agent never acts. Sent as "reasoning_effort" (low | high | max); empty = the provider default.
  // PBSD_REASONING_EFFORT / "reasoning_effort", with per-role overrides in "role_reasoning_effort".
  std::string reasoning_effort;
  std::map<std::string, std::string> role_reasoning_effort;
  // Shared Lake packages (Mathlib and friends, several GB), linked into a model project's .lake/packages instead of fetched into every
  // worktree. PBSD_LEAN_PACKAGES / "lean_packages"; empty = leave projects alone.
  std::string lean_packages;
  // When set, a `prism prove` that ends "not proved" is retried once with this model (PRISM_PROVER_FPK is replaced for the retry).
  // PBSD_PROVER_FPK_ESCALATE / "prover_fpk_escalate".
  std::string prover_fpk_escalate;
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
  // result: passed | failed | crash | infra (no attempt charged) | changes (reviewer asked for changes)
  void finish(const std::string& id, const std::string& result, const Config& c);
  // Reviewer findings or a contract change: tasks in `ready` go back to ready, tasks in `block` go
  // back to blocked (and are released again by unblock() once their deps pass).
  void reopen(const std::vector<std::string>& ready, const std::vector<std::string>& block);
  // Typed messages between tasks (AGENTS.md section 2). `to` is a task id or a package id.
  void post(const std::string& to, const std::string& from, const std::string& kind, const std::string& body);
  std::vector<json> messages_for(const std::string& id, const std::string& wp, int limit = 30);
  std::string state_of(const std::string& id);
  void event(const std::string& task, const std::string& kind, const json& data = {});
  void cost(const std::string& task, int64_t hit, int64_t miss, int64_t out, double usd, const std::string& provider = "");
  void heartbeat(const std::string& id, int lease_s);
  struct Snapshot {
    std::map<std::string, int64_t> states;
    std::vector<std::tuple<int, int64_t, int64_t>> phases;   // phase, total, passed
    double cost = 0; int64_t passed_24h = 0; double first_pass = 0;
  };
  Snapshot snapshot();
  int64_t remaining();
  bool backup_to(const std::string& path);         // online copy (sqlite3 backup API) to the HDD
 private:
  sqlite3* h_ = nullptr;
  std::mutex m_;
};

// ---- DeepSeek client with retry, backoff, Retry-After, breaker, key rotation
struct Usage {
  int64_t hit = 0, miss = 0, out = 0; bool peak = false, local = false;
  double p_in = 0.15, p_hit = 0.03, p_out = 0.50;  // set from the provider that served the turn
  std::string provider;
  double usd() const;
};

// Client-side limiter for one provider: in-flight cap plus token buckets on RPM and TPM, and a
// cooldown after 429/5xx. try_acquire never blocks; Api::chat_api waits and retries across providers.
class RateLimiter {
 public:
  RateLimiter(int concurrency, int rpm, int64_t tpm);
  bool try_acquire(int64_t est_tokens, double t);
  void release(int64_t est_tokens, int64_t actual_tokens, double t);   // refunds or charges the difference
  void cooldown(double until) { double c = until_.load(); while (until > c && !until_.compare_exchange_weak(c, until)) {} }
  bool cooling(double t) const { return t < until_.load(); }
  int in_flight() const { return inflight_; }
  int limit() const { return conc_; }
 private:
  void refill(double t);
  std::mutex m_;
  int conc_, inflight_ = 0, rpm_; int64_t tpm_;
  double req_tokens_, tok_tokens_, last_ = 0;
  std::atomic<double> until_{0};
};
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
  // affinity: tasks with the same key (the work package) go to the same provider first, so the
  // provider's prompt cache keeps the shared prefix warm.
  // tier: "frontier" or "flash"; only providers of that tier are used (any provider if none has it).
  std::expected<json, std::string> chat(const json& messages, const json& tools, Usage& u,
                                        const std::string& role = "", int slot = -1,
                                        const std::string& affinity = "", const std::string& tier = "");
  static bool peak_now();
  LocalServer* local() { return local_.get(); }
  std::string status_line();                      // backend, tok/s, restarts
 private:
  std::expected<json, std::string> chat_api(json body, Usage& u, const std::string& affinity, const std::string& tier = "");
  struct ProvState { RateLimiter lim; size_t key = 0; std::atomic<int64_t> ok{0}, limited{0}, failed{0};
                     ProvState(const Provider& p) : lim(p.concurrency, p.rpm, p.tpm) {} };
  std::vector<std::unique_ptr<ProvState>> ps_;
  std::expected<json, std::string> chat_local(const std::string& url, json body, Usage& u, int slot,
                                              LocalServer* srv);
  const Config& c_;
  std::mutex m_;
  std::unique_ptr<LocalServer> local_, aux_;
  std::atomic<int64_t> out_tok_{0}, prompt_tok_{0};
  std::atomic<double> gen_s_{0};
  std::atomic<int64_t> fallback_turns_{0};
};

// Order in which providers are tried for one request: the providers of the tier with weight > 0,
// starting at a weighted choice by the affinity key (a package keeps its provider, so its prompt
// cache stays warm), then the weight-0 providers (overflow and failover only), in config order.
std::vector<size_t> provider_order(const std::vector<Provider>& ps, const std::string& affinity, const std::string& tier);

// Keep a conversation inside the slot's context: older tool results are elided first.
size_t approx_tokens(const json& msgs);
int compact_history(json& msgs, size_t budget_tokens);

// ---- machine pressure and jobs
double mem_used_frac();
double io_pressure_some10();
double load1();
struct JobResult { int code; std::string tail; bool timed_out; };
// env: when set, the job gets exactly this environment (KEY=VALUE strings) instead of the runner's.
JobResult run_job(const std::vector<std::string>& argv, int node, int timeout_s, const std::string& cwd,
                  int oom_adj = 500, const std::vector<std::string>* env = nullptr);
std::vector<std::string> gate_env(const std::string& build_dir);   // PATH, HOME, locale, TMPDIR, PBSD_* only

// ---- harness: one task end to end
// infra: an API outage or a gate infrastructure error; the task is retried without charging an attempt.
// changes: a reviewer asked for changes; the unit's tasks named in the findings are reopened.
enum class Result { passed, failed, crash, infra, changes };
struct TaskOutcome { Result r; std::vector<Usage> usage; std::string gate_sha; };
// "a/, b/{x,y}/" or ["a/", ...] -> {"a/", "b/x/", "b/y/"}; "-" and free text ("toolchain flags") dropped.
std::vector<std::string> owns_list(const json& task);
std::string role_tier(const json& task, const Config& c);       // tier for this task and attempt
// Merge branch src into dst without a working tree (git merge-tree --write-tree), with a
// compare-and-swap ref update. Returns an empty string on success, else the conflict or error.
std::string merge_branch(const Config& c, const std::string& dst, const std::string& src, const std::string& msg);
TaskOutcome run_task(const json& task, Api& api, const Config& c, Db& db, int slot = -1);
// Gate admission: counting slots per kind (compile, qemu, proof) plus optional cloud burst slots.
struct GateLease { std::string kind; bool burst = false; std::string host; };
// workers_file line: "<ssh-host> <slots> [kvm]". QEMU gates go only to workers marked kvm (GCP's
// AMD machines, for example, have no nested virtualization) or to the local qemu slots.
struct Worker { std::string host; int slots = 0; bool kvm = false; };
std::vector<Worker> load_workers(const std::string& path);
GateLease gate_acquire(const std::string& kind, const Config& c);
void gate_release(const GateLease& g);
std::string gate_status(const Config& c);

// ---- plan analysis (pbsd_run cpath / estimate)
int cpath(const Config& c);
int estimate(const Config& c, int argc, char** argv);

// ---- progress
std::string render(const Db::Snapshot& s, const std::map<std::string, int>& running, const Config& c,
                   double start_time, int width = 40);

// ---- scheduler
int run(const Config& c);
int selftest(const Config& c);
}  // namespace pbsd
