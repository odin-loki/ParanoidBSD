# PBSD Agent Port — Master Plan
### Multi-model Stage F/G automation, consolidated-session architecture

Byline: Odin Loch

---

## 0. Goal

Replace the missing "hand-port" step with an agent loop that:
- Opens each file's context **once** per attempt, not once per stage (real token saving)
- Routes work across three models by cost/risk instead of always using the most expensive one
- Still runs every dynamic verification step for real — compile, sanitizers, differential, formal check — because those are the actual safety net, not something an LLM can self-certify

This does **not** replace or rewrite the deterministic tooling that already exists. It consumes it.

---

## 1. What already exists — reuse, don't rebuild

| Piece | Path | Role |
|---|---|---|
| Inventory + risk/envelope scoring | `tools/clang_cxx23_port.py` | Produces `c_inventory.csv`, risk tier, envelope hint |
| File ordering | `pbsd/queue.json` | Leaf-first processing order |
| C++23 stub scaffolds | `pbsd/ports/<wave>/*.cppm` | `Status::NotImplemented` bodies — this is what gets filled in |
| Deterministic rewrite passes (tier 0–4) | `tools/pbsd_passes/passes/` | Mechanical rewrites where safe |
| **The actual work queue** | `docs/migration/clang_port/refusals.jsonl` | Everything the deterministic passes couldn't safely handle — this is what the agent should iterate over, not `queue.json` alone |
| Static lattice analysis | `tools/pbsd_passes/passes/tier3_maths.py` | Pointer-kind (CCured-style), Steensgaard escape, null lattice — already deterministic, already runs |
| Differential runner | `tools/pbsd_passes/differential.py` | Stage G oracle — reuse as-is |
| IR-equivalence oracle | `tools/pbsd_passes/ir_oracle.py` | Reuse as-is |
| Progress bookkeeping | `docs/migration/batch_progress.json` | Extend, don't replace |
| Master plan stages | [cxx23-port-master-plan.md](cxx23-port-master-plan.md) §7 | Stages A–H — this plan implements F/G, feeds H |

**Open assumption — flag this if wrong:** "math verification" is taken to mean **ESBMC**, which has zero integration in the repo currently (checked — no references anywhere). `tier3_maths.py` is a separate, already-working static pass and keeps running as part of the deterministic pre-pass regardless. If you actually meant tier3_maths when you said math verification, the ESBMC section below (§5.4) is the part to cut.

---

## 2. What's new

One script: **`tools/pbsd_agent_port.py`**, built from four pieces:

1. `model_clients.py` — DeepSeek Flash / Pro (thinking + `reasoning_effort=max`), native API only
2. `session.py` — the consolidated per-file session: one context, a tool-call loop, an escalation state machine
3. `esbmc_check.py` — subprocess wrapper, new integration (pending §1 assumption)
4. `pbsd_agent_port.py` — CLI entrypoint, same flag conventions as `run_todo_passes.py`

---

## 3. Model routing

Two tiers (DeepSeek-only), escalate up only, never down, per file:

| Tier | Model | API | Used when |
|---|---|---|---|
| 1 (default) | **DeepSeek V4 Flash** | `api.deepseek.com`, model `deepseek-v4-flash` | risk tier 2/3 (self-contained, parsers) — the bulk of files |
| 2 (final) | **DeepSeek V4 Pro** | `api.deepseek.com`, model `deepseek-v4-pro` | risk tier 1, hard lineages (sched/UMA/Capsicum/UDA/BIFROST/…), **or** Flash fails/times out after `--max-retries` |

Both models share one client class; only `model` differs. Every call enables thinking with `reasoning_effort=max` (override via `DEEPSEEK_REASONING_EFFORT`).

Escalation reason gets logged to the Port Record (§6) either way — evidence trail for whether Flash is actually saving money.

Bounded retries per tier before escalating: **2** fix-up turns on tool failure, then escalate. Configurable via `--max-retries`.

If Pro also fails after its retry budget: mark `NEEDS-REVIEW`, append `docs/migration/clang_port/agent_port_failures.jsonl`, move to the next file. **Never block**.

---

## 4. Per-file session flow

One session, one file, opened once:

**Step 1 — Load context (once):**
- Original `.c` source
- Existing `.cppm` stub from `convert_c_batch.py`
- This file's entries in `refusals.jsonl` (why the deterministic passes punted)
- Risk tier + envelope hint from `c_inventory.csv` / `ast_census_slim.json`
- Prior partial-port state if resuming a `NEEDS-REVIEW` file at a higher tier

**Step 2 — Draft (single LLM turn, current tier's model):**
- Informal spec notes (Stage B) as a companion comment block
- Full C++23 implementation replacing `Status::NotImplemented` (Stage F)
- Test cases in the existing `tools/pbsd_passes/corpus/` format (Stage D)

**Step 3 — Tool loop, same session, same context window:**
1. Compile (`clang++ -std=c++23 ...`) — errors go back into the *same* session for a fix-up turn, not a fresh call
2. ASan/UBSan run — dynamic ground truth (Stage C). Cache the original-binary baseline once per file so it isn't re-run on every escalation tier
3. Differential run via existing `differential.py` against the corpus (Stage G)
4. ESBMC bounded check on the ported unit (Stage G, formal) — pending §1
5. Any failure → feedback goes into the *same* context, not a reload → bounded retry → escalate on exhaustion

**Step 4 — On all-green:** write the Port Record (§6), set `batch_progress.json` status to `converted`, advance the queue.

**Step 5 — On exhaustion at Pro:** `NEEDS-REVIEW`, log trail + failures.jsonl, continue. Never block.

---

## 5. Config

- `DEEPSEEK_API_KEY` from `secrets/api-keys` (gitignored; stub is `secrets/api-keys.example`). Env vars still override if already set.
- Base URL: `https://api.deepseek.com` for `deepseek-v4-flash` / `deepseek-v4-pro`. Optional `DEEPSEEK_REASONING_EFFORT` (default `max`).
- Note: `deepseek-chat` / `deepseek-reasoner` were retired July 24, 2026 — already past, so make sure nothing in the existing codebase still references the old model IDs.
- Concurrency defaults: `--jobs 48` (Flash) / `--pro-jobs 24` (Pro); thread pool = sum of both so Pro is not starved.
- Timeout per file: default **600s** (max-effort thinking needs headroom).

### 5.1 Caching

DeepSeek caches automatically server-side (prefix-match, no manual cache key) — cache-hit input runs roughly 40–120x cheaper than cache-miss. But the discount only applies to input tokens that hit an exact, unbroken prefix match. That means `session.py` has to be built as an **append-only message list** — each fix-up turn appends to the conversation rather than reconstructing the prompt — or every retry pays full cache-miss price again and the whole point of the consolidated session is lost.

Concretely: system prompt + original file + stub + refusal reasons go in first and never change within a session; tool output and the model's own prior turns append after. Track `prompt_cache_hit_tokens` / `prompt_cache_miss_tokens` from each response's usage block into the cost log from §6 — don't assume the hit rate, measure it per file.

One thing caching does **not** touch: output tokens aren't cached on either API, and this workload is output-heavy (drafting a full port + tests each turn). Caching mainly helps the retry loop's repeated input, not the dominant cost driver — don't expect it to cut the bill by the full 90%+ headline figure.

---

## 6. Bookkeeping — Port Record (Stage H)

Extend `schema.py` / `batch_progress.json` entries with:
- `model_used` (which tier actually closed it out)
- `escalation_trail` (list of tier, reason, retry count)
- `stage_evidence` — B/C/D/E/F/G pass/fail per your Appendix A fields
- `tokens_in`, `tokens_out`, `est_cost_usd` per attempt — write these to a separate `docs/migration/clang_port/agent_port_cost.jsonl` too, so you can verify after a real run whether the tiering is actually cheaper than just using Pro for everything, rather than assuming it

---

## 7. Build order for tonight

1. Pull `DEEPSEEK_API_KEY` into env / `secrets/api-keys`; gitignore the real key file
2. `model_clients.py` — Flash/Pro wrappers, thinking + `reasoning_effort=max` (§5.1 note: build the message list append-only from the start)
3. `session.py` — context accumulator + tool-call loop + escalation state machine (this is the core of the token saving — get this right before anything else)
4. Wire in existing modules: `refusals.jsonl` reader, `differential.py`, `ir_oracle.py` — call them, don't reimplement them
5. `esbmc_check.py` — new, subprocess + timeout + SAT/UNSAT/counterexample parse (confirm §1 first)
6. `pbsd_agent_port.py` CLI — `--scope`, `--limit`, `--file`, `--max-retries`, flags matching `run_todo_passes.py` for consistency
7. Extend `schema.py` for the Port Record fields in §6
8. Smoke test on a small tier-3 batch first — `hbsd/src/bin/echo` is already the default prefix in `convert_c_batch.py`, so start there before pointing it at anything risk-tier-1

---

## 8. What this plan deliberately does not do

It does not collapse Stage C/D/G into the LLM's own say-so. "All at once" here means *one session, many tool calls* — not *one generation replaces the tools*. If tonight's build pressure tempts a shortcut where the model just asserts a file is correct without the compile/sanitizer/differential/ESBMC loop actually running, that's the one thing worth not cutting, especially on tier-1 files and especially while Flash is doing most of the volume.
