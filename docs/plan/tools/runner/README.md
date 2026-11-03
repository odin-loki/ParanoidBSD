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

## Use

```
export DEEPSEEK_API_KEY=...            # optional DEEPSEEK_API_KEY_2 for key rotation
export PBSD_REPO=/path/to/bare-or-main-clone PBSD_WT=/run/pbsd/wt PBSD_STATE=~/.pbsd
pbsd_run init       # load docs/plan/tasks.json into SQLite (WAL, synchronous=FULL); safe to rerun
pbsd_run run        # start or resume; live progress bars, ETA, cost
pbsd_run status     # print progress once
pbsd_run selftest   # check the watchdog and the job path
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
