# Lessons and mistakes (read this first)

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

## F. Why proofs are slow (root-cause analysis, 7 Nov 2026, measured)

One `prism prove` of `theorem band_le_left (a b : Nat) : a &&& b ≤ a` with the Goedel 8B on the RTX 3090 took **567 s and failed** (24 model calls, 13 Lean checks, budget exhausted). GPU utilisation was 0-13% the whole time.

| Part | Measured | Share |
|---|---|---|
| Start-up per `prism prove` process (lake/Mathlib check, 4.9 GB model read from D: at 191 MB/s, load, first-call plan compile ~30 s) | about 120 s before the first model call | 21% |
| Lean kernel checks | 3.3 s each (`import Mathlib`), 13 of them = about 43 s | 8% |
| Model calls | about 17 s each, 24 of them = about 400 s | 71% |

Why a model call takes about 17 s (FORGE `bench` and `run` on the same `.fpk`, CUDA): decode 38-74 tok/s and prefill 130-312 tok/s. An RTX 3090 should do several times that. The decode counters show **724 kernel launches and 6 syncs per token and 0 CUDA graphs captured**, so decoding is launch-bound and the GPU idles (hence the 4% utilisation); the Xeon Gold 6242 (2.8 GHz) is slow at issuing launches. The reply may be up to 768 tokens (`max_new`) under a grammar allowing 40 lines of 400 characters, so one call is up to about 12-20 s of decode plus a few seconds of prefill.

Root causes, in order of effect:
1. **Search yield is low.** Of 24 candidates, most were rejected: invented names (`Nat.ands_le_left`, `Nat.amp`), `sorry`, duplicates, wrong tactics. Each wasted candidate costs about 17 s.
2. **Decode is launch-bound** (no CUDA graphs) and **prefill is slow**: FORGE upstream work (CUDA graph capture for decode, a faster prefill GEMM).
3. **A new process per `lean_prove` call** repeats about 120 s of start-up. A task with 8 calls loses about 16 minutes before any search.
4. **Replies are allowed to be much longer than a tactic needs** (768 tokens).
5. Not a cause: the API (model turns are seconds and cost cents), Lean checks, disk speed.

Remedies, none yet done unless noted: cap `max_new` and the grammar's line count (PRISM); keep the prover resident between calls (PRISM/FORGE server mode); CUDA-graph decode in FORGE; run provers in parallel on more GPUs (burst VM with a GPU); give the model the Mathlib names it needs (a lemma library); `lean_prove` limited to two calls per theorem per attempt (done).
