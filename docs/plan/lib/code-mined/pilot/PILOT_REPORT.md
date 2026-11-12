# S4 pilot report: proving the mined contracts (12 Nov 2026)

Every number below is regenerated from the committed data by `summarize.sh` (`data/summary.md`, sample A; `data-B/summary.md`, sample B). Raw material: `data/units/uNNN/` (contract, extracted function, the exact unit that was proved, solver logs, model replies, Lean files). Scripts and how to rerun: section 9.

## 1. Answer in five lines

1. **Route chosen: Frama-C WP** (Frama-C 33.0, Why3 1.8.2, Alt-Ergo 2.6.4, Z3 4.8.12, CVC5 1.1.2), run on the mined ACSL contract itself. It is the only route of the three that proves the *stated* contract with real ACSL semantics, costs no model tokens, and took about 10 s per function.
2. **Sample A (200 functions, 11 core kinds, stratified): 29 machine-checked proofs (14.5%): 15 PROVED, 14 PROVED-ASSUMING.** All 29 replayed from a clean directory with the WP cache off (29 of 29 OK). 171 are not proved; 149 of those did not even build, because the mined code needs kernel headers that the unit builder cannot supply.
3. **The model did not prove anything.** GLM-5.3-Flash writing loop invariants/asserts: 0 of 24 functions (72 attempts). GLM-written Lean models: 6 of 14 theorems passed Lean, but on inspection none of the six may be sold as PROVED (the model is not the C function, or the theorem is weaker than the contract). Total Morph spend for the whole pilot: **USD 0.55 (conservative price; USD 0.34 at Morph's listed price) of the USD 40 cap**.
4. **Extrapolation to the 32,000 core functions with today's builder: about 5,000 proved (plausible range 3,000-6,500), model cost under USD 50 even if every failure is sent to the model, about 14 h of CPU at 8 parallel processes.** The price of the proofs is not the model. The price is the build environment (section 6) and the S5 audit.
5. **Do not start the main run yet.** Section 7 lists what is missing; the first item (real headers per translation unit) decides whether the proved share is about 15-20% or about 30-45%.

## 2. What was run

| Item | Value |
|---|---|
| Candidate pool | 24,825 functions tagged `verified` by step 4, kind in {arith-helper, bit-op, bitmask, bitmap, allocator, refcount, ring-buffer, list-op, lock-or-sync, parser, error-path, init-teardown}, with a formal contract (`make_cands.sh`) |
| Sample A | 200, quotas by kind (arith 50, init-teardown 20, error-path 20, parser 20, lock 15, allocator 15, bitmap 15, ring-buffer 15, refcount 12, list-op 10, bitmask 8), 75% "scalar" contracts (no `\valid`, `->`, `*`, `[`, quantifier), seeded shuffle `pbsd-s4-pilot-1` (`select_sample.sh`) |
| Sample B | 60 functions that contain a real loop (comments stripped), same pool, kinds prone to loops, seed `pbsd-s4-pilot-B` (`make_sample_B.sh`). Added because sample A has almost no loops and loops are where a model could help |
| Unit builder (`mkunit.sh`) | prelude (kernel typedefs) + all `#define`s of the file (a macro defined twice in the file is dropped, not guessed) + the file's top-level types + the mined contract (`result` -> `\result`, nothing else changed) + the function text, extracted mechanically from the source (no model) |
| Name resolution | an identifier the compiler cannot find is looked up in an index of all single-line `#define`s of the Linux and HBSD headers (3.28 M names, 2.69 M with exactly one definition, `build_macro_index.sh`); only names with ONE definition are used; each resolved name is listed in the result and makes the proof PROVED-ASSUMING |
| WP run (`prove_unit.sh`) | A: contract goals only. B: with `-wp-rte` (no signed overflow, bad shift, division by zero, out-of-bounds, invalid pointer). Annotation errors are fatal, so a contract Frama-C ignored can never give a proof. Vacuity check: `assert \false` at entry must NOT be provable. PROVED needs A and B fully proved and not vacuous |
| Model loop (`annotate_unit.sh`, `guard_annot.sh`) | GLM may add only `loop invariant/assigns/variant` and `assert`; the guard rejects any change to the C code and any other annotation (lemma, axiom, assume, behavior, requires, ensures, ghost); WP then re-checks from scratch. A wrong annotation costs money, never soundness. 3 attempts, reasoning low, low, high |
| Second engine (`prism_check.sh`) | PRISM contracts + wp + pir stages on the same unit, as a cross-check only |
| Replay (`replay_proved.sh`) | fresh directory, only `prelude.h` and `unit.final.c`, WP cache off, 60 s per goal, scan for assume/admit/lemma/axiom/ghost/behavior, contract text identical to the mined one |
| Mutation test (`mutate_proved.sh`) | flip one operator in the C code of each PROVED unit; WP must fail on the unchanged contract |
| Lean route (`lean_unit.sh`) | GLM writes a Lean 4.34 model over BitVec plus `theorem contract`; gate: compiles, no sorry/admit/axiom/native_decide/opaque/unsafe, `#print axioms` limited to propext, Classical.choice, Quot.sound, Lean.ofReduceBool |
| Toolchain and inputs pinned | `data/toolchain.txt`: versions, sha256 of `sample.tsv`, `prelude.h`, the macro index; per unit `meta.tsv` has sha256 of contract, function text and prelude |

## 3. Results, sample A (200 functions)

| Final label | Count |
|---|---|
| PROVED | 15 |
| PROVED-ASSUMING | 14 (reasons overlap: 11 have a pointer in the contract or signature, 3 use macros resolved from other headers, 5 call a function that is not declared in the unit, see 5.2) |
| BOUNDED / TESTED / REVIEWED | not attempted in this pilot |
| not proved | 171 |

| WP status | Count | Meaning |
|---|---|---|
| PROVED | 29 | all contract goals and all run-time-error goals proved, not vacuous |
| CONTRACT-ONLY | 2 | contract proved, but the contract allows an input that causes signed overflow or a bad memory access (`skl_get_dimm_width`, `wilc_wlan_cfg_set_byte`): the **contract is too weak to exclude undefined behaviour** |
| FAILED | 19 | WP could not prove the contract (causes below) |
| UNBUILT | 149 | Frama-C could not read the unit (causes below) |
| WPERROR | 1 | `nm_prev`: the function exists in a form WP cannot select |

**By kind (proved = PROVED + PROVED-ASSUMING):** arith-helper 12/50 (24%), error-path 6/20 (30%), lock-or-sync 4/15 (27%), bitmask 2/8, init-teardown 3/20 (15%), bitmap 1/15, parser 1/20 (5%), allocator 0/15, refcount 0/12, ring-buffer 0/15, list-op 0/10. Kinds that manipulate kernel structs (allocator, refcount, ring-buffer, list-op) are 0 because they did not build (all but one ring-buffer unit, which hit a WP error; section 4).
**By class:** scalar contracts 26/149, pointer/field contracts 3/51.
**Built units only (51):** 29 proved (57%), 2 contract-only, 19 failed, 1 error.

**Strength of the 29 proved contracts:** 17 equate `\result` to an expression of the inputs ("functional"), 12 only compare it with constants ("range", for example `0 <= \result <= 7`). Range contracts are true but weak (the S5 risk from SELLABLE_PLAN section 7).

**Mutation test:** 26 mutants over 20 of the proved units (the other 9 have none of the operators): 25 killed, 1 survived. The survivor is `val_seg`, whose contract is the range `0 <= \result <= 7`: the weak contract cannot see the change. This is the evidence that the proofs are about the code, and that range contracts are the weak spot.

**Second engine, PRISM:** on the 29 proved units its `wp` stage said PROVED-ASSUMING for 4, NEEDS-HARNESS for 20, NOTRUN for 4, no finding for 1. On the 22 built-but-not-proved units: NEEDS-HARNESS 12, ERROR 3, NOTRUN 4, none 3. No disagreement (PRISM never refuted a WP proof, never proved what WP failed), but it adds almost nothing: it refuses pointer functions, never gives an unconditional PROVED when a `requires` exists, and reads the contract with machine integers. Not used as a proof.

## 4. Why 75% did not build (the main finding)

| Cause (sample A, 149 unbuilt) | Count |
|---|---|
| unknown types/macros/attributes after which the C parser stops ("syntax error": `device_t`, `list_for_each_entry`, `__kprobes`, ...) | 47 |
| enum constant, variable, function or multi-line macro from a header (`current`, `V4L2_...`) | 32 |
| struct type from a header (`Cannot find field`) | 32 |
| macro with several different definitions in the headers (arch/config variants: `GENMASK`, `UL`) | 21 |
| function not found by the extractor (it is a macro or generated by a macro) | 11 |
| C++ source | 5 |
| other | 1 |

Sample B (60 loop-bearing functions): **55 unbuilt (92%)**, 5 built and failed, 0 proved. Loop functions are the struct-heavy ones.

Only 133 of the 149 are environment-blocked (the others are not C functions). If they proved at the rate of the built ones (57%) the sample would reach about 105/200; that is an upper bound, because the built ones are the easy self-contained helpers. A realistic reading is 30-45%.

## 5. Why the 19 built functions failed, and what the model did

### 5.1 WP failures (19 in sample A; 5 in sample B, all loops)

- Bit-level contracts: Alt-Ergo, Z3 and CVC5 reached through WP's integer model cannot prove identities such as `(msr & (1UL<<n)) == 0 <=> ((msr>>n)&1) == 0`. At least 11 of the 19 have bit operators in the contract or the code (`vmx_ctl_allows_zero_setting`, `iwl_he_is_sgi`, `host1x_uclass_indoff_indroffset_f`, `get_mmcr1_rsq`, ...); 4 of the 6 Lean theorems of section 5.3 are on such units. Counter-measure: a bit-vector prover path.
- Inline asm and compiler builtins (`crypto_int64_equal_01` has `__asm__`, `_cvtmask32_u32` a `__builtin_ia32`, `fls` a `__builtin_constant_p`): WP cannot give them semantics. Correct to fail.
- **Contracts that are false under C semantics** (found by WP, not by the earlier reviews): `check_attr_len` compares an `unsigned int` with a macro defined as `-1` (ACSL compares mathematically, C converts to UINT_MAX); `get_mmcr1_rsq` returns a 46-bit mask through an `int` (truncated in C). These go back to the S2 repair loop; they are a data-quality product of the proving stage.
- Loops (2 in A, 5 in B): need loop invariants.

### 5.2 Assumptions that remain in a "proved" function (why 14 are PROVED-ASSUMING)

Calls to functions that are undeclared or have no specification are treated by WP as arbitrary (the proof then holds for every behaviour, but assumes the callee terminates and has no undefined behaviour); macros resolved from other headers by unique textual definition; pointer preconditions; the code is preprocessed for x86_64 GCC (`#if` arms chosen by the compiler), the first definition of the function in the file is the one proved. WP's `-wp-rte` skips pointer alignment and function-pointer checks (Frama-C says so). No `assigns` clause is stated in the mined contracts, so nothing is proved about what a pointer function leaves unchanged.

### 5.3 GLM-5.3-Flash as the prover (cost and result)

**Loop invariants / asserts (annotate loop), all 24 functions that were sent, both samples, all failed:**
- 72 attempts: 9 rejected by the guard (the model changed the code), 11 returned an empty answer (token cap), 7 did not parse, 5 WP errors, 40 checked by WP and not proved (37 FAILED, 3 CONTRACT-ONLY/not all goals).
- USD per function (conservative price): **mean 0.0060, median 0.0037, p90 0.0166**; at Morph's listed price mean 0.0038. Tokens per function: input mean 1,927 (median 1,749, p90 2,069); output mean 13,679 (median 8,325, p90 39,098; almost all of it reasoning, at the 14,000/20,000 token caps). Wall time mean 195 s, median 118 s, p90 473 s.
- By kind (functions sent; mean USD): arith-helper 0.0072 (n=14), parser 0.0083 (n=2), bitmask 0.0046 (n=3), bitmap 0.0035 (n=3), error-path 0.0014 (n=1), init-teardown 0.0014 (n=1). Over all sampled functions of a kind the mean is below USD 0.0025 because most never reach the model.
- Cost per model-proved function: unbounded (none proved). Recommendation: **do not budget model money for invariants with this model and these prompts**; the failures are not missing invariants (bit-level reasoning, asm, false contracts).

**Lean 4 models (route c), 14 built-but-unproved scalar loop-free units:** 8 failed all three attempts, 6 passed Lean (`LEAN-MODEL-PROVED`). Mean USD 0.0131 per unit (median 0.0181, p90 0.0205), mean 30,488 output tokens, mean 396 s. Reading the six:

| Unit | What the Lean file proves | Verdict |
|---|---|---|
| `host1x_uclass_indoff_indroffset_f` | the C expression equals itself | faithful, a tautology |
| `get_mmcr1_rsq` | models the return as 64 bits; C returns `int` | **unfaithful**: the contract is false for C, the "proof" hides it |
| `iwl_he_is_sgi` | constants typed in by the model | not checked against the macros |
| `ad5446_init` | adds hypotheses about what the two callees return | **weaker than the contract** (`requires true`) |
| `_cvtmask32_u32` | assumes the builtin is the identity | invented semantics |
| `__is_raid10_far` | invents the bodies of two helper functions | not checked |

So the Lean route gives machine-checked theorems about the model's *own* text. Without a mechanical model-to-C check (differential test of the Lean model against the compiled function) it can never reach PROVED; it is REVIEWED at best and it produced two harmful results out of six. Price if run anyway: about USD 0.031 per `LEAN-MODEL-PROVED` (conservative), plus the audit.

## 6. Cost, time and the 32,000-function extrapolation

**Measured (sample A):** WP phases only (build, contract goals, run-time-error goals, vacuity): **mean 10.0 s, median 9.8 s, p90 20.3 s per function** (built units: mean 14.0 s; units that fail to build: median 1.1 s), at 8 concurrent Frama-C processes on a PC with load average 10-17 from other runs. Phase 1 for 200 functions took 308 s (0.65 functions/s). Model: USD 0 for 90% of the functions (not sent). Total pilot spend USD 0.55 conservative, 236 ledger calls (the number includes a first run of the annotate loop that was repeated with larger token caps, the Lean experiment, sample B and a smoke test).

**Wall time per function by kind (WP only):** arith-helper median 13 s, bitmap 15 s, error-path 10 s; allocator, list-op, lock, parser, refcount, ring-buffer and init-teardown have a median of 1-2 s because most of them stop at the build step (full table in `data/summary.md`).

| 32,000 core-kind functions | Estimate | Basis |
|---|---|---|
| Proved with today's builder | **about 5,000** (range 3,000-6,500). Post-stratified over the 24,825 candidates with a contract: 4,748 (19%); the other-kind pool of about 7,200 is unmeasured and taken at 15% | per-kind rates of sample A (small n: Wilson 95% interval of the sample rate 10-20%) |
| Proved after a real build environment | 40-50% of 32,000 would be the ceiling (13,000-16,000); a planning figure of 30% (about 10,000) is safer | upper bound from the built-unit rate (57%), pointer/struct functions prove less easily |
| WP compute | about 89 process-hours of Frama-C (10 s x 32,000): **about 14 h at 8 parallel** (measured throughput), about 5 h at 24 (not measured at 24) | 10 s mean, 0.65 functions/s |
| Model, if every failed function got the annotate loop | about 3,000 functions x USD 0.0060 = **USD 18** (conservative) or USD 11 (listed); about 12 h at 14 parallel; expected yield 0 | section 5.3 |
| Model, if every failed scalar loop-free function got the Lean attempt | about 2,200 x 0.0131 = USD 29; yield unusable without a fidelity gate | section 5.3 |
| Model, recommended | **USD 0** for the WP pass; keep USD 40 for repair of false contracts (S2) | |

Cost per proved function with the recommended route: no model cost, about 10 CPU-seconds.

## 7. What is missing before the main run

1. **Real headers per translation unit** (blocker for 66% of the sample). Preprocess each source file with the include paths and `-D` flags of its own build (`compile_commands.json` for Linux with a configured tree, `bmake`-style flags for HBSD), then cut the function out of the preprocessed text. Filter C++ (`.cc/.cpp`, templates) before sampling. Without this the proved share stays near 15-20% and concentrates in the arith-helper, error-path and lock kinds.
2. **A bit-vector path for contracts.** WP's integer model cannot prove bitwise contracts (at least 11 of the 19 failures). Options: a bit-precise engine run on a harness (`__CPROVER_assume(requires); assert(ensures)` with CBMC/ESBMC, giving BOUNDED or PROVED within the type range), or Lean/`bv_decide` with a mechanical model-to-C check. Until then those functions end BOUNDED, not PROVED. The semantic gap between ACSL mathematical integers and machine integers must be handled explicitly (the two false contracts of 5.1).
3. **Decide the labels for assumptions** and write them into the per-function ledger: callees without a specification, macro resolution by unique textual definition, `#if` arm and machdep, first definition of the function, no `assigns`. Today they are recorded per unit (`resolved.names`, WP warnings) but not as a named list in the label.
4. **S5 statement audit is still the sale-critical gap.** The pilot proves the contract text as mined; it does not show that the contract says what the prose says. 12 of 29 proved contracts are range-only, and 2 of the 51 built units are CONTRACT-ONLY (the contract admits undefined behaviour). A second-model comparison with a human-readable diff is needed on a stratified sample of at least 400 PROVED records, with the mutation test extended (only 26 mutants fit 20 units here).
5. **Loops have no working method.** 0 of 7 built loop functions were rescued by 21 model attempts. Plan for a different method (k-induction/CBMC for BOUNDED, or a stronger model with the WP error text and counter-examples) before spending on loops. Parsers, bitmaps and ring-buffers are loop-heavy.
6. **Feed the proof failures back to S2:** false contracts found by WP (unsigned vs `-1`, `int` truncation) and contracts that allow undefined behaviour (CONTRACT-ONLY) are defects of the mined annotations.
7. **Run infrastructure for 32,000 units** (not needed for correctness): the scripts are resumable and write to persistent paths; add a chunked runner (as in step 4) and, if wanted, parallelism beyond 8 (not measured).
8. **Check the Morph price** in the dashboard (the two published lists differ by up to 3x; the pilot uses the higher one for the cap) and the GLM output-token behaviour: at `reasoning_effort: low` it still spent 8,000-14,000 tokens before answering on bit-vector and invariant tasks.

## 8. Things that did not work (honest list)

- The first extraction of function text also copied the tail of a multi-line macro that preceded the function; fixed (`extract_fn.awk`).
- A macro defined in several `#if` arms was silently resolved to the last one; fixed (dropped).
- The loop test matched the words "for"/"while" in comments (sample B was built with that flaw once: 60 functions, thrown away and rebuilt with `has_loop.awk`; the flawed run is in `mined-data/pilot-B.flawed-loopfilter`, spend USD 0.04).
- The first annotate run used an 8,000-token cap, which the model exhausted in reasoning (empty replies); repeated at 14,000/20,000 for all units. Both runs are in the ledger.
- PRISM as a proof engine: NEEDS-HARNESS for most units, one reading of the contract (machine integers) differs from ACSL, and it never reports an unconditional PROVED. Kept as a cross-check.
- Edits to the shell scripts through the Bash tool halved backslashes; see LESSONS section G.

## 9. Reproduce

```
sudo apt-get install -y opam m4 libgmp-dev pkg-config graphviz bubblewrap autoconf z3 cvc5   # once, root
bash install_framac_wp.sh              # Frama-C 33.0 + WP + Alt-Ergo in ~/.opam-pbsd (about 25 min); source env.sh before use
bash make_cands.sh; bash select_sample.sh; bash build_macro_index.sh
wsl -- bash pilot_watch.sh 8 14        # unit build, WP, annotate, PRISM (resumable; STOP file stops it; USD cap in morph_call.sh)
bash replay_proved.sh; bash mutate_proved.sh; bash run_lean.sh 6; bash export_data.sh
```
Spend cap and credit-error handling: `morph_call.sh` (cap 40 USD from `morph_ledger.tsv`, credit/auth error text matched only on the API error object, STOP file). No key is stored anywhere in the repository or the data.
