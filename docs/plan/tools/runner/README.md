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

`backend` in `$PBSD_STATE/config.json` (or `PBSD_BACKEND`):
- `local` (default): llama-server at `local_url` (default `http://127.0.0.1:8080/v1`, or `PBSD_LOCAL_URL`). Ready-made configs for both quants and the launch scripts are in `../llama/`. Keys: `local_model`, `local_slots`, `ctx_per_slot`, `max_tokens`, `tool_result_chars`, `request_timeout_s`, `health_every_s`, `load_timeout_s`, `restart_max`, `server_cmd`, `compact_prompt`, `aux_url`, `aux_model`, `role_backend`.
- `api`: DeepSeek Flash (needs `DEEPSEEK_API_KEY`). With `fallback_api: true`, a local run uses the API only while llama-server is down.

`pbsd_run health` checks the server. `pbsd_run selftest` starts a stand-in server (`pbsd_run fake-server PORT`) to test the restart, chat and context-compaction paths.

## Use

```
cp ../llama/config.q4km.json $PBSD_STATE/config.json   # local GLM-5.3-Flash (default)
export DEEPSEEK_API_KEY=...            # only for backend=api or fallback_api; optional DEEPSEEK_API_KEY_2
export PBSD_REPO=/path/to/bare-or-main-clone PBSD_WT=/run/pbsd/wt PBSD_STATE=~/.pbsd
pbsd_run init       # load docs/plan/tasks.json into SQLite (WAL, synchronous=FULL); safe to rerun
pbsd_run run        # start or resume; live progress bars, ETA, cost
pbsd_run status     # print progress once
pbsd_run health     # is llama-server up
pbsd_run selftest   # watchdog, job path, server restart, local chat, context compaction
```

To change slot counts, NUMA nodes and limits, write `$PBSD_STATE/config.json`, for example:

```json
{"slots": {"agent": {"count": 48}, "compile": {"count": 12, "cores": 1, "ram_gb": 2, "node": 0},
           "qemu": {"count": 3, "cores": 2, "ram_gb": 4, "node": 0},
           "proof": {"count": 8, "cores": 2, "ram_gb": 48, "node": 1}},
 "ram_admit": 0.85, "ram_pause": 0.92, "load_max": 30}
```

## Features

- **Progress:** an overall bar and one per phase; running, queued, blocked, failed, escalated and quarantined counts; throughput over the last 24 hours, with time left and finish time; cost so far and projected total; slot use, RAM and load.
- **Fault tolerance:**
  - SQLite WAL task database with a transaction per state change and an append-only event log.
  - Expired leases are recovered on start, after an integrity check.
  - Lease heartbeats.
  - A commit at every turn end, and a `wip/<id>` ref plus saved conversation state every 10 minutes.
  - Resume from the checkpoint, with a fresh context from attempt 3 and a split suggestion from attempt 5.
  - Escalation at attempt 8, and quarantine after 3 crashes.
- **API:** off-peak only, so no requests during peak hours. Retries use exponential backoff with jitter and honour Retry-After. A circuit breaker trips after 20 failures in a minute, the runner rotates to the second key on 401, and it stops cleanly on 402.
- **Jobs:**
  - NUMA pinning through libnuma (or numactl).
  - Each job runs in its own process group and is killed on watchdog timeout. Proofs use the task's `proof_budget_minutes`.
  - `oom_score_adj` is set to 500 for jobs, so they are killed before the database or git server.
  - Admission control on RAM, load and I/O pressure, and gates pause while RAM is above the pause level.

## Still to provide

- `tools/gates/<role>.sh <task-id> <worktree>` in the repository: the build, test, fault-injection, PRISM and QEMU commands for each role.
- The local git server setup (bare repository and `wp/*` branches).
