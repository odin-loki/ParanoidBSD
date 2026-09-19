# What to do next

Ranked by whether it changes a coverage number someone might quote, or
whether it turns a queue item into a confirmed bug.

## Confirm or kill the CBMC queue

1. **Done (19 Sep).** 150 `sys/`+`lib/` exported-arithmetic failures read; records in [queue/arithmetic-triage.json](queue/arithmetic-triage.json). Unread in that bucket is 0. `contrib/` and other prefixes remain.
2. **Done (19 Sep).** 35 unread index-OOB + 62 address-taken statics read; records in [queue/oob-static-triage.json](queue/oob-static-triage.json). Four defects fixed (`days_pmonth`/`parse8601`, ppp `protoname` ×2, pkru `3<<30`). Contrib remainder is deferred.
3. **Run a frontier-model pass only on READ-THESE**, never on the 1,481 pointer/memory records. `taxonomy.py --needs-model` is the class list; this JSON is the finding list. A model's answer is a hypothesis until it is checked against the source.
4. **Done (19 Sep).** `vnic_dev.c` `mrh` was a real unchecked `M_NOWAIT` (fixed). The two `in*_fib_algo.c` hits are `p == NULL || p->field` short-circuit false positives.

Skip, until the first-party queue is empty: `contrib/less`, flex, compiler-rt builtins, dtrace test programs, byacc `vdiv` fixtures.

## Make the instruments that ran actually answer

5. **ESBMC.** Either feed it preprocessed FreeBSD TUs (the Linux binary cannot parse this tree as-is) or stop marking the stage `ok` when every cell is `ERROR`. 10,868 ERROR rows currently look like a column in the matrix and are UNTOUCHED for proof purposes.
6. **FuSeBMC.** 182 `CLEAN` out of 292,752 is not a fuzz-guided proof run. Diagnose `NOSEED` / `NORETURN` before spending another hour of wall clock.
7. **Clang TU-ERROR.** 6,324 of 22,212 analyser TUs did not build. Every one of those functions is indistinguishable from clean in a findings report and is `TU-ERROR` in the matrix. `includes.py` is the lever; the kernel `device_if.h` / userland `INCS` path is already documented in `tools/verify/README.md`.
8. **CBMC TIMEOUT (1,253) and BOUNDED (512).** Raising `--unwind` or `--timeout` is a different question per function, not a global knob. Start with `sys/` TIMEDOUT functions that classify still calls SCALAR.

## Cover the trees this run left UNTOUCHED

9. **KDE compile DB (19 Sep).** 732 TUs in `/home/odin/kde-compile-db-build/compile_commands.json` (KF6 + Wayland). A cxx-analyze ingest of that DB is still UNTOUCHED in the matrix.
10. **IR transfer of the 40-twin queue (19 Sep).** 40/40 `ir.equal` ∧ `abi_equal` on this tree's HARDENEDBSD `vm.ufs.raw`. Evidence: [queue/ir-oracle-pbsd.jsonl](queue/ir-oracle-pbsd.jsonl). The rest of `pbsd/` (15,742 functions) is still UNTOUCHED.
11. **`hbsd/tests` and `hbsd/tools`** only if someone wants them in the denominator. They are 4,270 UNTOUCHED rows that inflate the headline if left in the universe and never scoped.

## Class coverage

12. **Install CodeQL** if you want paper 0.88 on this box (currently 0.87). Infer buys no new COVERED class.
13. **Do not round PARTIAL up.** Infoleaks, lock order, trust-boundary input, and C++ exception leaks stay PARTIAL until an instrument actually PROVES or FINDS them with useful recall.

## Housekeeping

14. **Done (19 Sep).** `cxx_analyze.py` / `test_cxx_analyze.py` default `PBSD_ROOT` from the repository root (`Path(__file__).parents[2]`), same as `cbmc_driver.py`.
15. Keep packs in `docs/verify/sweep-<date>/evidence/` and leave `~/pbsd-sweep` as the working tree. Do not commit `matrix.jsonl` (84 MB, regenerable) or `classes.json` (74 MB, regenerable).
16. Never quote `FAILED` 2,538, tidy 16,410, or warnings 192,383 as bug counts. The sentences that are allowed are in [REPORT.md](REPORT.md).
