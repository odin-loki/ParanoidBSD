# What to do next

Ranked by whether it changes a coverage number someone might quote, or
whether it turns a queue item into a confirmed bug.

## Confirm or kill the CBMC queue

1. **Done (19 Sep).** Exported-arithmetic is closed: 333 settled in [queue/arithmetic-triage.json](queue/arithmetic-triage.json) (228 first-party + 105 contrib). 6 defect, 226 not-a-defect, 101 deferred (including 63 named skip: less/flex/compiler-rt/byacc). Remaining `[]`.
2. **Done (19 Sep).** 35 unread index-OOB + 62 address-taken statics read; records in [queue/oob-static-triage.json](queue/oob-static-triage.json). Four defects fixed (`days_pmonth`/`parse8601`, ppp `protoname` ×2, pkru `3<<30`). Contrib remainder is deferred.
3. **Frontier-model on READ-THESE (19 Sep).** First-party arithmetic/OOB/static and contrib arithmetic are read. Do not run a model on the 1,481 pointer/memory records.
4. **Done (19 Sep).** `vnic_dev.c` `mrh` was a real unchecked `M_NOWAIT` (fixed). The two `in*_fib_algo.c` hits are `p == NULL || p->field` short-circuit false positives.

Skip, until the first-party queue is empty: `contrib/less`, flex, compiler-rt builtins, dtrace test programs, byacc `vdiv` fixtures.

## Make the instruments that ran actually answer

5. **ESBMC (19 Sep).** Packed run was 10,868 ERROR (`-xc`). Live kinduction file is complete: 808 PROVED-UNBOUNDED / 344 FAILED / 38 UNKNOWN / 135 TIMEOUT / 5,108 ERROR (6,433 rows). First-party FAILED (sys/ or lib/, not isctype): 172. Fifteen shl/OOB-looking sites not already in CBMC arithmetic-triage were read: **0 defects**. Do **not** pass `--retry-status ERROR`. Evidence: [queue/esbmc-failed-triage.json](queue/esbmc-failed-triage.json).
6. **FuSeBMC (19 Sep).** Diagnosed: NOSEED is CBMC seed failure on missing contrib/crypto test headers; NORETURN is `_exit`; ERROR is harness compile on Linux; 182 CLEAN is budget-not-crash, not a proof.
7. **Clang TU-ERROR (19 Sep).** Live packed file is still 16,906 OK / 5,304 ERROR / 2 TIMEOUT. `bin/csh` GENHDRS now follow the Makefile recipe; scoped `contrib/tcsh` is **51 OK / 3 ERROR**. `contrib/wpa` **named** TUs are 183/183 OK; the 257 ERROR are unnamed orphans (`hlr_auc_gw`, hs20, dbus, Windows) the build does not compile. A prefix `-I` would wrongly compile those. Remaining top missing: heimdal `config.h`, OpenSSL internals, contrib `math_config.h`. Not silent-clean stubs. Do **not** `--resume` the live `analyze.jsonl` with a narrowed `--scope`.
8. **CBMC TIMEOUT resume (in flight).** Live file 9,487 rows, 6,282 PROVED, hole ~1,381. Unread new FAILED: 0. Thirty-six earlier FAILED: 2 defects patched, 2 deferred tests, 32 not-a-defect. Do **not** pass `--retry-status` again.

## Cover the trees this run left UNTOUCHED

9. **KDE compile DB (19 Sep).** 148/149 `flagsrc=compile_commands` TUs OK. The 182/490 figure mixed in 341 guessed moc/autogen files (`--scope .`). Use `--compile-commands-only`. The one compile_commands ERROR is `kwindowsystem` QML plugin missing `.moc`. Still not a KDE proof column. Evidence: [queue/cxx-analyze-ccdb-digest.json](queue/cxx-analyze-ccdb-digest.json).
10. **IR transfer of the 40-twin queue (19 Sep).** 40/40 `ir.equal` ∧ `abi_equal` on this tree's HARDENEDBSD `vm.ufs.raw`. Evidence: [queue/ir-oracle-pbsd.jsonl](queue/ir-oracle-pbsd.jsonl). Re-index of the live PROVED set is still 40 `hbsd_cpp` files and **0 `pbsd/` twins**. The rest of `pbsd/` (15,742 functions) cannot inherit a C proof they do not have a twin for.
11. **`hbsd/tests` and `hbsd/tools`** only if someone wants them in the denominator. They are 4,270 UNTOUCHED rows that inflate the headline if left in the universe and never scoped.

## Class coverage

12. **Done (19 Sep).** CodeQL CLI 2.27.0. Smokes and COPYIN-OK as before. A local dest-use query (`copyin_dest_used.ql`) on the same one-TU DBs is **DEST-USED-OK** on `kern_prot.c` (`groups` in `sys_setgroups` / `freebsd14_setgroups` / `user_setcred_copyin_supp_groups`) and **DEST-USED-EMPTY** on time/context/resource/ffclock-on (`&local` dests are not a named `VariableAccess`). Still not a taint run. Infer still absent.
13. **Do not round PARTIAL up.** Infoleaks, lock order, trust-boundary input, and C++ exception leaks stay PARTIAL until an instrument actually PROVES or FINDS them with useful recall.

## Housekeeping

14. **Done (19 Sep).** `cxx_analyze.py` / `test_cxx_analyze.py` default `PBSD_ROOT` from the repository root (`Path(__file__).parents[2]`), same as `cbmc_driver.py`.
15. Keep packs in `docs/verify/sweep-<date>/evidence/` and leave `~/pbsd-sweep` as the working tree. Do not commit `matrix.jsonl` (84 MB, regenerable) or `classes.json` (74 MB, regenerable).
16. Never quote `FAILED` 2,538, tidy 16,410, or warnings 192,383 as bug counts. The sentences that are allowed are in [REPORT.md](REPORT.md).
