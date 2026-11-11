# Code-mined shards: verification results

Run on 11 Nov 2026 following [VERIFICATION_PLAN.md](VERIFICATION_PLAN.md). Everything here is **measured** on Odin's PC unless marked otherwise. Tools: `docs/plan/tools/verify/` (shell, awk, jq, C++; no Python). Results: `tags/` (one line per function), `verify-results/` (per-step data and summaries). **Step 4 was stopped by Odin on 11 Nov 2026 at 1,458 records** (US$3.20 spent, upper bound); its numbers below are final for that partial run, and the tags include them. It can be resumed with `tools/verify/start_step4.cmd` after deleting `mined-data/verify/xcheck/STOP`.

## 1. What was checked

| Step | What it does | Functions in scope | Result |
|---|---|---|---|
| retry | the 214 chunks that failed during mining, retried with a bigger token budget | 214 chunks | 203 recovered; 11 are test vectors / data files with no functions |
| 1 mechanical | name in the slice; `result` on a void function; contract names that exist nowhere in the file; error codes in the prose missing from the formal contract; no `requires` / `ensures`; `ensures true` | 1,135,593 | **55,307 fail** (4.9%); 43,516 of the 149,268 with a formal contract (29%) |
| 2a stub grammar | single-line `/*@ ... */`, clauses start with requires/ensures/assigns, balanced brackets, no prose between operands | 149,268 contracts | **23,105 fail** (15%) |
| 2b Frama-C 25.0 | each contract parsed and typed by Frama-C's ACSL front end, in its own stub with the real signature | 145,743 contracts | 78,819 pass, 45,801 fail, 20,260 type errors, 863 unknown |
| 3 random testing | pure integer helpers: contract translated to C (exact 128-bit semantics), 200,000 random and edge-value inputs | 2,312 tested (of 31,367 candidates) | 1,586 pass, **695 counterexamples**, 17 trap under their precondition |
| 4 cross-check | DeepSeek V4.1 Flash 1017 on Morph reads code and annotation, AGREE / DISAGREE per function | 132,513 eligible records (476,980 functions); stopped at 1,458 records, 4,944 functions | 3,399 agree, **1,545 disagree (31%)** |
| tiebreak | 60 random DISAGREEs judged in this session (three reviewers) | 60 | DeepSeek right in 43 (72%) under "is it false", **55 (92%)** under the plan's definition (weaker than the prose is a defect) |

## 2. What the defects are

- **Step 1**, top reasons: contracts naming identifiers that do not exist in the file (26,311: prose words or invented names), the function name not in its slice (8,525), no `requires` (8,041), `ensures true` (5,918), names that are not C or C++ names (4,683: `<unnamed VRAM size probe>`, `sys_mac_syscall (stub)`), prose error codes missing from the formal contract (3,317), `result` on a void function (1,058).
- **Step 2**: 21,000 of the 23,105 stub-grammar failures are prose inside the ACSL block (`requires psock has a peekable msg_rx`). Frama-C rejects 98% of what the stub grammar rejects and about 22,000 more: calls to C functions (ACSL cannot call them: `cpu_is_omap15xx()`), lowercase `null`, `old(x)` without the backslash, English words, and **operator-precedence bugs** (`result == (x >> 18) & 0x3` compares first, then masks a boolean: 1,875 contracts, counted as failures).
- **Step 3**: of 695 counterexamples, **257 are wrong for ordinary inputs** (still failing with inputs 0..2000; examples: `gen_endbr` claims `0x0f1e0ffa`, the code yields `0xfa1e0ff3`; precedence bugs) and **436 only at the integer limits** (the contract ignores wraparound: `a + 1` at `INT_MAX`, a 64-bit shift, a 64-bit product). Both are false in ACSL; the second kind is the milder one and is tagged separately (`fuzz_note`).
- **Step 4 / tiebreak**: DeepSeek's objections were checked against the code in 60 random cases: 43 upheld, 16 overturned, 1 unclear. 12 of the 16 overturns were "the formal contract is weaker than the prose but true": the plan counts that as a defect, so under the plan's definition DeepSeek was right in 55 of 60. Only about 4 were DeepSeek misreading the code. The upheld ones are concrete: missing error returns, leaked resources on failure paths, `requires true` on functions that dereference their arguments.

## 3. Tags (step 6)

`tags/tags-NNN.jsonl.gz`: one line per function with `verified: {mech, syntax, fuzz, xcheck[, tiebreak][, fuzz_note]}` and `status`:

| status | functions | meaning |
|---|---|---|
| verified | 64,325 | no check failed or disagreed, and at least one passed or agreed |
| rejected | 81,651 | at least one check failed or disagreed (an upheld disagreement counts) |
| unchecked | 989,617 | no check could judge it: prose-only annotations outside the 1,458 records step 4 reached |

**Read "verified" carefully.** Of the verified functions, about 60,000 rest only on syntax (the formal contract parses and types in Frama-C); about 1,400 are also random-tested and about 2,000 cross-checked. A contract that parses can still be false: only `fuzz: pass` and `xcheck: agree` say anything about truth, and only `fuzz: pass` is evidence from running the code. Nothing here is a proof.

## 4. Changes from the plan

- **Step 2** uses Frama-C (installed on 11 Nov as `frama-c-base` 25.0) and a stub grammar, as planned. Each contract is parsed in its own file: in one batch file a single crash or bad stub stopped the parse and hid every later verdict.
- **Step 3** does not use libFuzzer or E-ACSL. E-ACSL's runtime crashed under libFuzzer (shadow memory against thread-local storage) and in a hand-built standalone run (malloc/free mismatch with GMP); the Ubuntu package has no `e-acsl-gcc.sh`. Instead `contract2c` translates the scalar subset of ACSL (arithmetic, comparisons, `==>`, `<==>`, `?:`, casts) into C evaluated in `__int128`, and a driver draws 200,000 random and edge values. Unsupported constructs are skipped and labelled. Only 2,312 of 31,367 candidates could be tested: 18,674 have pointer or non-integer parameters, 8,302 contracts use constructs outside the subset, 2,079 functions do not compile on their own.
- **Step 4** covers the plan's subset (state-machine and driver-glue that pass steps 1 and 2), which is 132,513 records rather than the estimated 58k. Measured cost is about **$0.0018 per record** (DeepSeek is a reasoning model: 95% of its output is reasoning; `reasoning_effort` made no difference), so the full subset would cost about $240. It runs with a **$60 cap**, records holding a formal contract first (18,874 of them), which covers about 33,000 records. The eligible list was built before a step 1 fix (C++ method names were wrongly failed), so some C++ functions from the KDE and PBSD trees are missing from it.
- **Tiebreak**: the plan named Claude Sonnet through an API; there is no Claude API key on the PC, and Odin asked for the tiebreaks in this session. A random sample of 60 was judged instead of every disagreement (about 20,000 are expected), which is enough to estimate DeepSeek's precision but not to clear individual records. The reviewers' rule ("weak but true is not a defect") differed from DeepSeek's prompt; the numbers above give both readings.

## 5. Still open

1. Step 4 was stopped at 1,458 of 132,513 eligible records (the 18,874 with a formal contract come first). Resuming it continues where it stopped; re-run `tools/verify/tag_all.sh` afterwards and commit `tags/`.
2. Decide whether "weaker than the prose" is a defect (the plan says yes; the tiebreak reviewers said no). It moves 12 of 60 sampled verdicts.
3. More tiebreaks if individual records matter: the packets builder (`tools/verify/tb_build.sh`) makes any number.
4. The stratified manual audit (not planned) would give a per-kind error rate.
