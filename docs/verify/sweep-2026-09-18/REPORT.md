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
a 0–255 wire byte; type 32 is signed `1<<31`). 63 less/flex/compiler-rt/byacc
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
`cxx_analyze.py --compile-commands` built **182 of 490** TUs (the old
guessed-flag run was 6 of 1,392). The rest are still ERROR, mostly moc
autogen missing Qt headers. Clang findings in `qobjectdefs.h` are Qt's
header, not KDE. Do not quote either run as “KDE is clean.”

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
`config.h`, OpenSSH `includes.h`, OpenSSL `internal/common.h`, contrib
`math_config.h` (arm-optimized-routines, not lib/msun). First-party
`includes.py` already passes `-I lib/msun/src`. These TUs stay `TU-ERROR`
until a generated config exists; they are not silent-clean.

### ESBMC live file (not the packed 10,868 ERROR)

`/home/odin/pbsd-sweep/esbmc.jsonl` now has mixed statuses (kinduction):
ERROR 9,544, PROVED-UNBOUNDED 808, FAILED 344, TIMEOUT 134, UNKNOWN 38.
The 18 September pack was the `-xc` unrecognised-option run. 808 unbounded
proofs exist; they are not in the packed digest. A hung `--smoke` of
`stdc_has_single_bit_uc` was blocking the ERROR retry. The ERROR retry
(`--resume --retry-status ERROR --mode kinduction`, jobs 8) is left
running; it must not be SIGTERM'd.

### CBMC TIMEOUT resume (unwind 32 / 180 s, in flight)

`--retry-status TIMEOUT,BOUNDED,ERROR` already ran against
`/home/odin/pbsd-sweep/cbmc.jsonl` and dropped 2,093 rows. The unwind-16
file is `cbmc-old.jsonl` (1,253 TIMEOUT / 512 BOUNDED / 328 ERROR).
`tools/verify/run-cbmc-resume.sh` is `--resume` only so the new
BOUNDED/ERROR rows are not dropped again.

Live file while this is written: **8,836** rows, **6,241 PROVED** (four
new: `svc_exit`, `sysconf`, `getosreldate`, `mixer_get_nmixers`, all
were BOUNDED at unwind 16). Four new FAILED at unwind 32 were read:
two test programs deferred, `arc4random` / `localeconv` are unmodelled
auxv/locale pointers, not defects. Records:
[queue/timeout-retry-triage.json](queue/timeout-retry-triage.json).

Do not pass `--retry-status` at the live file. `cbmc_driver.py --pair-list`
still exists so a future sample can write a *new* jsonl. `sys/` SCALAR
TIMEOUT in the old file is 755 functions. Re-indexing twins on the live
PROVED set is still **40 `hbsd_cpp` files, 0 `pbsd/` twins**.

### CodeQL CLI (19 Sep)

GitHub CodeQL 2.27.0 is at `~/.local/codeql` and on PATH.
`taxonomy.py --available` is now **46/52 COVERED (0.88)**;
`TRUST-UNVALIDATED-INPUT` is COVERED because this instrument rates
FINDS. That is presence, not a taint run: no CodeQL database has been
built on this tree. Infer is still absent and covers nothing extra.
Installer: [tools/verify/install_codeql.py](../../../tools/verify/install_codeql.py)
(zip extracts drop execute bits; the script restores them on ELF and
shebang files).
