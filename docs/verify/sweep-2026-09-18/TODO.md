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
7. **Clang TU-ERROR (20 Sep).** Live packed file is still 16,906 OK / 5,304 ERROR / 2 TIMEOUT. `bin/csh` GENHDRS and `contrib/wpa` named 183/183 as before. The three “missing” headers already exist: `kerberos5/include/config.h`, OpenSSL `include/internal/common.h`, AOR `math/math_config.h`. Top ERROR prefixes are orphans or objdir-generated, not silent-clean stubs: `crypto/heimdal` 807 (default `MK_MITKRB5=yes` builds MIT, not Heimdal), `crypto/krb5` 564, `crypto/openssl` 411, `contrib/llvm-project` 356, `contrib/wpa` 257, AOR 254. Named amd64 kernel leftovers (`vdso_offsets.h`, `vdso_ia32_offsets.h`, `acpi_wakecode.h`) are kernel-objdir recipes `gen_headers()` does not run. No cheap `-I`. Do **not** `--resume` the live `analyze.jsonl` with a narrowed `--scope`. Evidence: [queue/analyze-error-prefix.json](queue/analyze-error-prefix.json).
8. **CBMC TIMEOUT resume (JOBS=2).** Live file 9,854 rows, 6,294 PROVED, hole ~1,014. Unread new-FAILED is **0**. Current last is BOUNDED `init_crc16tab`. Settled 60, 3 defects. Pid 887 `--resume` only. Do **not** pass `--retry-status`.

## Cover the trees this run left UNTOUCHED

9. **Done (20 Sep).** KDE 726-TU `--compile-commands-only` ingest **finished**: **685 OK / 39 ERROR / 2 TIMEOUT**, 0 guessed. Findings 72: 62 Qt; 10 kde-src (k7zip + klocalimagecache still the only read upstream defects). ERROR is DB-honest (qtx11extras private `-I` missing from the DB, stale autogen/moc, Solid `-std=c99` under clang++). TIMEOUT: `kwayland` `registry.cpp`, `ki18n` `klocalizedcontext.cpp`. **Not a KDE proof column.** Do not quote 685 OK as clean. Evidence: [queue/cxx-analyze-ccdb-726-digest.json](queue/cxx-analyze-ccdb-726-digest.json).
10. **IR transfer of the 40-twin queue (19 Sep).** 40/40 `ir.equal` ∧ `abi_equal` on this tree's HARDENEDBSD `vm.ufs.raw`. Re-index of the live PROVED set is **44 `hbsd_cpp` files** (four new msun `.cpp`: `s_asinh`, `s_ilogb`, `s_ilogbf`, `s_ilogbl`) and **0 `pbsd/` twins**. Evidence: [queue/proved-twin-index.json](queue/proved-twin-index.json).
11. **`hbsd/tests` and `hbsd/tools`** only if someone wants them in the denominator. They are 4,270 UNTOUCHED rows that inflate the headline if left in the universe and never scoped.

## Class coverage

12. **Done (19 Sep).** CodeQL CLI 2.27.0. Smokes and COPYIN-OK as before. A local dest-use query (`copyin_dest_used.ql`) on the same one-TU DBs is **DEST-USED-OK** on `kern_prot.c` (`groups` in `sys_setgroups` / `freebsd14_setgroups` / `user_setcred_copyin_supp_groups`) and **DEST-USED-EMPTY** on time/context/resource/ffclock-on (`&local` dests are not a named `VariableAccess`). Still not a taint run. Infer still absent.
13. **Do not round PARTIAL up.** Infoleaks, lock order, trust-boundary input, and C++ exception leaks stay PARTIAL until an instrument actually PROVES or FINDS them with useful recall.

## Housekeeping

14. **Done (19 Sep).** `cxx_analyze.py` / `test_cxx_analyze.py` default `PBSD_ROOT` from the repository root (`Path(__file__).parents[2]`), same as `cbmc_driver.py`.
15. Keep packs in `docs/verify/sweep-<date>/evidence/` and leave `~/pbsd-sweep` as the working tree. Do not commit `matrix.jsonl` (84 MB, regenerable) or `classes.json` (74 MB, regenerable).
16. Never quote `FAILED` 2,538, tidy 16,410, or warnings 192,383 as bug counts. The sentences that are allowed are in [REPORT.md](REPORT.md).
