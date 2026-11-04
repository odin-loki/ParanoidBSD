# pbsd_run (C++23)

PBSD runner, scheduler and agent harness. It replaces the deprecated Python scripts in `../deprecated/`.

## Build

Needs a C++23 compiler, CMake ≥ 3.24 and Ninja or Make. Dependencies:

| Library | Source |
|---|---|
| libcurl (with headers) | system |
| libnuma (optional) | system |
| SQLite | system if found, else the bundled amalgamation |
| nlohmann_json | system if found, else fetched |

```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release          # -DPBSD_STATIC=ON by default: one static binary
cmake --build build
```

A fully static link needs static libcurl, OpenSSL and libnuma (`.a` files). On a distribution without them, configure with `-DPBSD_STATIC=OFF`.

## Backends

`backend` in the config (or `PBSD_BACKEND`):
- `api` (default): GLM-5.3-Flash over every provider that has a key. Built in: Z.ai (`ZAI_API_KEY`), DeepInfra (`DEEPINFRA_API_KEY`), Fireworks (`FIREWORKS_API_KEY`), Together (`TOGETHER_API_KEY`), OpenRouter (`OPENROUTER_API_KEY`); `<VAR>_2` … `<VAR>_9` add keys; DeepSeek (`DEEPSEEK_API_KEY`) is a last-resort, off-peak-only provider. A `providers` array in the config replaces the built-in list; fields: `name`, `url`, `model`, `key_env`, `concurrency`, `rpm`, `tpm`, `price_in`, `price_hit`, `price_out`, `peak_double`, `offpeak_only`, `extra` (merged into the request body). Other keys: `api_max_tokens`, `api_timeout_s`, `expected_out_tokens`, `ctx_per_slot`, `tool_result_chars`.
- `local`: llama-server at `local_url` (configs and launch scripts in `../llama/`; AGENTS.md Appendix D). With `fallback_api: true`, turns go to the API providers while the server is down.

Ready-made configs: `config.glm-api.json` (24 agents, PC only) and `config.glm-api-burst.json` (128 agents, 64 gate slots on cloud CI through `burst_cmd`). Set each provider's `concurrency` at or below the account's real limit (Z.ai: console rate-limits page).

Config file: `PBSD_CONFIG`, else `$PBSD_STATE/config.json`, else `$PBSD_CHECKPOINT/config.json`.

## Use

```
export ZAI_API_KEY=... DEEPINFRA_API_KEY=...          # any subset of providers
export PBSD_STATE=/run/pbsd/state PBSD_WT=/run/pbsd/wt PBSD_REPO=/run/pbsd/repo PBSD_CHECKPOINT=/mnt/d/pbsd-ckpt
cp config.glm-api.json $PBSD_CHECKPOINT/config.json
pbsd_run cpath      # critical path and width of the task graph
pbsd_run estimate   # wall clock, binding constraint and cost per agent count (k=v overrides, e.g. burst_cores=128 out=6000)
pbsd_run init       # load docs/plan/tasks.json into SQLite (WAL); safe to rerun
pbsd_run run        # start or resume; live progress bars, ETA, cost, gate slots, providers
pbsd_run status     # print progress once
pbsd_run selftest   # watchdog, jobs, local server restart and chat, compaction, RPM/TPM buckets, failover, gate slots
```

The task database lives in `PBSD_STATE` (tmpfs by default). Every `checkpoint_s` it is copied to `checkpoint_dir` on the HDD and the repository is pushed with `--mirror` to `checkpoint_repo`; after a reboot the database is restored from the HDD copy.

## Features

- **Progress:** an overall bar and one per phase; running, queued, blocked, failed, escalated and quarantined counts; throughput over the last 24 hours, with time left and finish time; cost so far and projected total; slot use, RAM and load.
- **Fault tolerance:**
  - SQLite WAL task database with a transaction per state change and an append-only event log.
  - Expired leases are recovered on start, after an integrity check.
  - Lease heartbeats.
  - A commit at every turn end, and a `wip/<id>` ref plus saved conversation state every 10 minutes.
  - Resume from the checkpoint, with a fresh context from attempt 3 and a split suggestion from attempt 5.
  - Escalation at attempt 8, and quarantine after 3 crashes.
- **API:** per-provider in-flight cap and RPM/TPM token buckets; package affinity for prompt-cache hits; on 429/5xx/connection errors the provider cools down (Retry-After or exponential backoff with jitter) and the request fails over to the next provider; 401/403 rotate keys; 402 removes only that provider, and the run stops cleanly when all are out of credit.
- **Gates:** counting slots per kind (compile, QEMU, proof) so gates never oversubscribe the cores, with optional cloud CI burst slots (`burst_cmd`, `burst_slots`).
- **Jobs:**
  - NUMA pinning through libnuma (or numactl).
  - Each job runs in its own process group and is killed on watchdog timeout. Proofs use the task's `proof_budget_minutes`.
  - `oom_score_adj` is set to 500 for jobs, so they are killed before the database or git server.
  - Admission control on RAM, load and I/O pressure, and gates pause while RAM is above the pause level.

## Still to provide

- `tools/gates/<role>.sh <task-id> <worktree>` in the repository: the build, test, fault-injection, PRISM and QEMU commands for each role.
- The local git server setup (bare repository and `wp/*` branches).
