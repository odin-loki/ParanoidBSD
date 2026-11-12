# S4 pilot, round 2: real headers per translation unit, bit-vector path, repair list (12 Nov 2026)

Raises the share of mined core-kind functions that WP/CBMC can prove, without weakening any claim. Measured on sample A (the pilot's 200) and a fresh stratified sample C (200, other seed, disjoint from A and B; `select_sample_C.sh`). All numbers regenerate from `data2/{A,C}/labels.tsv` with `numbers2.sh`, `estimate2.sh`, `summarize2.sh`. No model calls were made (spend: USD 0).

## 1. Result in six lines

| | pilot builder (before) | step 1: real headers, WP | step 1+2: plus bit-vector (CBMC) |
|---|---|---|---|
| sample A (200) | 29 proved (14.5%) | 36 (18.0%) | **38 (19.0%)**: 17 PROVED, 21 PROVED-ASSUMING |
| sample C (200) | 25 (12.5%) (old builder run for this report) | 37 (18.5%) | **47 (23.5%)**: 17 PROVED, 30 PROVED-ASSUMING |
| A+C (400), post-stratified to the 24,823 candidates with a contract | about 4,750-5,000 | 5,137 (20.7%) | **5,607 (22.6%)**, Wilson range 3,500-8,750 |

1. The gain is real but smaller than the report's optimistic reading (30-45%): **19-23% proved**, not 30-45%. The pilot's "built units: 57% proved" does not transfer: the units that now build are the struct-heavy ones.
2. Unbuilt fell from 75% (A: 149 of 200) to 43% (A: 86, C: 80). Of the 400, 166 are still not built (section 3).
3. The bit-vector path added **12 proofs** (A 2, C 10) and, more important, **24 counterexamples**: contracts that are false under C semantics (section 5, list for repair).
4. Every proved unit replays from a clean directory with the WP cache off: **73 of 73 WP and 12 of 12 CBMC replays OK**, vacuity check passes (`replay2.sh`).
5. Extrapolation to the 32,000 core functions: **about 6,700-7,200 proved** (5,600 from the 24,823 candidates with a contract + 1,100-1,600 from the ~7,200 others at 15-22%), range 4,500-10,000. Pilot figure was about 5,000.
6. Cost: no model cost. Wall time measured 8 min per 200 functions at 14 parallel processes (A, shared PC, load 30-50), 12 min for C with the bit-vector and replay stages: about 20-35 h for 32,000 at the same parallelism (not measured at scale; the kbuild `.i` generation is serialised by a lock).

## 2. What changed (step 1: real headers per translation unit)

`mkunit2.sh` replaces the pilot's text-based unit builder (`mkunit.sh`):
- **Linux** (67% of the pool): the file is preprocessed with the exact command line kbuild itself uses (`make <file>.i`, flags read from the `.cmd` file), in a configured tree: `~/kb/x86` = x86_64 `allyesconfig` minus KASAN/KCSAN/KMSAN/UBSAN/KCOV/GCOV/WERROR/ORC (`build_kernel_x86.sh`; config sha in `data2/*/units/*/kconfig.txt`), then `x86d` = x86_64 `defconfig` (functions that exist only in the "option off" arm of an `#ifdef`, e.g. `zswap_swapon`), then arm64 / arm / riscv / powerpc / mips / s390 `allyesconfig` with `LLVM=1` (`build_kernel_cfg.sh`; files under `arch/<a>/` use only their own). Headers: the preprocessed unit of a `.c` file that really includes the header; a probe unit with `module.h kernel.h slab.h device.h` only if none does. libelf for objtool came from the Ubuntu `.deb` unpacked without root (`~/kb/inc`, `~/kb/lib`).
- **HBSD/PBSD** (30%): no configured tree exists. Flags reconstructed (`tu_bsd.sh`): `-D_KERNEL -D__FreeBSD__ -nostdinc -I sys`, `machine/` -> `sys/amd64/include`, `*_if.h` generated with the tree's own `makeobjops.awk`, headers a build would find in `/usr/include` aliased from `include/` and `sys/sys/`, and **empty `opt_*.h`/`offset.inc` stubs, which are an assumption (all kernel options off) and are named in the label** (`stub-headers-EMPTY(...)`).
- **Slicer** (`tuslice.cpp`, C++): cuts the function out of the preprocessed text and keeps only the declarations it transitively needs (types, enumerators, variables, prototypes); every other function becomes a prototype with no body and no spec. Rewrites for the Frama-C parser, each counted in `slice.err`: `__auto_type`, `asm inline`, bit-field widths evaluated by gcc, flexible/zero-length array members as `[1]`, `GNU attributes other than noreturn/packed dropped` (struct alignment and padding may differ from the real kernel).
- **Contract**: only `result` -> `\result`, then macros expanded with the definitions live at the function (`gcc -E -dD` stream; parameters and ACSL words never expanded). The mined text is kept in `contract.acsl`, the expanded one in `contract.exp`; the unit contains exactly one ACSL block (the annotation guard, checked by `replay2.sh`).
- **WP** (`prove_unit2.sh`): as in the pilot (A = contract goals, B = + `-wp-rte`, vacuity `assert \false`), with one change that is stated here and in every label: the contract goals now exclude `@terminates`, because a mined contract only claims partial correctness; termination goals are run separately and a function whose termination goals are not all proved carries `termination-not-proved(p/t)` and is **PROVED-ASSUMING, never PROVED**. 

Per-unit assumptions that downgrade PROVED to PROVED-ASSUMING (listed in `labels.tsv` column 8): `callees-without-spec:<names>` (WP treats an undefined callee as arbitrary), `pointer-or-memory-precondition`, `header-function-in-probe-unit`, `probe-adds-module.h...`, `config:<non-allyesconfig-x86>` (defconfig arm, or other architecture modelled with machdep gcc_x86_64/gcc_x86_32: ABI and endianness are not the target's), `x86_64-config-used-for-non-x86-arch-file`, `stub-headers-EMPTY(...)`, `termination-not-proved(p/t)`, `bv-sat-uncertified`. Global assumptions (not per unit): the code is what the configuration compiles, GCC attributes ignored as above, no `assigns`, WP `-wp-rte` skips alignment and function pointers, solver soundness (Alt-Ergo 2.6.4, Z3 4.8.12, CVC5 1.1.2).

## 3. What is still not built (166 of 400) and why

Counts per tree (A / C): linux 25 / 33, bsd-kernel 23 / 12, bsd-user 34 / 33, linux-tools 4 / 2. Causes, largest first:
- **BSD userland and contrib** (autoconf/generated headers: `config.h`, `k5-int.h`, `openssl/crypto.h`, `includes.h`, `autoconf.h`, `xlocale`): there is no build tree to take them from; inventing them would be unfaithful. Needs a real `make` of those ports.
- **Linux files in no configuration** (`kbuild produced no flags`: m68k/amiga, mips/ejtag, msm, amd swsmu files not selected by allyesconfig for the five arches, parisc/sparc/arc/sh without a configured tree): 13 in A+C.
- **Function not in the preprocessed unit** (defined by a macro, `#if 0`, or in a configuration arm none of ours selects): about 12.
- C++ sources (3 in A), `true`/`false` unbound in the contract when the unit has no stdbool (4), Frama-C parse limits (`Call to __ilog2_u32 in constant`, a syntax error on a kernel construct: 5).
- Slowest remaining class is not a failure of the builder but of the sample: 50 of the unbuilt are BSD user-space files and C++ LLVM code.

## 4. Why built units still fail (A: 31 FAILED + 31 CONTRACT-ONLY; C: 41 + 16)

- **CONTRACT-ONLY (contract proved, run-time-error goals not): 47.** Mostly `mem_access` goals: the function dereferences a pointer or field and the mined contract has no `\valid`. The contract is true but admits undefined behaviour: these go on the repair list (section 5), not into PROVED.
- **FAILED: 72.** Solver timeouts on bit-level goals that CBMC could not take over because the contract has a pointer (98 of the 115 bit-vector skips and failures), loops without invariants (3 unwinding failures at 66), inline asm / builtins (no semantics), `terminates`-independent contract failures.
- **Bit-vector path coverage:** it needs a scalar contract and scalar parameters. Of the 155 units sent to it (all FAILED or CONTRACT-ONLY after WP): 12 BV-PROVED, 24 BV-FALSE (false contract), 1 BV-CONTRACT-ONLY, 98 skipped for a pointer parameter, 9 void, 3 loop not unwound, 2 not parseable, 2 quantifier, 2 field/array access, 1 call, 1 inline asm. Extending it to pointer parameters (symbolic memory harness) is the main remaining lever for step 2.

## 5. Step 2: bit-vector path (`acsl2bv.cpp`, `bv_unit.sh`)

CBMC (5.95.1, SAT, exact machine integers) on a harness `assume(requires); r = f(args); assert(ensures)` generated from the unit. The ACSL-mathematical-integer versus C-machine-integer gap is closed explicitly: the contract is evaluated in `__int128`, every parameter/literal/enumerator/`\result` widened first, a cast `(T)e` converts to `T` first as in ACSL; `+ - *` use overflow-checked helpers, a shift is a multiplication or a floor division by 2^k, and an intermediate outside 128 bits, a shift count outside [0,100) or a zero divisor makes a harness assertion FAIL (it is never assumed away). Chained comparisons are read as conjunctions. Unsupported constructs (pointers, `\valid`, quantifiers, calls, fields) give SKIP. Inline asm and target builtins: SKIP (CBMC has no semantics, so neither proof nor counterexample would be trusted). Statuses: `BV-PROVED` (assertion holds for all inputs, requires satisfiable by a vacuity run, signed overflow / shift / division / pointer / bounds checks of the function clean), `BV-CONTRACT-ONLY`, `BV-FALSE`, `BV-VACUOUS`, `BV-UNWIND`. A BV proof is labelled **PROVED-ASSUMING with `bv-sat-uncertified`**: the SAT result is not proof-certified. Replayed in a clean directory (12 of 12 OK).

## 6. Step 3: contracts to send back for repair (nothing repaired here)

`data2/A/repair_list.tsv` (42 rows) and `data2/C/repair_list.tsv` (29 rows): columns `N name kind class tree label how assumptions cause`.
- **FALSE (24, CBMC counterexample in the cause column, trace in `units/*/bv.cex`)**, for example `iwl_he_is_sgi` and `fun_port_type` (`==` binds tighter than `&&` in the ensures), `check_attr_len` (`unsigned` compared with `-1`), `__ffsl`, `kmalloc_gfp_adjust`, `check_data_rlimit`. **Caution:** where column 8 lists `callees-without-spec`, the counterexample may rely on an arbitrary return value of that callee (for example `ad5446_init`, `numa_init_sysfs`): the contract is false only if the callee can behave that way. Treat those as "needs a callee assumption in `requires`", the others as wrong.
- **CONTRACT-ONLY (47)**: contract holds but admits undefined behaviour (missing `\valid`, signed overflow); the S2 repair prompt should add the precondition.

## 7. Verification hygiene kept

Vacuity check (WP and CBMC); annotation errors fatal (`-kernel-warn-key annot-error=active`); the unit has exactly one ACSL block equal to the expanded mined contract (guard replaces the model-annotation guard since no model was used); scan for assume/admit/lemma/axiom/ghost/behavior/check inside ACSL; WP cache off in replay; no Lean in this round, so no sorry/axiom scan is applicable (the Lean results of the pilot are unchanged and still not sellable). Mutation test was not rerun on the new proofs.

## 8. What is left

1. Build the BSD userland ports (needs `configure`/`make` of contrib) or drop them from the sellable scope: 34+33 of 400 functions.
2. Bit-vector path for pointer contracts (98 skips), loop-bearing functions (invariants or k-induction), and inline-asm helpers (semantics for `bsr`, `bt`...).
3. Repair round for the 71-row list, then rerun: the 47 CONTRACT-ONLY units probably become PROVED-ASSUMING (+12 percentage points).
4. Architectures without a tree (sparc, parisc, arc, sh, m68k, loongarch, xtensa...), drivers selected by no x86/arm/riscv/ppc/mips/s390 config.
5. Mutation test and the S5 statement audit (still the sale-critical gap) on the new proved set; the share of range-only contracts among the 85 was not recomputed.
6. Run time: at 32,000 units budget one day of the PC; the kernel `.i` generation takes a global lock and a per-file kbuild run; cache `~/kb/tu` is not committed (2-5 MB per file).

## 9. Reproduce

```
bash build_kernel_x86.sh; bash build_kernel_cfg.sh x86d x86_64 defconfig; bash build_kernel_cfg.sh arm64 arm64 allyesconfig LLVM=1   # also arm riscv powerpc mips s390
bash select_sample_C.sh; bash build_tools.sh
wsl -- bash run_step1.sh A 14      # then C: build, WP, bit-vector, replay, export to data2/<A|C>  (resumable)
bash numbers2.sh; bash estimate2.sh
```
Environment: Frama-C 33.0 + WP, Why3 1.8.2, Alt-Ergo 2.6.4, Z3 4.8.12, CVC5 1.1.2, CBMC 5.95.1, gcc 13.3, clang for the non-x86 configs; Linux 6.18.60.
