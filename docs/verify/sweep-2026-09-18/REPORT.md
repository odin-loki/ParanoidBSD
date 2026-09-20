# Whole-tree verify sweep, 18 September 2026

**Confirmed bugs found by this run: zero.**
**Functions whose checked properties are proved: 6,237 of 335,855 (1.86%).**
**Functions some instrument at least looked at: 284,073 (84.6%).**

Those three sentences are the report. Everything below is why they are not the same number, and why 2,538 CBMC `FAILED` records are not 2,538 bugs.

The method is [`tools/verify/README.md`](../../../tools/verify/README.md).
The words are [GLOSSARY.md](GLOSSARY.md).
The next work is [TODO.md](TODO.md).
Confirmed defects from *earlier* runs remain in [`docs/security/UB_FINDINGS.md`](../../security/UB_FINDINGS.md).

---

## Absences, first

A summary that lists findings and omits the instruments that never produced evidence is the lie this harness exists to prevent.

| What | Status | Why it is not evidence |
|---|---|---|
| ESBMC | 10,868 records, all `ERROR` | Linux binary, no preprocessed FreeBSD sources. Invoked, answered nothing. |
| FuSeBMC | 292,752 records: 258,497 `ERROR`, 26,770 `NOSEED`, 6,102 `NOFUNC`, 1,188 `NORETURN`, 182 `CLEAN`, 13 `CRASH` | The method ran; it did not prove. Ingest kept `ERROR`/`NOFUNC` only. |
| KDE C++ | 1,392 TUs × 2 stages: 6 OK, 1,386 ERROR, **0 findings** | No Qt 6.10+ headers, no `compile_commands.json`, guessed flags. The C++ matrix has 0 TOUCHED rows. |
| `pbsd/` | 15,742 functions, all UNTOUCHED | Not in `--scope`. The C++ ports were not swept. |
| `kde/` in the C matrix | 22,901 functions, all UNTOUCHED | C instruments do not apply; the C++ re-run did not ingest. |
| `hbsd/tests`, `hbsd/tools`, `hbsd/share`, `hbsd/kerberos5` | UNTOUCHED | Out of this run's scope. |
| `cxx-analyze` / `cxx-tidy` on the C tree | failed, 0.1 s, rc 2 | Drivers invoked against C scopes. Kind mismatch, not a KDE result. |
| CodeQL, Infer | not installed | Paper class coverage is 46/52; on this box it is 45/52. |
| Frontier-model reading pass | not run | 423 unread CBMC READ-THESE findings are exactly that queue. |

ESBMC's stage is marked `ok` in `state.json` because the driver exited 0 after writing 10,868 `ERROR` lines. That is not a proof column.

---

## How many bugs did we find?

**None that this run has confirmed.**
Confirmation here means: a person read it, UBSan (or an equivalent) reproduced it, and it landed in `UB_FINDINGS.md`. That did not happen for any new site between this sweep starting and this file being written.

What the run *did* produce is a ranked queue, not a bug count.

### The only queue worth a person's time (CBMC)

`report.py` ran over 2,538 `FAILED` functions and threw most of them out as the harness's own known lies:

| Bucket | Count | Already in `UB_FINDINGS.md` |
|---|---:|---:|
| pointer/memory — missing precondition, not a bug | 1,481 | — |
| static — callers constrain the domain, deferred | 380 | — |
| per-CPU read CBMC cannot model | 132 | — |
| float div-by-zero (IEEE-754) | 88 | — |
| extern-driven | 18 | — |
| CBMC could not model it | 4 | — |
| **EXPORTED, arithmetic — READ THESE** | **337** | **25** |
| index out of its array's range — READ THESE | 36 | 1 |
| static, but its address is taken — READ THESE | 62 | 0 |

**435** in a READ-THESE bucket, **423** not yet in the not-a-defect table.

The arithmetic bucket is the one that has historically contained real UB (`stdc_leading_ones_uc`, `rint`, `ceil`/`floor`, `nice`). 312 of those 337 are unread. By top-level directory:

| Tree prefix | Arithmetic READ-THESE |
|---|---:|
| `contrib/` | 105 |
| `sys/` | 103 |
| `lib/` | 47 |
| `usr.bin/` | 15 |
| `sbin/` | 13 |
| `usr.sbin/` | 13 |
| `stand/` | 13 |
| `bin/` | 11 |
| `crypto/` | 7 |
| `libexec/` | 5 |
| `cddl/` | 5 |

First-party `sys/` + `lib/` is 150 candidates. Start there. `contrib/less`, `contrib/flex`, compiler-rt builtins, and dtrace test programs are the same bucket and a lower priority for ParanoidBSD.

Full records: [queue/cbmc-read-these.json](queue/cbmc-read-these.json).

### Everything else is a finding, not a bug

| Instrument | Translation units | Findings | What that number is |
|---|---:|---:|---|
| clang `--analyze` | 22,212 (15,885 OK / 6,324 ERROR / 3 TIMEOUT) | 2,901 | Path-sensitive reports, intra-TU. 953 `core.NullDereference`, 511 `unix.Malloc`. Matrix ingest named 1,607 functions `REPORTED`. |
| clang-tidy | 22,212 | 16,410 | Dominated by `bugprone-implicit-widening-of-multiplication-result` (7,149) and CERT lints. |
| cppcheck | 22,212 | 7,938 | 1,190 `nullPointer`, 715 `shiftTooManyBits`. |
| Coccinelle (in-tree semantic patches) | 22,212 | 4,882 | 3,329 `shift-into-sign`, 1,025 `onesided-index`, 285 `realloc-self`, **3 `nowait-deref`**. |
| compiler warnings | 22,212 | 192,383 | 118,057 `-Wsign-conversion`. Advisory. Systems C does this on purpose. |

clang `--analyze` compiling 15,885 of 22,212 TUs is visibility 0.715 on this scope, not a defect rate.

The 3 `nowait-deref` hits are the only Coccinelle class this tree already gates on. They need a person, not a press release.

---

## How well proven is the code?

**It is well scanned. It is not well proved.**

### Code coverage (the matrix)

Universe: 335,855 functions (`hbsd` 297,212, `kde` 22,901, `pbsd` 15,742).

| Strength | Functions | Fraction |
|---|---:|---:|
| TOUCHED | 284,073 | 0.846 |
| — PROVED | 6,237 | 0.019 |
| — CHECKED (`BOUNDED` or `FAILED`) | 3,050 | 0.009 |
| — SCANNED (TU tools only) | 274,786 | 0.818 |
| UNTOUCHED | 51,782 | 0.154 |
| — never attempted | 43,103 | needs a run (`pbsd/`, `kde/`, out-of-scope dirs) |
| — attempted, no answer | 8,679 | needs a build fix or a bound raised |

PROVED means: CBMC's checked properties (bounds, pointer, signed overflow, shifts, division, leaks — the UB tier) hold for all inputs **inside `--unwind`**. It does not mean the function returns the right answer. It does not mean ESBMC or FuSeBMC agreed (they did not answer). It does not transfer to the C++ port unless the IR oracle has `ir.equal` and `abi_equal` for that unit.

Of the 10,868 functions CBMC was actually asked about:

| Verdict | Count | Fraction of attempted |
|---|---:|---:|
| PROVED | 6,237 | 0.574 |
| FAILED | 2,538 | 0.234 |
| TIMEOUT | 1,253 | 0.115 |
| BOUNDED | 512 | 0.047 |
| ERROR | 328 | 0.030 |

57% of *attempted* functions proved. 1.86% of *functions that exist* proved. Use the second number when someone asks “how well proven is the tree.”

By first component of `hbsd/`:

| Scope | PROVED | CHECKED | SCANNED | UNTOUCHED |
|---|---:|---:|---:|---:|
| `sys/` | 2,178 | 1,198 | 131,530 | 5,075 |
| `lib/` | 787 | 390 | 9,106 | 2,671 |
| `contrib/` | 1,379 | 653 | 75,810 | 606 |
| `crypto/` | 590 | 150 | 36,605 | 53 |
| `usr.sbin/` | 509 | 242 | 8,310 | 210 |
| `usr.bin/` | 263 | 132 | 3,797 | 31 |
| `stand/` | 203 | 87 | 2,483 | 0 |
| `sbin/` | 144 | 92 | 3,808 | 25 |
| `bin/` | 73 | 28 | 965 | 0 |
| `libexec/` | 56 | 32 | 763 | 0 |
| `cddl/` | 55 | 46 | 1,591 | 8 |
| `secure/` | 0 | 0 | 18 | 0 |
| `tests/` | 0 | 0 | 0 | 1,595 |
| `tools/` | 0 | 0 | 0 | 2,675 |

`sys/` is where the kernel lives and where most of the unread arithmetic queue lives (103). 2,178 proofs there is real; 131,530 SCANNED is “clang looked at the file.”

### Class coverage (kinds of defect)

52 classes in scope, 8 named out of scope.

| | COVERED | PARTIAL | GAP | fraction COVERED |
|---|---:|---:|---:|---:|
| Paper (every instrument this file knows) | 46 | 6 | 0 | 0.88 |
| This box (`--available`) | 45 | 7 | 0 | 0.87 |

Missing on this box: CodeQL, Infer. Installing CodeQL would newly COVER one class; Infer covers nothing that is not already covered.

PARTIAL on this box (best instrument is SOME, silence proves nothing):

- `INFOLEAK-PAD` — uninitialised bytes copied across a trust boundary
- `API-PRECONDITION` — a documented contract enforced nowhere
- `LOCK-ORDER` — lock order inversion
- `TRUST-UNVALIDATED-INPUT`
- `TRUST-UNCHECKED-DEVICE`
- `TRUST-STACK-DISCLOSURE`
- `CXX-EXCEPTION-LEAK`

Named out of scope, still in the denominator so the headline cannot hide them: data races, TOCTOU, non-atomic compounds, wrong results, missing specs, crypto misuse, side channels, build-config-only defects.

### The product

Class 0.87 × code 0.846 ≈ **0.73**: of the defect *kinds* we know how to look for, we looked at about three-quarters of the functions that exist.

Proofs are **0.019** of the functions that exist.

0.73 is “the scanners ran.” 0.019 is “the model checker closed the loop.” Do not quote the first as the second.

---

## What ran

Command (C tree):

```
PYTHONUNBUFFERED=1 PATH=~/.local/bin:$PATH
python3 tools/verify/sweep_all.py \
  --scope sys lib bin sbin usr.bin usr.sbin stand contrib crypto cddl secure libexec \
  --out /home/odin/pbsd-sweep --jobs 32 --timeout 60 --resume
```

Then KDE, after a first pass that used the wrong `PBSD_ROOT`:

```
PBSD_ROOT=/home/odin/ParanoidBSD PATH=~/.local/bin:$PATH
python3 tools/verify/sweep_all.py \
  --scope kde --out /home/odin/cxx-sweep --kind cxx \
  --jobs 32 --timeout 60 --resume
```

Stage times (C tree, sequential in `sweep_all.py`):

| Stage | Seconds | Result |
|---|---:|---|
| universe | 132.5 | ok |
| classify | 3,172.6 | ok |
| cbmc | 3,129.8 | ok — the only proof column |
| esbmc | 1,715.1 | ok exit, 100% ERROR |
| fusebmc | 1,170.3 | ok exit, almost no CLEAN |
| analyze | 1,524.8 | ok |
| tidy | 1,738.1 | ok |
| cppcheck | 1,435.8 | ok |
| coccinelle | 4,128.8 | ok |
| warnings | 326.6 | ok |
| cxx-analyze | 0.1 | failed rc 2 |
| cxx-tidy | 0.1 | failed rc 2 |

Classify ran; its per-class counts are regenerable from `classes.json` (dropped from the pack as regenerable, 74 MB). CBMC only attempted functions classify put in the unguarded (`SCALAR`/`VOID`) bucket, which is why 10,868 is much smaller than 297,212.

KDE: universe 21.8 s, cxx-analyze 34.4 s, cxx-tidy 91.5 s.

Packs: [evidence/](evidence/). Digests name absences first, on purpose.

---

## What this does not say

- It does not say ParanoidBSD is safe, or that HardenedBSD/FreeBSD is.
- It does not say the C++ ports in `pbsd/` preserve the proofs. Transfer still requires `ir.equal` and `abi_equal` per [`tools/verify/README.md`](../../../tools/verify/README.md) “Transfer to PBSD”.
- It does not say clang's 2,901 analyser findings are defects. Most GEOM “garbage value” reports in earlier runs were a callee in another TU.
- It does not replace `docs/security/UB_FINDINGS.md`. That file is still the list of bugs that survived a human.

Machine-readable companion: [numbers.json](numbers.json).

---

## 19 September 2026 — what closed after this report

The 18 September numbers above are the sweep as packed. The next day closed
four queues that report listed as work, without re-running the whole tree.

### Arithmetic READ-THESE (`sys/` + `lib/`)

150 `sys/`+`lib/` exported-arithmetic failures were read first. The rest of
first-party (`bin`/`sbin`/`usr.bin`/`usr.sbin`/`stand`/`crypto`/`libexec`,
78 more) is now in the same file: **228 settled, 5 defects, 189 not-a-defect,
34 deferred**. The fifth defect is OpenSSL `BN_set_params` `1 << 31` after a
clamp to 31, fixed to `1U <<`. `inet6_option_space` is still the open
behaviour-changing one. Contrib arithmetic (105) is now also in that
file: **333 settled, 6 defect, 226 not-a-defect, 101 deferred,
remaining []**. The sixth defect is
`contrib/telnet/libtelnet/encrypt.c:findencryption` (`1 << (type-1)` on
a 0–255 wire byte; type 32 is signed `1<<31`). Unwind-32 TIMEOUT retries
then found the same type-level `1 << 31` in `gpioctl` `print_caps`
(loop always reaches 31) and makefs `ilog2` (miss path before `errx`);
both now `1U <<`. 63 less/flex/compiler-rt/byacc
records are deferred named-skips, not unread. Records:
[queue/arithmetic-triage.json](queue/arithmetic-triage.json).

### IR transfer on this tree's HARDENEDBSD image

40 `hbsd_cpp` twins from [queue/ir-transfer-queue.jsonl](queue/ir-transfer-queue.jsonl)
ran on `vm.ufs.raw` (`FreeBSD 15.1-STABLE-HBSD`, clang 21.1.8).
**40/40 `ir.equal` and `abi_equal`.** Evidence:
[queue/ir-oracle-pbsd.jsonl](queue/ir-oracle-pbsd.jsonl).

`s_atan.cpp` / `s_atanf.cpp` first `compile_fail`'d because
`bit_cast_rewrite` turned `*(volatile double *)&atanlo[3]` into
`std::bit_cast<volatile double>(atanlo)[3]` (array, and not a volatile load).
The pass now leaves volatile-same-type loads alone. That is a converter
bug, not a bug in `atan`.

### KDE `compile_commands.json`

732 TUs across KF6 + Wayland-related modules (kwayland unblocked via
plasma-wayland-protocols 1.19). A later ingest of that DB with
`cxx_analyze.py --compile-commands` and `--scope .` on the cmake build
tree mixed two populations: **148 of 149 `flagsrc=compile_commands` TUs
OK** (1 ERROR), and **34 OK / 307 ERROR** among 341 guessed moc/autogen
files the directory walk added. Quote 148/149, not 182/490. Remaining
guessed ERROR is `QObject` / KF `*_export.h` because those TUs were
never in the compile DB. `cxx_analyze.py --compile-commands-only`
drops that walk. A later filter of the mixed jsonl to
`flagsrc=compile_commands` is the same 148/149; the one ERROR is
`kwindowsystem/src/qml/KWindowSystem_org_kde_kwindowsystemPlugin.cpp`
missing its `.moc`. Evidence:
[queue/cxx-analyze-ccdb-digest.json](queue/cxx-analyze-ccdb-digest.json).
Do not quote either run as “KDE is clean.” Leftover kguiaddons /
knotifications installs later finished; the merged DB is **22 modules /
726 unique TUs** (`karchive` installed). `--compile-commands-only` now
takes that database as the universe (a cmake `--root` is not
`kde/frameworks`). A jobs=1 ingest of those 726 **finished**: **685 OK /
39 ERROR / 2 TIMEOUT**, 0 guessed. ERROR is DB-honest (14 TUs missing
`private/qtx11extras_p.h` because the DB never had `QtGuiPrivate`, plus
stale autogen/moc and two Solid `.c` TUs with `-std=c99` under clang++).
TIMEOUT is `kwayland` `registry.cpp` and `ki18n` `klocalizedcontext.cpp`.
Of 72 findings, 62 are clang on Qt headers; **two kde-src defects**
(neither vendored): `k7zip.cpp:1803` uninitialized `seqOutStream`, and
`klocalimagecacheimpl.cpp:90` (`new QPixmap` dropped when caching is off).
`notifybypopup.cpp:359` is an unparented `QDBusPendingCallWatcher` with
`deleteLater` on `finished` (clang FP). Do not quote 685 OK as “KDE is
clean.” This is still not a proof column. Evidence:
[queue/kde-compile-db-expand-digest.json](queue/kde-compile-db-expand-digest.json),
[queue/cxx-analyze-ccdb-726-digest.json](queue/cxx-analyze-ccdb-726-digest.json),
[queue/k7zip-uninit-seqOutStream.json](queue/k7zip-uninit-seqOutStream.json),
[queue/kauth-dbus-watcher-leak.json](queue/kauth-dbus-watcher-leak.json),
[queue/kde-src-clang-findings.json](queue/kde-src-clang-findings.json),
[queue/klocalimagecache-pixmap-leak.json](queue/klocalimagecache-pixmap-leak.json).

### Coccinelle `nowait-deref` (3)

| Site | Verdict |
|---|---|
| `sys/dev/enic/vnic_dev.c:60` | defect: `mrh` `M_NOWAIT` used as `bus_space_read_region_4` dest after only `rh` was checked. Fixed `if (!rh \|\| !mrh)`. Coccinelle matched `sizeof(*mrh)`; the write through NULL is the actual fault. |
| `sys/netinet/in_fib_algo.c:567` | not-a-defect: `if (lr == NULL \|\| !rn_inithead(&lr->rnh, ...))` short-circuits |
| `sys/netinet6/in6_fib_algo.c:135` | not-a-defect: same `\|\|` shape |

Index-OOB and address-taken statics: [queue/oob-static-triage.json](queue/oob-static-triage.json).
98 settled (4 defect, 74 not-a-defect, 20 deferred contrib). The four defects
are fixed: `parse8601` `tm_mon > 11`, ppp `protoname` `>= nitems`, pkru
`3u << keyidx`. `cxx_analyze.py` now defaults `PBSD_ROOT` from the tree.

### FuSeBMC (why 182 CLEAN is not a proof column)

292,752 records: 258,497 ERROR, 26,770 NOSEED, 6,102 NOFUNC, 1,188 NORETURN,
182 CLEAN, 13 CRASH. NOSEED is CBMC failing to emit a seed — top reasons are
missing contrib/crypto test headers (`testutil.h`, `includes.h`, `kmp.h`,
`math.h` via the host path). NORETURN is `_exit`/`quick_exit` (AFL cannot
tell exit from crash). ERROR is the harness not compiling on Linux. CLEAN
means the budget ran out with no crash; it is not PROVED. Another hour of
wall clock does not change that.

### Clang TU-ERROR

The live `analyze.jsonl` is 16,906 OK / 5,304 ERROR / 2 TIMEOUT (the packed
report said 6,324 ERROR). The missing headers are generated or private:
`config.h` (807 ERROR under `crypto/heimdal`; 0 under `kerberos5/`).
Those three headers already exist in-tree (`kerberos5/include/config.h`,
OpenSSL `include/internal/common.h`, AOR `math/math_config.h`). Prefix
ERROR is orphans or kernel-objdir generated files (`vdso_offsets.h`,
`acpi_wakecode.h`), not a missing `-I`. First-party `includes.py` already
passes `-I lib/msun/src`. Evidence:
[queue/analyze-error-prefix.json](queue/analyze-error-prefix.json).
`bin/csh` GENHDRS (`sh.err.h` / `ed.defns.h` / `tc.const.h`) now follow
the Makefile recipe instead of writing only `iconv.h`. A scoped
`--out` of `contrib/tcsh` (live file not resumed: resume drops every
non-OK row) is **51 OK / 3 ERROR**; the three leftovers want `config.h`
(`gethost.c`, `ma.setp.c`, `vms.termcap.c`). Evidence:
[queue/analyze-tcsh-cshhdrs-digest.json](queue/analyze-tcsh-cshhdrs-digest.json).
`contrib/wpa` TUs the build names are **183/183 OK**; the 257 ERROR are
unnamed orphans (`hlr_auc_gw`, hs20, dbus, Windows). A prefix `-I`
would compile those as if they were `usr.sbin/wpa`. These TUs stay
`TU-ERROR` until a generated config exists; they are not silent-clean.

### ESBMC live file (not the packed 10,868 ERROR)

`/home/odin/pbsd-sweep/esbmc.jsonl` is kinduction, not the packed `-xc`
column. After the ERROR retry dropped ERROR rows once, the live file is
**808 PROVED-UNBOUNDED / 344 FAILED / 38 UNKNOWN / 135 TIMEOUT /
5,108 ERROR** (6,433 rows). A `--resume` with no `--retry-status` then
reported **0 pairs left to check** against the current `classes.json`
SCALAR/VOID ∩ sys+lib set: that file is the kinduction result, not a
hole vs the packed 10,868 `-xc` column. Do **not** pass `--retry-status
ERROR` again. A 25-row sample of the 344 FAILED is libc already in the
CBMC arithmetic queue (`killpg`, `alarm`, `clock`, `nice`, `isctype`),
not 344 bugs. Text-class of all 344: 163 arithmetic, 140 pointer, 41
other. First-party (sys/ or lib/, not isctype): 172 FAILED. Fifteen
that looked like shl/OOB and were not already in the CBMC arithmetic
queue were read: **0 defects** (11 not-a-defect, 4 deferred tests).
Evidence:
[queue/esbmc-failed-sample.json](queue/esbmc-failed-sample.json),
[queue/esbmc-failed-firstparty.json](queue/esbmc-failed-firstparty.json),
[queue/esbmc-failed-triage.json](queue/esbmc-failed-triage.json).

### CBMC TIMEOUT resume (unwind 32 / 180 s, in flight)

`--retry-status TIMEOUT,BOUNDED,ERROR` already ran against
`/home/odin/pbsd-sweep/cbmc.jsonl` and dropped 2,093 rows. The unwind-16
file is `cbmc-old.jsonl` (1,253 TIMEOUT / 512 BOUNDED / 328 ERROR).
`tools/verify/run-cbmc-resume.sh` is `--resume` only so the new
BOUNDED/ERROR rows are not dropped again.

Live file while this is written: **9,757** rows, **6,292 PROVED**
(BOUNDED 180 / FAILED 2,593 / ERROR 180 / TIMEOUT 512). ~1,111 pairs
still missing. Unread new-FAILED is 0. Current TIMEOUTs
(`ksz8995ma_writereg`; live `mtkswitch_reg_write`) are unmodelled MMIO/softc.
Settled unwind-32 FAILED is 55 (2 defects). Restart remains `--resume`
only at JOBS=2 (pid 887). Records:
[queue/timeout-retry-triage.json](queue/timeout-retry-triage.json).

Do not pass `--retry-status` at the live file. `cbmc_driver.py --pair-list`
still exists so a future sample can write a *new* jsonl. `sys/` SCALAR
TIMEOUT in the old file is 755 functions. Re-indexing twins on the live
PROVED set is **44 `hbsd_cpp` files, 0 `pbsd/` twins** (four new msun
`.cpp` proved in-tree, not in clang_port staged). Evidence:
[queue/proved-twin-index.json](queue/proved-twin-index.json).

### CodeQL CLI (19 Sep)

GitHub CodeQL 2.27.0 is at `~/.local/codeql` and on PATH.
`taxonomy.py --available` is now **46/52 COVERED (0.88)**;
`TRUST-UNVALIDATED-INPUT` is COVERED because this instrument rates
FINDS. A one-TU smoke (`tools/verify/codeql_smoke.py`) compiles from
`hbsd/src` with `include_flags()` (not a copied-out empty tree) and saw
`abs` and `sys_ffclock_setestimate`. The first `copyin()` query on the
ffclock database was **EMPTY** because HARDENEDBSD does not set
`FFCLOCK`: `kern_ffclock.c` is `standard` in `sys/conf/files` but
`opt_shim()` writes an empty `opt_ffclock.h`, so the extracted body is
the real `#else` stub `return (ENOSYS)`. That is not an extractor
failure. `sys/kern/kern_time.c` `sys_clock_settime` (copyin is not
option-gated) is **COPYIN-OK**: the query sees `copyin` in
`sys_clock_settime`, `user_clock_nanosleep`, `sys_settimeofday`,
`sys_setitimer`, `sys_ktimer_create`, `sys_ktimer_settime`. Forcing `-DFFCLOCK` on the
same ffclock TU is also **COPYIN-OK** (`copyin` in
`sys_ffclock_setestimate`). Three more kernel TUs, still one extract
each, are also **COPYIN-OK**: `kern_context.c` (`sys_setcontext`,
`sys_swapcontext`), `kern_prot.c` (`sys_setgroups`,
`freebsd14_setgroups`, `sys_setcred`,
`user_setcred_copyin_supp_groups`), `kern_resource.c`
(`sys_setrlimit`, `sys_rtprio`, `sys_rtprio_thread`). A later local
dest-use query (`copyin_dest_used.ql`) on those same DBs is
**DEST-USED-OK** on `kern_prot.c` (`groups` in `sys_setgroups`,
`freebsd14_setgroups`, `user_setcred_copyin_supp_groups`) and
**DEST-USED-EMPTY** on time/context/resource/ffclock-on: dest as
`&local` is not a named `VariableAccess`. That is still not a taint
run. Infer is still absent and covers nothing extra.
Evidence:
[queue/codeql-smoke.jsonl](queue/codeql-smoke.jsonl),
[queue/codeql-ffclock.jsonl](queue/codeql-ffclock.jsonl),
[queue/codeql-time.jsonl](queue/codeql-time.jsonl),
[queue/codeql-copyin.jsonl](queue/codeql-copyin.jsonl),
[queue/codeql-ffclock-on.jsonl](queue/codeql-ffclock-on.jsonl),
[queue/codeql-ffclock-on-copyin.jsonl](queue/codeql-ffclock-on-copyin.jsonl),
[queue/codeql-context.jsonl](queue/codeql-context.jsonl),
[queue/codeql-context-copyin.jsonl](queue/codeql-context-copyin.jsonl),
[queue/codeql-prot.jsonl](queue/codeql-prot.jsonl),
[queue/codeql-prot-copyin.jsonl](queue/codeql-prot-copyin.jsonl),
[queue/codeql-resource.jsonl](queue/codeql-resource.jsonl),
[queue/codeql-resource-copyin.jsonl](queue/codeql-resource-copyin.jsonl),
[queue/codeql-time-dest.jsonl](queue/codeql-time-dest.jsonl),
[queue/codeql-prot-dest.jsonl](queue/codeql-prot-dest.jsonl),
[queue/codeql-context-dest.jsonl](queue/codeql-context-dest.jsonl),
[queue/codeql-resource-dest.jsonl](queue/codeql-resource-dest.jsonl),
[queue/codeql-ffclock-on-dest.jsonl](queue/codeql-ffclock-on-dest.jsonl).
