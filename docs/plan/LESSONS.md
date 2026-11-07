# Lessons and mistakes (read this first)

See also [REHEARSAL_REPORT_2026-11-07.md](REHEARSAL_REPORT_2026-11-07.md) for the measured results and data.

For any AI (or person) who runs, repairs or extends the PBSD pipeline. Each item is a mistake that cost real time or money on 6-7 Nov 2026, with the rule that came out of it. **Measured** items were observed on Odin's PC (Windows 11, WSL2 Ubuntu, RTX 3090, 48 GB RAM cap). Add new ones at the top of the relevant section.

## A. Running on this PC (WSL, Windows)

1. **WSL shuts the VM down when no `wsl` session is attached.** A job started with `nohup ... &` inside a `wsl -- bash -lc` that then returns is killed within minutes, and `/run` and `/tmp` are wiped (uptime reads 0 min). Hold a `wsl` process open for the whole run (a foreground or background-held `wsl -- bash ...` from the host, or a `tmux` session plus an open `wsl -- sleep infinity`). Before trusting any "is it still running" check, look at `uptime`.
2. **`/run/pbsd` is tmpfs.** After any WSL restart, re-run `tools/cloud/pc-setup.sh` as root (`wsl -u root`) to recreate `/run/pbsd/repo`, state and worktrees, then `. ~/.pbsd/env` and `tools/cloud/preflight.sh`. Rehearsals cloned from `/run/pbsd/repo` and failed with "repository does not exist".
3. **Windows must not sleep or install updates during a run.** Sleep = Never; pause updates. Odin set sleep to Never on 7 Nov.
4. **`pgrep -f "pbsd_run run"` matches its own shell.** Use `pgrep -x pbsd_run`, and check the state DB.
5. **Shell quoting between Windows and WSL breaks things.** A sed replacement containing `` silently wrote a vertical-tab character into AGENTS.md (the prover prompt lost `alid`): after editing prompts, grep for control characters (`grep -c $''`). MSYS rewrites `/path` arguments; PowerShell eats `$` and backslashes; sed/awk one-liners deleted a needed line (`need lake` in `proof_check.sh`) and mangled TLA+ backslashes. Write a patch file and run it in WSL, or use the Edit tool; restore with `git checkout` if a file was damaged. Python exists only in WSL.
6. **Running as root vs odin.** `apt` and `pc-setup.sh` need `wsl -u root`; run in the background as odin, `sudo` waits for a password and hangs (a stale preflight shell sat blocked for 10 hours). Root also triggers git "dubious ownership" warnings on `/run/pbsd/repo`; harmless.
7. **Do not replace a binary a run is using.** Build to a new directory (`~/pbsd-run-build2`), copy it over only when nothing is running.
8. **WSL RAM is capped at 48 GB**; one NUMA node; one 24 GB GPU. `prover_slots` is 1 on purpose: two provers do not fit with the models loaded.

## B. Money

9. **GLM-5.3-Flash defaults to maximum reasoning.** The same short Lean request spent 8,000 tokens (the whole cap) on reasoning in 97 s and returned nothing; at `reasoning_effort: "low"` it answered in 3.7 s. One turn at the default took 44 minutes. The config sets `low`; `escalate_reasoning_effort` (`high`) applies from attempt 3 only.
10. **Prover tasks are the cost driver** (about 77% of spend in rehearsal A, about $1.9 per prover task before the effort fix). Cap spend with a watchdog (`rehearsal_b.sh [usd]`); the real runner has no hard cap, so check cost after the first hour and set a cap at the provider.
11. **Credit and cards:** Odin's balance was about $5 on 7 Nov and a bank outage blocked top-ups. Do not start an expensive run without confirming credit; run capped tests ($1-2) first to measure cost per task.
12. **Never leave an API key in a synced folder** (it sat in OneDrive). It was deleted; the key is in `~/.pbsd/env` (mode 600) only.

## C. Agent behaviour that the gates had to be hardened against

13. **Placeholder output passes a syntax check.** A decomposer "passed" by writing `/* ${doc} */` into three files. `unit_build.sh` now rejects placeholders, comment-only files, tiny `.md` files and `.lean` files with no declaration, **only for files the task changed**.
14. **Agents read forever and never write.** P0-02-IF kept reading for 40 turns on three attempts. The runner nudges, then withdraws `read_file`, `list_dir` and `grep` until a file is written.
15. **Same failure, 338 gate runs.** One proof task repeated the same failing gate output. Circuit breaker: warn at 3 identical failures, fail the attempt at 6.
16. **Specs that axiomatise the problem, or take parameters.** A statement written `def foo (x : T) : Prop` passed the gate but cannot be proved as `Model.foo : Spec.foo`; the gate now requires parameterless `def name : Prop`. The decomposer wrote `axiom ...` for the package's operations, so nothing could be proved, and `deriving DecidableEq, Repr` on a structure with function fields did not compile. The build gate now compiles `proofs/*/spec/` with the pinned Lake template and requires `def name : Prop`, with no `axiom`, `opaque`, `constant`, `sorry` or `admit`.
17. **`lean_prove` retried on the same hard theorem.** Each call is the 8B then the 32B, 7-10 minutes; the agent spent 90 minutes for 2 of 9 proofs. Now two calls per theorem per attempt, 900 s per call. Prove easy goals by hand with Mathlib tactics.
18. **Mathlib is required.** Goedel-Prover-V2 targets Lean 4.9 + Mathlib v4.9.0. Without Mathlib `ring`, `norm_num`, `linarith` are unknown tactics and the model fails. The shared packages are linked into every project (`.lake/packages`); copying the project without them made every proof time out at 1,202 s.
19. **Vacuous contracts pass.** A scalar `// requires:` comment is not recorded by PRISM, so a vacuous precondition passed. Scalar functions must use a single-line ACSL block `/*@ requires ...; ensures result ...; */` (the return value is `result`, never `\result`). Signed arithmetic is flagged for overflow whatever the `requires` says.
20. **Interim build hooks.** The whole `pbsd/` tree does not build (about 157 modules cannot find headers in `hbsd/`), so the default build gate would fail every task. `PBSD_BUILD_CMD=/opt/pbsd/gates/unit_build.sh` checks only owned files; P0-07 must define the real build.
21. **Pin the gates.** Gates run from `/opt/pbsd/gates` and `/opt/pbsd/tasks.json`, not from the worktree an agent can edit. After editing gates or prompts, re-run `pc-setup.sh` and confirm preflight says "pinned gates match the repository".

17b. **The decomposer froze false statements.** In the sixth rehearsal three of seven locked statements were false as written (no premise tying the claim to the operation), so P0-02-MODEL could never pass: the agent found counterexamples but the gate wanted proofs. The IF prompt now requires a by-hand truth check of every statement, and the prover prompt says: prove `neg_<name>`, post `proof_failure` to `if`, move on. `slim_check` cannot test these (function-typed fields have no sampler).
17c. **Never rebuild or replace a binary while a run uses it.** A PRISM relink during a rehearsal made `lean_prove` return code 127 and spoiled the result. Build elsewhere and swap when idle.

## D. PRISM and FORGE

22. **PRISM tools are fetched, not vendored.** `prism-deps` only fetches; cppcheck, CBMC and ESBMC were built by hand. Clang 18 cannot build PRISM (`std::expected`); use the pinned clang 23.1.4.
23. **PRISM bugs fixed upstream:** per-call seed duplicates and prime-name regex (PRISM#40), source paths relative to the scan root (#41; absolute paths gave `uint32_t` errors in contracts/wp), link `.lake/packages` (#42). Run PRISM from inside the scope directory.
24. **FORGE bugs fixed upstream:** grammar `{m,n}` repetition (#69), a governor destroyed at exit (double free at process end on CUDA) (#70), autodiff shapes stored by reference (use-after-free) (#71). FORGE has no training path, so there is no fine-tuning (decided 7 Nov).
25. **PRISM proves pointer functions only with a stated size.** Use `// requires: \valid(p + (0..N-1))` with a constant N (verdict `PROVED-ASSUMING`); a function still `NEEDS-HARNESS` after two tries falls back to T3 (`BOUNDED(k)`, labelled as such, never `PROVED`).

## E. Process mistakes by the AI that did this work

26. **Verify before declaring done.** A mock regression "passed" until content checks exposed a comment-only file; the first statement of "rehearsal finished" was checked against the state DB, not the output file.
27. **Do not delete directories to "fix" things.** `~/PRISM` was moved aside and the clone timed out; it was restored from the backup. Move, don't delete, and confirm the replacement works first.
28. **Do not trust a background-task "completed" notice.** It can mean only that the launcher exited. Check the real process and the state DB.
29. **The auto-mode classifier blocks `curl | sh` installers and `gh pr merge` from Bash.** Ask Odin to run installers manually; merge PRs from Windows cmd/PowerShell when Odin says so.
30. **Odin's constraints:** C, C++ and CUDA only for anything that ships (no Python in the product pipeline); Goedel 8B by default, 32B only as escalation; no fine-tuning; PRISM and FORGE must be used for proofs and inference.

## F. Why proofs were slow (root-cause analysis, 7 Nov 2026, measured; fixed)

Start: one `prism prove` of `theorem band_le_left (a b : Nat) : a &&& b ≤ a` (Goedel 8B, RTX 3090, 24 model calls, 13 Lean checks, failed) took **567 s** with the GPU at 0-13%.

**First analysis was partly wrong.** I benchmarked an old FORGE dev build (`~/forge-main`: 38-74 tok/s decode, 724 launches per token, no CUDA graphs) and blamed launch overhead. The FORGE that PRISM actually links already uses CUDA graphs: **4,427 tok/s prefill and 127 tok/s decode (87 with a grammar)**. Lesson: benchmark the binary that runs in production, and measure inside the real process before concluding.

**Real cause (timing added inside PRISM's model call, `PRISM_TIMING=1`):** per call, prefill is under 1 s and the GPU forward pass takes 8 ms per token, but `grammar_constraint::apply` (the grammar mask, on the CPU) took **0.3-2 s per token, 95% of decode time**. Decode ran at 3 tok/s. Two defects in FORGE's `grammar.hpp`:
1. `*`, `+`, `?` and `{m,}` compile to right recursion and a finished frame stayed on the stack for every iteration: the stack grew with the match, no state repeated, every step copied a deeper stack (cost grew along the line: 184 s for a 92-token reply).
2. `apply` walked the whole 150k-token trie on one thread for every token although the allowed set depends only on the stack set.

**Fixes (FORGE#72, PRISM#43 draft, runner):**
- FORGE: pop finished frames before a tail call; cache the allowed-token mask per stack set; spread a miss over threads; `grammar_constraint::reset()`.
- PRISM: one grammar constraint per (model, grammar) for the process; `lean_proof.gbnf` uses unbounded `char+` and `( line )*` (the validator already enforces 40 lines of 400 bytes); the sha256 of the 5 GB model (44 s) is cached by path, size and mtime; `PRISM_MAX_NEW` (the runner sets 400); `PRISM_TIMING=1`.
- Runner: `lean_prove(file, "*")` proves every theorem with a sorry in one process (model loaded once); `prover_slots` 2 (two 8B processes share the 3090; measured: two concurrent proves each finished in about 120 s, same as one alone, 15.7 GB of 24 GB); the 32B escalation takes the whole GPU (exclusive slot).

**Result, same search:** 567 s -> 188 s (digest computed) -> **118 s** (digest cached). Decode per call from up to 184 s to about 0.5 s.

**What is left in the 118 s:** about 34 s one-time warm-up in the first prefill (CUDA graph instantiate / kernel setup, per process), about 43 s of Lean checks (3.3 s each, `import Mathlib`), about 8 s `lake build`, about 15 s of decode. The search itself still fails often: the 8B invents Mathlib names (`Nat.ands_le_left`, `Nat.amp`), writes `sorry`, or repeats itself, so most of the 24 candidates are wasted; that is model quality, not speed.

Rules: measure with a timer inside the real process first; keep grammars free of large counted repetitions; do not benchmark a different build from the one in production.

## G. Orphan provers (7 Nov 2026)

When a rehearsal's runner was killed by `timeout`, its `prism prove --all` child (the 32B escalation: 25 GB of RAM, the whole 24 GB GPU) kept running for over an hour. It spoiled the next measurement (a prove took 574 s instead of about 120 s) and would have starved a real run. Fixes: jobs now die with the runner (`PR_SET_PDEATHSIG`), and `rehearsal_b.sh` kills leftover provers first. Before any timing or run, check `nvidia-smi` shows about 0% and no `prism prove` in `ps`.

## H. Speed of one T1 task and dropping Goedel (7 Nov 2026)

Time split of the passing 48-minute P0-02-MODEL run (345 requests, one agent): 36% quick API turns (about 3.3 s each), 50% gate runs and `lean_prove` (38 full gate runs, about 38 s each, mostly Lean builds), 13% one 32,768-token request (384 s). The agent had no cheap compile check and abused `lean_prove` on made-up "probe" theorems. New tool `lean_check(file)` (incremental `lake build Model Spec`): measured 24 s cold, 4.4 s warm, 7.5 s after an edit, with the Mathlib packages linked. Prompt: use it after every edit, run the gate once it compiles with 0 sorry.

**Odin dropped the Goedel prover (7 Nov 2026):** it returned no proof in 13 calls, GLM wrote every proof, and T1 is 12 of 12,976 tasks (about $30 of API). `pc-setup.sh` no longer sets `PRISM_PROVER_FPK`, so the runner does not offer `lean_prove`; preflight reports it as off. No GPU is needed for the run, so the GPU burst VM is not needed either. The code stays for a later revisit (`lean_prove`, two prover slots, the FORGE/PRISM speed fixes all stay merged).
