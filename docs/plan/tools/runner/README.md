# pbsd_run (C++23)

PBSD runner, scheduler and agent harness. It replaces the deprecated Python scripts in `../deprecated/`. The plan it runs is `../../AGENTS.md`.

## Build

Needs a C++23 compiler (GCC 13+, or Clang 19+: Clang 18 with libstdc++ 13 has no `std::expected`), CMake ≥ 3.24 and Ninja or Make. Dependencies:

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

## Backends and models

**`backend`** in the config (or `PBSD_BACKEND`):
- **`api` (default):** GLM-5.3-Flash over every provider that has a key.
  - Built in: Z.ai (`ZAI_API_KEY`), DeepInfra (`DEEPINFRA_API_KEY`), Fireworks (`FIREWORKS_API_KEY`), Together (`TOGETHER_API_KEY`), OpenRouter (`OPENROUTER_API_KEY`).
  - `<VAR>_2` … `<VAR>_9` add keys.
  - GLM-5.3 entries for the same keys serve the optional `frontier` tier.
  - DeepSeek (`DEEPSEEK_API_KEY`) is a last-resort, off-peak-only provider.
- **`local`:** llama-server at `local_url` (configs and launch scripts in `../llama/`; AGENTS.md Appendix D). With `fallback_api: true`, turns go to the API providers while the server is down.

**`providers`:** a `providers` array in the config replaces the built-in list. Fields: `name`, `url`, `model`, `key_env`, `tier` (`flash` or `frontier`), `weight`, `concurrency`, `rpm`, `tpm`, `price_in`, `price_hit`, `price_out`, `peak_double`, `offpeak_only`, and `extra` (merged into the request body).
- **`weight`:** the share of packages that start on that provider (default 1). A package keeps its provider for the prompt cache. Weight 0 means overflow and failover only.
- **Shipped configs:** they use GMI Cloud (weight 3), Morph (1) and Novita (1) at fp8, each pinned through OpenRouter with `extra.provider.order` and `allow_fallbacks: false`. Z.ai direct has weight 0 (AGENTS.md section 7.3). Prices include OpenRouter's 5.5% credit fee.
- **Estimates:** `pbsd_run estimate` prices each tier at the weighted average of the configured providers.

**Model tiers:**
- A role runs on `role_tier[role]`, else `default_tier` (`flash`).
- From attempt `escalate_at` on, it runs on `escalate_tier` (default `flash`, so escalation is off). Set `escalate_tier: "frontier"` to move hard tasks to GLM-5.3.
- Providers fail over only within a tier.

**Local Lean prover:** `lean_prover_url` (default in the configs: `http://127.0.0.1:8081/v1`, from `../llama/lean-prover.sh`), plus `lean_prover_model`, `lean_samples`, `lean_rounds`, `lean_max_tokens` and `lean_check_timeout_s`. When it is set, prover tasks get the `lean_prove` tool (AGENTS.md section 10).

**Ready-made configs:**
- `config.glm-api.json`: 32 agents, PC only.
- `config.glm-api-burst.json`: 224 agents, with cloud gate workers from `workers_file`.

Set each provider's `concurrency` at or below the account's real limit (Z.ai: console rate-limits page). Model ids other than Z.ai's and OpenRouter's are unverified; check each catalogue.

**Config file:** `PBSD_CONFIG`, else `$PBSD_STATE/config.json`, else `$PBSD_CHECKPOINT/config.json`.

## Use

```
export ZAI_API_KEY=... DEEPINFRA_API_KEY=...          # any subset of providers
export PBSD_STATE=/run/pbsd/state PBSD_WT=/run/pbsd/wt PBSD_REPO=/run/pbsd/repo PBSD_CHECKPOINT=/mnt/d/pbsd-ckpt
cp config.glm-api.json $PBSD_CHECKPOINT/config.json
pbsd_run cpath      # critical path and width of the task graph
pbsd_run estimate   # cost, gate CPU, wall clock per agent count: "low"/"mid"/"high", k=v overrides
                    #   e.g. cores=208 hit=0.8 T2.attempts=4 implementer.in=60000
pbsd_run init       # load docs/plan/tasks.json into SQLite (WAL); safe to rerun, keeps existing states
pbsd_run run        # start or resume; live progress bars, ETA, cost, gate slots, providers
pbsd_run status     # print progress once
pbsd_run selftest   # 23 checks, including an end-to-end task with a scripted model
```

The task database lives in `PBSD_STATE` (tmpfs by default). Every `checkpoint_s` it is copied to `checkpoint_dir` on the HDD, and the repository is pushed with `--mirror` to `checkpoint_repo`. After a reboot the database is restored from the HDD copy. `backup_cmd` adds an off-machine copy every `backup_every_s`.

## How a task runs (`src/harness.cpp`)

1. **Worktree.** `wp/<WP>` first merges `merge_target` (`pbsd-cutover`). The task's worktree on `t/<id>` is then cut from `wp/<WP>`, or resumed from `wip/<id>`/`t/<id>` and refreshed from `wp/<WP>`. It uses the sparse pattern `wt_sparse`: everything except `hbsd/`, `kde/`, `linux/` and bulky docs.
2. **Prompt.** Layered for the prompt cache: global rules, role template (both from `AGENTS.md` section 6), the sorted header list, unit source, then the task with its owned paths and its messages.
   - Sources: a `linux:<dir> part k/N` source is the k-th of N equal line ranges over the directory's sorted `.c`/`.h`/`.S` files, under `ref_roots.linux`.
3. **Tools.**

   | Tool | Notes |
   |---|---|
   | `read_file`, `list_dir`, `grep` | Worktree, main checkout or `ref:<tree>/...`; no absolute paths or `..`. |
   | `write_file` (with `append`), `edit_file` | Owned paths only. |
   | `run_gate` | |
   | `post_message`, `read_messages` | |
   | `lean_prove` | Provers only. |
   | `submit` | Reviewers must give `verdict` and `findings`. |

   Any write after a passing gate invalidates it. Reasoning text is not resent.
4. **Gate.**
   - The runner commits only the owned paths, with a `PBSD-Base` trailer, and runs the **pinned** `gates_dir/<role>.sh` on that SHA.
   - **Locally:** in a scrubbed environment (no `*_KEY`/`*_TOKEN`), with the optional `gate_wrap` prefix and `PBSD_PROOF_TIER` from the task.
   - **On a worker:** push to `mirror_url` as `refs/gates/<id>`, then `ssh <host> -- remote_gate <role> <id> <sha> <tier>`.
   - **Infrastructure errors:** exit 3 or ssh 255 retry up to `gate_infra_retries` times, then end the attempt as `infra`.
5. **Merge.** On a pass, `t/<id>` is merged into `wp/<WP>` (`git merge-tree --write-tree`, then a compare-and-swap `update-ref`). `-IF` and integrator tasks also merge `wp/<WP>` into `pbsd-cutover`. A conflict sends the task back with the conflict as a message.
6. **Reviewer "changes".** The named unit tasks are reopened with the findings as a message. The PROOF (when code changes) and the REVIEW wait for them.

## Features

- **Progress:**
  - an overall bar and one per phase;
  - running, queued, blocked, failed, escalated and quarantined counts;
  - throughput over the last 24 hours, with time left and finish time;
  - cost so far (recorded per turn) and the projected total;
  - slot use, RAM and load;
  - one line per provider (tier, model, in flight, ok, 429s, errors).
- **Fault tolerance:**
  - A SQLite WAL task database with a transaction per state change and an append-only event log.
  - Expired leases are recovered on start, after an integrity check.
  - Lease heartbeats.
  - A commit of the owned paths at every turn end, and a `wip/<id>` ref plus saved conversation state every `checkpoint_s`.
  - Resume from the checkpoint, with a fresh context from attempt 3 and a split suggestion from attempt 5.
  - Escalation at attempt 8, and quarantine after 3 crashes or 20 infra results.
  - Infrastructure failures charge no attempt; 20 in a row pause leasing for 5 minutes.
- **API:**
  - per-provider in-flight cap and RPM/TPM token buckets;
  - package affinity for prompt-cache hits;
  - on 429/5xx/connection errors the provider cools down (Retry-After, or exponential backoff with jitter) and the request fails over within its tier;
  - 401/403 rotate keys;
  - 402 removes only that provider.
- **Gates:**
  - counting slots per kind (compile, QEMU, proof), so gates never oversubscribe the cores;
  - cloud workers from `workers_file` (`<ssh-host> <slots> [kvm]`, re-read every 30 s);
  - QEMU gates only on local slots or workers marked `kvm`.
- **Jobs:**
  - NUMA pinning through libnuma (or numactl);
  - each job in its own process group, killed on watchdog timeout;
  - proofs use the task's `proof_budget_minutes`;
  - `oom_score_adj` 500;
  - admission control on RAM, load and I/O pressure.

## Still to provide

- **Gate commands:** the real build, test and QEMU commands for the hooks in `../gates/lib.sh` (`PBSD_BUILD_CMD`, `PBSD_TEST_CMD`, `PBSD_QEMU_IMAGE`). P0-07 sets them. P0-07 also pins the proof tool versions and confirms the checker flags in `../gates/proof_check.sh`.
- **Cloud gate workers:** `workers_file`, `mirror_url` and `remote_gate`; setup in `../../CLOUD_SETUP.md` and `../cloud`.
- **Runtime task splitting:** a decomposer's split proposal is a `handoff` message; nothing creates the new tasks automatically.
