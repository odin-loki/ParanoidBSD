# What to do next

Ranked by whether it changes a coverage number someone might quote, or
whether it turns a queue item into a confirmed bug.

## Confirm or kill the CBMC queue

1. **Read the 103 `sys/` + 47 `lib/` exported-arithmetic failures** in [queue/cbmc-read-these.json](queue/cbmc-read-these.json). Same process as `UB_FINDINGS.md`: read the code, UBSan if it is arithmetic UB, then either fix or write the reasoning that kills it.
2. **Read the 35 unread index-OOB and 62 address-taken statics.** The static-but-`&fn` bucket is the one `report.py` split out because in-file callers do *not* constrain those.
3. **Run a frontier-model pass only on READ-THESE**, never on the 1,481 pointer/memory records. `taxonomy.py --needs-model` is the class list; this JSON is the finding list. A model's answer is a hypothesis until it is checked against the source.
4. **Look at the 3 `nowait-deref` Coccinelle hits.** That checker already gates. Three sites is a short read.

Skip, until the first-party queue is empty: `contrib/less`, flex, compiler-rt builtins, dtrace test programs, byacc `vdiv` fixtures.

## Make the instruments that ran actually answer

5. **ESBMC.** Either feed it preprocessed FreeBSD TUs (the Linux binary cannot parse this tree as-is) or stop marking the stage `ok` when every cell is `ERROR`. 10,868 ERROR rows currently look like a column in the matrix and are UNTOUCHED for proof purposes.
6. **FuSeBMC.** 182 `CLEAN` out of 292,752 is not a fuzz-guided proof run. Diagnose `NOSEED` / `NORETURN` before spending another hour of wall clock.
7. **Clang TU-ERROR.** 6,324 of 22,212 analyser TUs did not build. Every one of those functions is indistinguishable from clean in a findings report and is `TU-ERROR` in the matrix. `includes.py` is the lever; the kernel `device_if.h` / userland `INCS` path is already documented in `tools/verify/README.md`.
8. **CBMC TIMEOUT (1,253) and BOUNDED (512).** Raising `--unwind` or `--timeout` is a different question per function, not a global knob. Start with `sys/` TIMEDOUT functions that classify still calls SCALAR.

## Cover the trees this run left UNTOUCHED

9. **KDE, properly.** Qt 6.10+ and a real `compile_commands.json`. Until then a KDE sweep will keep reporting 0 findings on 1,386 compile errors. Do not quote that as “KDE is clean”.
10. **Sweep `pbsd/`.** 15,742 functions, all UNTOUCHED. That is the C++ port, which is the product. CBMC's C++ front end still cannot parse `noexcept`; the intended path is still “prove the C, transfer via IR oracle.”
11. **`hbsd/tests` and `hbsd/tools`** only if someone wants them in the denominator. They are 4,270 UNTOUCHED rows that inflate the headline if left in the universe and never scoped.

## Class coverage

12. **Install CodeQL** if you want paper 0.88 on this box (currently 0.87). Infer buys no new COVERED class.
13. **Do not round PARTIAL up.** Infoleaks, lock order, trust-boundary input, and C++ exception leaks stay PARTIAL until an instrument actually PROVES or FINDS them with useful recall.

## Housekeeping

14. The WSL copy of `cxx_analyze.py` now defaults `PBSD_ROOT` from the tree rather than `/home/user/paranoidbsd`. That patch is not in this OneDrive clone; port it or the next KDE run will scan an empty path again.
15. Keep packs in `docs/verify/sweep-<date>/evidence/` and leave `~/pbsd-sweep` as the working tree. Do not commit `matrix.jsonl` (84 MB, regenerable) or `classes.json` (74 MB, regenerable).
16. Never quote `FAILED` 2,538, tidy 16,410, or warnings 192,383 as bug counts. The sentences that are allowed are in [REPORT.md](REPORT.md).
