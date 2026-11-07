# Rehearsal report, 6-7 Nov 2026

What was tested, what was measured, what broke, what was fixed, and what is still open. Read [LESSONS.md](LESSONS.md) for the rules that came out of it. Raw data: [rehearsal-data/2026-11-07/](rehearsal-data/2026-11-07/). **Measured** means observed on Odin's PC (Windows 11, WSL2 Ubuntu, RTX 3090, 48 GB RAM cap); **estimate** means not measured.

## 1. Result

The pipeline now runs one real verified-core package end to end with the real model, gates and checkers:

| Task | Result | Time | Model requests | Cost |
|---|---|---|---|---|
| P0-02-IF (decomposer: contract headers and locked Lean statements) | passed, attempt 1 | 1,182 s | 30 | $0.046 |
| P0-02-MODEL (prover, tier T1: proofs of the locked statements) | **passed, attempt 2** | 2,899 s | 345 | $0.766 |

The final `Model.lean` proves the 7 locked statements plus 3 helper lemmas, with no `sorry`, `admit`, axiom or `native_decide`. The T1 gate (Lake build against the pinned template with Lean 4.9.0 and Mathlib v4.9.0, the `example : Spec.X := Model.X` statement check, the axiom audit) returned 0 on the 38th gate run of the task.

**The Goedel prover proved nothing in that run.** All 13 `lean_prove` calls returned code 1 (not proved) or 2 (error); GLM-5.3-Flash wrote the proofs itself and fixed them from the gate's output. This works for a small model with simple statements. Whether it holds for the harder verified-core packages is the main thing the pilot has to show.

Token use (measured, `cost` table): IF 30 requests, 0.82 M input tokens (75% cache hits), 89 k output; MODEL 345 requests, 26.3 M input tokens (71.5% cache hits), 141 k output. About 76 k input tokens and 410 output tokens per request, 8.4 s per request including gate time, one agent working alone.

## 2. Timeline of what was found, in order

1. **Pipeline and gates (6 Nov).** Built PRISM, ESBMC/CBMC/cppcheck, Lean 4.9 + Mathlib, FORGE patches; ran a scripted-model mock pilot (17 of 17 tasks pass); real Morph API validated; rehearsals A and B with the real model.
2. **Agent behaviour hardened** after real runs: placeholder files passing a syntax check, agents reading forever and never writing, 338 identical gate runs on one proof, specs written with `axiom`, statements with parameters, statements that were false as written, and `lean_prove` retried on the same hard theorem. Each became a gate or a runner rule (LESSONS.md section C).
3. **Cost control.** GLM-5.3-Flash defaults to maximum reasoning: 8,000 tokens and no answer in 97 s on a short request, against 3.7 s at `reasoning_effort: low`. The config now defaults to low and escalates to high from attempt 3.
4. **Why proofs were slow (7 Nov, section 4).** One `prism prove` took 567 s with the GPU idle; the cause was the CPU-side grammar mask, not the GPU. Fixed upstream (FORGE#72, PRISM#43/#44); the same search now takes about 118 s.
5. **Last rehearsals.** The model phase hit a 50-minute test limit repeatedly. Two runs were also spoiled by my own mistakes (a PRISM relink during a run; an orphaned 32B prover holding the GPU for 76 minutes). With a clean machine and a 2-hour limit it passed.

## 3. Decisions made (Odin, 7 Nov)

- No fine-tuning (`FINETUNE_PLAN.md` is shelved).
- Goedel-Prover-V2-8B as the prover, the 32B only as the escalation; two 8B processes share the 3090 (`prover_slots` 2, measured: two concurrent proves each finished in about 120 s, same as one alone, 15.7 GB of 24 GB); the 32B takes the whole GPU.
- T2 policy for PRISM: scalar functions get a single-line ACSL block (`PROVED`); pointer functions get `// requires: \valid(p + (0..N-1))` with a constant N (`PROVED-ASSUMING`); a function still `NEEDS-HARNESS` after two attempts falls back to T3 (`BOUNDED`, never `PROVED`).
- GLM reasoning effort `low`, `high` from attempt 3.

## 4. Root cause of slow proofs (measured)

My first analysis was wrong: I benchmarked an old FORGE dev build and blamed launch overhead. The FORGE that PRISM links already uses CUDA graphs (4,427 tok/s prefill, 127 tok/s decode; 87 with a grammar).

Timing added inside PRISM's model call (`PRISM_TIMING=1`) showed the real cause. Per call, prefill took under 1 s and the GPU forward pass 8 ms per token, but `grammar_constraint::apply` (the grammar mask, on the CPU) took 0.3-2 s per token, 95% of decode time, so decode ran at 3 tok/s with the GPU at 0-13%.

| Run (same search, 24 model calls, 13 Lean checks, failed) | Time |
|---|---|
| Before | 567 s |
| Mask cache + tail calls + threaded miss (FORGE#72), grammar constraint reuse (PRISM#43) | 188 s (checksum computed) |
| Plus the cached model checksum (44 s saved) | 118 s |

Two defects in FORGE's `grammar.hpp`: (1) right-recursive loops (`*`, `+`, `?`, `{m,}`) left a finished frame on the stack per iteration, so the stack grew with the match, no state repeated and every step copied a deeper stack (a 92-token reply took 184 s); (2) `apply` walked the whole 150k-token trie on one thread for every token, though the allowed set depends only on the stack set. What remains in the 118 s: about 34 s one-time warm-up per process, about 43 s of Lean checks (3.3 s each), about 8 s lake; the search itself still fails often because the 8B invents Mathlib names.

## 5. Merged and pushed

| Repository | Change | State |
|---|---|---|
| FORGE | #69 grammar `{m,n}`; #70 governor never destroyed; #71 autodiff shapes by value; **#72 grammar tail calls, per-state mask cache, threaded miss, `reset()`** | merged |
| PRISM | #40 per-call seed + primed names; #41 source paths relative to scan root; #42 link `.lake/packages`; **#43 faster `prove`; #44 env registry fix** | merged; `main` CI green |
| ParanoidBSD (`claude/dreamy-mahavira-7e0e79`) | runner (`reasoning_effort`, escalation, progress nudges, circuit breaker, Lean spec gate, `lean_prove` batch mode and caps, two prover slots, jobs die with the runner), gates, prompts, scripts, LESSONS.md, this report | pushed |

One mistake worth stating: I merged PRISM #43 while its CI was red. The failure was a timing issue (CI ran before FORGE#72 merged), but `main` then failed a test that required every `PRISM_*` variable to be documented; #44 fixed it and `main` is green.

## 6. Open items

1. **Final `Spec.lean` and `Model.lean` of this run were lost.** The test repository sits in `/run`, which WSL wipes when idle, and the conversation file is deleted when a task passes. What survives is in `rehearsal-data/`: the task databases, events, cost rows, run logs, the spec lock (hash and the 7 statement names) and the run output. `rehearsal_b.sh` now bundles the repository (`repo.bundle`) at the end of each phase; the next run will keep the files. Re-running to capture them costs about $0.8 and 50 minutes.
2. **Pilot not started.** 447 tasks; cost per task and the share of prover tasks that escalate are unknown until it runs. Morph credit is low (about $5 on 7 Nov); a card top-up and a provider spend cap are needed first.
3. **GCP burst worker blocked** by billing on project `hbsd-513913` (`billingEnabled: false`). The plan's 6-day wall clock needs it (about 44 days on the PC alone, estimate). GPU quota and zone availability are unchecked, and the runner cannot yet send proof tasks to a remote GPU worker.
4. **T1 proof quality.** The 8B returned no proof in 13 calls; GLM-with-gate did the work. Check this on the harder packages (P1-02, the ring protocol).
5. **Whole-tree build** of `pbsd/` still fails (157 modules need headers in `hbsd/`); interim per-task gates are in use until P0-07 defines the real build.
6. **PRISM scalar-only limit.** Policy decided (section 3) but unmeasured: the pilot will show how many T2 obligations land as `PROVED`, `PROVED-ASSUMING` and `BOUNDED`.
7. **Not run:** PRISM's local `prism_tests` (the target was not built in my configuration; CI ran them and passed).

## 7. Files in `rehearsal-data/2026-11-07/`

- `if/`, `model/`: `tasks.db` (SQLite task database), `events.csv` (event log), `cost.csv` (one row per model request), `run-*.log` (runner display log).
- `model/spec-lock.json`, `spec-lock.json`: hash and statement names of the frozen spec.
- `rehearsal-b-final-run-output.txt`: the script output of the passing run.
