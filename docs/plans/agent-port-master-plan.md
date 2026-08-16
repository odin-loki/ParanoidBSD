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

1. `model_clients.py` — thin wrappers for DeepSeek's native API and Moonshot's native API (Kimi K3) — both direct, no OpenRouter
2. `session.py` — the consolidated per-file session: one context, a tool-call loop, an escalation state machine
3. `esbmc_check.py` — subprocess wrapper, new integration (pending §1 assumption)
4. `pbsd_agent_port.py` — CLI entrypoint, same flag conventions as `run_todo_passes.py`

---

## 3. Model routing

Three tiers, escalate up only, never down, per file:

| Tier | Model | API | Used when |
|---|---|---|---|
| 1 (default) | **DeepSeek V4 Flash** | `api.deepseek.com`, model `deepseek-v4-flash` | risk tier 2/3 (self-contained, parsers) — the bulk of files |
| 2 (escalate) | **DeepSeek V4 Pro** | `api.deepseek.com`, model `deepseek-v4-pro` | risk tier 1 (concurrency/locks/HW/syscalls/MM, per your own tier definition) **OR** Flash fails/times out twice at tier 1 |
| 3 (final) | **Kimi K3** | `api.moonshot.ai/v1`, model `kimi-k3` | Pro also fails **OR** file is in the plan's "never clang-tidy alone" list: scheduler/allocator semantics, Capsicum lineage, UDA descriptor engines, BIFROST, intentional-UB-reliant code |

Both DeepSeek models and Kimi K3 are OpenAI-Chat-Completions-compatible, so `model_clients.py` can share one thin client class and just swap `base_url` + `model` + API key per tier — no separate SDKs needed.

**Worth knowing before you commit tier 3 to it:** Moonshot's API is China-hosted. For a security-focused OS you're planning to pitch to Western-aligned defence/government buyers, that's a real due-diligence question worth resolving before kernel-port source routinely transits it — not a reason to block tonight's build, but flag it as an open item rather than something to discover later during a customer security review.

Escalation reason gets logged to the Port Record (§6) either way — this is your evidence trail if Flash's hallucination rate turns out to be a problem in practice, and it's what tells you empirically whether the tiering is actually saving money.

Bounded retries per tier before escalating: **2** fix-up turns on tool failure, then escalate. Configurable via `--max-retries`.

If Kimi K3 also fails after its retry budget: mark `NEEDS-REVIEW`, log the full escalation trail, move to the next file. **Never block** — matches your existing rule.

**One asymmetry to design around:** Kimi K3 can't turn thinking off — `reasoning_effort` goes low/high/max but reasoning tokens are always on and always billed as output at $15/M. That makes tier-3 output cost fairly fixed regardless of how tightly you prompt it; the lever you actually have at that tier is *how rarely you reach it*, not how cheaply it runs once you do.

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

**Step 5 — On exhaustion at tier 3:** `NEEDS-REVIEW`, log trail, continue. Never block.

---

## 5. Config

- `DEEPSEEK_API_KEY`, `MOONSHOT_API_KEY`, `CURSOR_API_KEY` from `secrets/api-keys` (gitignored; stub is `secrets/api-keys.example`). Env vars still override if already set.
- Base URLs, both native, no OpenRouter: `https://api.deepseek.com` for `deepseek-v4-flash` / `deepseek-v4-pro`; `https://api.moonshot.ai/v1` for `kimi-k3`. Same OpenAI-Chat-Completions shape for all three.
- Note: `deepseek-chat` / `deepseek-reasoner` were retired July 24, 2026 — already past, so make sure nothing in the existing codebase still references the old model IDs.
- Moonshot account needs a minimum top-up before the key is live — check that before you're mid-build tonight, not after the first call 401s.
- Concurrency: keep Pro's worker pool smaller than Flash's — Flash supports meaningfully higher concurrency per DeepSeek's published limits, so a shared pool size will bottleneck Flash for no reason.
- Timeout per file: reuse the existing `--file-timeout` convention from `run_todo_passes.py` (default 90s felt tight for a full Stage F+G loop — consider 240–300s default for this script specifically).

### 5.1 Caching

Both DeepSeek and Moonshot cache automatically server-side (prefix-match, no manual cache key) — cache-hit input runs roughly 40–120x cheaper than cache-miss on DeepSeek, ~10x on Kimi K3. But the discount only applies to input tokens that hit an exact, unbroken prefix match. That means `session.py` has to be built as an **append-only message list** — each fix-up turn appends to the conversation rather than reconstructing the prompt — or every retry pays full cache-miss price again and the whole point of the consolidated session is lost.

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

1. Pull `DEEPSEEK_API_KEY` / `MOONSHOT_API_KEY` into env; gitignore the stray key file
2. `model_clients.py` — Flash/Pro/Kimi wrappers, one shared client class, `base_url` + `model` + key are the only differences (§5.1 note: build the message list append-only from the start, retrofitting it later is more work)
3. `session.py` — context accumulator + tool-call loop + escalation state machine (this is the core of the token saving — get this right before anything else)
4. Wire in existing modules: `refusals.jsonl` reader, `differential.py`, `ir_oracle.py` — call them, don't reimplement them
5. `esbmc_check.py` — new, subprocess + timeout + SAT/UNSAT/counterexample parse (confirm §1 first)
6. `pbsd_agent_port.py` CLI — `--scope`, `--limit`, `--file`, `--max-retries`, flags matching `run_todo_passes.py` for consistency
7. Extend `schema.py` for the Port Record fields in §6
8. Smoke test on a small tier-3 batch first — `hbsd/src/bin/echo` is already the default prefix in `convert_c_batch.py`, so start there before pointing it at anything risk-tier-1

---

## 8. What this plan deliberately does not do

It does not collapse Stage C/D/G into the LLM's own say-so. "All at once" here means *one session, many tool calls* — not *one generation replaces the tools*. If tonight's build pressure tempts a shortcut where the model just asserts a file is correct without the compile/sanitizer/differential/ESBMC loop actually running, that's the one thing worth not cutting, especially on tier-1 files and especially while Flash is doing most of the volume.
