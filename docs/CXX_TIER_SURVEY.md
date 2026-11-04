# SURVEY — the C++ analysis tier, measured before it was built

Measured 2026-09-17 in the analysis container. Every number below has
the command that produced it and the sample size beside it. Anything
not measured says "NOT MEASURED".

Host: 4 cores (`nproc`), Intel Xeon @ 2.10GHz, 16 GB RAM, Linux
6.18.44. Every timing in this document and in `RUNBOOK.md` is from
this host at this core count.

---

## 0. The toolchain that is actually here

Each verified by running it, not by looking for a binary.

| tool | result |
|---|---|
| `clang++` | **Ubuntu clang version 18.1.3 (1ubuntu1)**, x86_64-pc-linux-gnu |
| `clang` | same 18.1.3 |
| `clang-tidy` | **Ubuntu LLVM version 18.1.3** |
| `scan-build` | present (`scan-build --help` responds) |
| `gcc` | 13.3.0 |
| `cmake` | **3.28.3** |
| `python3` | 3.11.15 |
| `cppcheck` | **2.13.0 — PRESENT.** The brief said it was not. See §5. |
| `cbmc` | 5.95.1, and `goto-cc` — the C tier's model checker is here |
| `infer`, `esbmc`, `fusebmc`, `semgrep`, `codeql`, `frama-c` | absent, as stated |
| `include-what-you-use` | absent |

### Qt and KDE build dependencies: absent, all of them

```
pkg-config --list-all | grep -iE 'qt|kf5|kf6'      -> no output
ls /usr/include/{qt5,qt6}, /usr/include/x86_64-linux-gnu/{qt5,qt6}
                                                   -> No such file or directory
find / -name QObject -o -name qobject.h            -> no output
find / -name 'ECMConfig.cmake'                     -> no output
which moc qmake qmake6 uic rcc                     -> none found
```

`cmake` ships only the legacy `FindQt4.cmake` family. There is no Qt5,
no Qt6, no KDE Frameworks, no `extra-cmake-modules`, and no
meta-object compiler anywhere on this machine.

**This is the central fact of the whole tier and it is not routed
around anywhere in what follows.**

---

## 1. Does cmake configure any of the ten frameworks?

**No. 0 of 12 projects configure. 0 `compile_commands.json` exist.**

Method: out-of-source configure into `/tmp/agent_cxx/cmake_probe/`, one
per project (nothing written under `/home/user/paranoidbsd`):

```
cmake -S <project> -B <builddir> -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
```

| project | rc | compile_commands.json | first error |
|---|---|---|---|
| frameworks/kconfig | 1 | NONE | Could NOT find ECM (missing: ECM_DIR) |
| frameworks/kcoreaddons | 1 | NONE | Could NOT find ECM |
| frameworks/kdecoration | 1 | NONE | CMakeLists.txt:14 `find_package(ECM ... REQUIRED)` |
| frameworks/kio | 1 | NONE | Could NOT find ECM |
| frameworks/kservice | 1 | NONE | Could NOT find ECM |
| frameworks/kwayland | 1 | NONE | CMakeLists.txt:12 `find_package` |
| frameworks/kwindowsystem | 1 | NONE | Could NOT find ECM |
| frameworks/kxmlgui | 1 | NONE | Could NOT find ECM |
| frameworks/layer-shell-qt | 1 | NONE | CMakeLists.txt:19 `find_package` |
| frameworks/plasma-framework | 1 | NONE | CMakeLists.txt:13 `find_package` |
| kwin | 1 | NONE | CMakeLists.txt:15 `find_package` |
| plasma-desktop | 1 | NONE | CMakeLists.txt:17 `find_package` |

Sample size: **all 12**, not a sample. Every one dies at
`find_package(ECM)` — the *first* dependency line in every
`CMakeLists.txt`. Nothing gets as far as `find_package(Qt6)`, so the Qt
failure is not even reached; it is behind the ECM failure.

---

## 2. Exactly which external dependencies are missing

Enumerated from every `find_package(...)` in the three trees
(`grep -rhoE 'find_package\s*\(\s*[A-Za-z0-9_.+-]+'`, 27 distinct
`Qt6*`, 21 `ECM`, 12 `KF6*` call sites) and the declared minimums
(`QT_MIN_VERSION`, `KF6_MIN_VERSION`).

**Declared minimums found in the tree — note how new these are:**

```
QT_MIN_VERSION  "6.5.0"   "6.7.0"   "6.10.0"
KF6_MIN_VERSION "6.0.0"   "6.22.0"
ECM             6.6.0 and 6.22.0
```

`kdecoration`, `kwayland` and `kwin` want **Qt 6.10** and **KF 6.22**.
Debian/Ubuntu stable does not ship those; see RUNBOOK §"what will still
not configure".

### Debian / Ubuntu

```
extra-cmake-modules                       <- ECM. The one that blocks all 12.
qt6-base-dev qt6-base-private-dev
qt6-declarative-dev qt6-declarative-private-dev
qt6-wayland qt6-wayland-dev qt6-wayland-private-dev
qt6-tools-dev qt6-tools-dev-tools qt6-5compat-dev qt6-svg-dev
libqt6opengl6-dev libqt6sql6-dev
libkf6config-dev libkf6coreaddons-dev libkf6i18n-dev libkf6service-dev
libkf6windowsystem-dev libkf6xmlgui-dev libkf6kio-dev libkf6archive-dev
libkf6bookmarks-dev libkf6configwidgets-dev libkf6dbusaddons-dev
libkf6globalaccel-dev libkf6guiaddons-dev libkf6iconthemes-dev
libkf6itemviews-dev libkf6widgetsaddons-dev libkf6kcmutils-dev
libkf6notifications-dev libkf6package-dev libkf6crash-dev
libkf6doctools-dev libkf6kirigami-dev libkf6kded-dev
libplasma-dev libplasmaactivities-dev libplasmaactivitiesstats-dev
plasma-wayland-protocols
libwayland-dev wayland-protocols libwayland-egl-backend-dev
libxcb1-dev libxcb-*-dev libx11-dev libx11-xcb-dev libxkbcommon-dev
libegl-dev libgbm-dev libepoxy-dev libdrm-dev
libudev-dev libinput-dev liblcms2-dev libcanberra-dev
libqaccessibilityclient-qt6-dev hwdata
```

### FreeBSD (the real target — the user's home PC)

```
pkg install \
  devel/cmake devel/extra-cmake-modules devel/ninja \
  devel/qt6-base devel/qt6-declarative devel/qt6-tools devel/qt6-wayland \
  devel/qt6-5compat graphics/qt6-svg \
  devel/kf6-kconfig devel/kf6-kcoreaddons devel/kf6-kservice \
  devel/kf6-ki18n devel/kf6-kwindowsystem devel/kf6-kxmlgui \
  devel/kf6-kio devel/kf6-karchive devel/kf6-kbookmarks \
  devel/kf6-kconfigwidgets devel/kf6-kdbusaddons devel/kf6-kglobalaccel \
  devel/kf6-kguiaddons devel/kf6-kiconthemes devel/kf6-kitemviews \
  devel/kf6-kwidgetsaddons devel/kf6-kcmutils devel/kf6-knotifications \
  devel/kf6-kpackage devel/kf6-kcrash devel/kf6-kdoctools \
  x11-toolkits/kf6-kirigami \
  x11/plasma6-libplasma x11/plasma6-plasma-activities \
  x11/plasma6-plasma-wayland-protocols \
  graphics/wayland graphics/wayland-protocols \
  x11/libxcb x11/libX11 x11/libxkbcommon graphics/libepoxy \
  graphics/mesa-libs graphics/libdrm devel/libudev-devd x11/libinput \
  graphics/lcms2 audio/libcanberra devel/llvm18
```

`devel/llvm18` matters: the analysis tier is pinned to clang 18's
checker set and a different clang is a different instrument. See
`cxx_analyze.py`'s `_meta.analyzer` field.

**NOT MEASURED:** whether these exact package names resolve on the
user's FreeBSD version. They are read off the ports tree's naming
convention, not installed and verified here — there is no FreeBSD in
this container.

---

## 3. How much KDE C++ can clang parse *today*, with no Qt?

### 3a. Bare `clang++ -fsyntax-only -std=c++20`, no include flags

**Sample size 50** (seeded random from all 1,392 `.cpp`, seed 1729).

```
PASS = 0    FAIL = 50    (0.0% parse)
40 distinct first-error messages
```

Top distinct first errors:

| n | message |
|---|---|
| 3 | missing header: `kiocore_export.h` |
| 3 | missing header: `QObject` |
| 3 | missing header: `effect/effect.h` |
| 2 | missing header: `kxmlgui_export.h` |
| 2 | missing header: `config-kwin.h` |
| 2 | missing header: `KCModule` |
| 2 | missing header: `kwin_export.h` |
| 1 | missing header: `QCoreApplication` |
| 1 | missing header: `QExplicitlySharedDataPointer` |
| 1 | missing header: `kdecoration3/private/kdecoration3_private_export.h` |

### 3b. Best-effort include guess + synthesised generated headers — **CENSUS, all 1,392**

This is the configuration `cxx_analyze.py` ships with: every
header-bearing directory of the owning project and its siblings on
`-I`, plus 49 synthesised `*_export.h` / `config-*.h` files that cmake
would have generated.

```
census = 1392    PASS = 6    FAIL = 1386    (0.43% parse)
254 distinct first-error messages
wall 2m23s at 8 threads on 4 cores
```

Top distinct first errors:

| n | message |
|---|---|
| 264 | missing header: `QObject` |
| 173 | missing header: `QDataStream` |
| 81 | missing header: `QCoreApplication` |
| 56 | missing header: `QList` |
| 34 | missing header: `QMatrix4x4` |
| 28 | missing header: `QString` |
| 26 | missing header: `QStringList` |
| 20 | missing header: `KCompositeJob` |
| 18 | missing header: `QHash` |
| 15 | missing header: `QByteArray` |

### 3b-bis. CONFIRMED by the real instrument, not just by `-fsyntax-only`

`cxx_analyze.py` was then run over the whole KDE tier — the same 1,392
files, `clang --analyze` rather than `-fsyntax-only`, the shipping
configuration:

```
1392 translation unit(s) to analyse across 3 scope(s)
     565  kde/frameworks
     633  kde/kwin
     194  kde/plasma-desktop
  flag source: 0 from compile_commands.json, 1392 GUESSED

6 translation unit(s) BUILT, 1386 did not (ERROR=1386)
0 finding(s) across the 6 that built
  OK       6
  ERROR    1386
exit 2       <- "0 translation units BUILT ... VISIBILITY is zero"
wall 3m26s at --jobs 6 on 4 cores
```

The syntax census and the analyser agree exactly: **6 and 1,386.**

**The six that parse** — the entire visible KDE corpus today, and all
six report zero findings, which here means "six files were checked and
were clean", not "the KDE tree is clean":

```
kde/frameworks/kconfig/src/gui/kconfigguistaticinitializer.cpp
kde/frameworks/kio/src/kioworkers/file/kauth/fdsender.cpp
kde/kwin/src/core/renderjournal.cpp
kde/kwin/src/plugins/zoom/focustracker.cpp
kde/kwin/src/utils/realtime.cpp
kde/plasma-desktop/kcms/touchpad/backends/x11/xcbatom.cpp
```

**0.43% visibility. The run exits 2 and says so, rather than printing
"0 findings" and letting it read as a clean tree.**

### 3c. How deep does the Qt dependency go?

Textual scan of all 1,392 `.cpp` for a direct `#include` of a Qt header,
plus the same scan of each file's same-named `.h`:

```
direct #include of a Qt header        901
Qt only through its own .h            282
neither                               209
                                   ------
                                    1,392
>= 85.0% need Qt headers  (LOWER BOUND - transitive includes not followed)
```

The 209 "neither" files were then actually compiled, which turns the
lower bound into a measurement:

| configuration | of 209 |
|---|---|
| include guess only | **3 parse** |
| include guess + generated-header shim | **6 parse** |

So the shim converted 136 "missing generated header" failures into
"missing `QObject`" failures and bought three files. **It does not open
the door; it shows you the next one.**

### 3d. What the generated-header shim is and is not

Two families, and the second is the one to distrust:

* `*_export.h` — what `GenerateExportHeader` writes. A handful of
  visibility and deprecation macros. Synthesising it is close to
  lossless.
* `config-<project>.h` — what cmake writes *after probing the system*.
  Synthesising it empty means **every `#if HAVE_FOO` block is not
  merely unanalysed, it is not compiled**. That is a different program.

Every TU that touches the shim is recorded `flagsrc="guess"` and every
finding out of it carries `guessed: true`. `--no-shim` turns it off and
raises the ERROR count instead, which is the honest reading when you do
not want findings from a program cmake never made.

---

## 4. The same measurement for hbsd/src C++

### 4a. Breakdown by subtree — "all C++ in HBSD" means four different things

Census of all 5,779 C++ files (`*.cpp *.cc *.cxx *.C`):

| n | subtree | what it is |
|---:|---|---|
| **5,010** | `contrib/llvm-project/` | vendored LLVM. 2,445 llvm, 1,008 clang, 897 lldb, 409 compiler-rt, 102 lld, 65 libcxx, 42 openmp, 38 libc, 4 libunwind |
| **356** | other `contrib/` | 233 kyua, 45 opencsd, 30 atf, 16 lutok, 9 libcxxrt, 6 netbsd-tests, 17 misc |
| **259** | **everything else — ParanoidBSD/FreeBSD's own C++** | see below |
| **106** | `contrib/googletest/` | vendored test framework |
| 38 | `crypto/` | vendored |
| 10 | `cddl/` | vendored ZFS/DTrace |

The 259 own-code files:

```
 96  lib/msun          31  lib/libc         13  usr.bin/clang
 67  tests/sys         10  usr.sbin/ctld     7  tools/regression
  6  usr.bin/dtc        6  lib/libdevdctl    4  lib/libnv
  4  lib/libutil++      3  usr.sbin/config  12  (13 dirs with 1-2 each)
```

**87% of "hbsd C++" is vendored LLVM.** Treating it as one lump with
ParanoidBSD's 259 own files would let 5,010 files of somebody else's
code set the tier's headline coverage number.

### 4b. Parse rate

| configuration | sample | PASS | rate |
|---|---:|---:|---:|
| bare `clang++ -fsyntax-only -std=c++17`, no `-I` | 50 (seed 1729) | 2 | 4% |
| + include guess, first version | 50 | 4 | 8% |
| + refined guess (`cxx_analyze.guess_flags`) | **200 (seed 7)** | **76** | **38%** |

Top remaining first errors at 38% (n=200, 124 failures, 30 distinct):

| n | message |
|---|---|
| 22 | missing header: `llvm/IR/Attributes.inc` |
| 18 | missing header: `llvm/CodeGen/GenVT.inc` |
| 11 | missing header: `clang/Basic/DiagnosticGroups.inc` |
| 11 | missing header: `clang/Basic/BuiltinTemplates.inc` |
| 10 | missing header: `clang/AST/DeclNodes.inc` |
| 6 | missing header: `sys/_types.h` |
| 4 | missing header: `clang/StaticAnalyzer/Checkers/Checkers.inc` |
| 4 | missing header: `BPFGenRegisterInfo.inc` |

Every one of the `.inc` files is **TableGen output**. It is generated
during an LLVM build and has no source-tree equivalent, which
`tools/verify/includes.py:llvm_shim()` already says in its docstring
about the C side: *"The `${OBJTOP}` entries in liblldb/Makefile are
tablegen output and have no source-tree equivalent; a file that needs
one is still an ERROR and still has to go on the record."*

The asymmetry between the two trees is the important part:

* **hbsd C++ fails on files that a build would MAKE.** Run the build,
  get the `.inc`, and coverage rises. It is work, not a wall.
* **KDE fails on headers that a package would INSTALL.** No amount of
  cleverness in the guess substitutes for Qt6. It is a wall.

`sys/_types.h` (6 of 200) is a FreeBSD header being asked for on a
Linux host — a container artefact, not a tree defect. It will not
occur on the user's FreeBSD machine.

---

## 5. Correction to the brief: cppcheck IS installed

The brief lists `cppcheck` as not present. It is: **Cppcheck 2.13.0**.

This matters more than a footnote, because cppcheck **does not need the
headers**. On `/tmp/agent_cxx/broken/nope.cpp` — a file whose first line
is `#include <QObject>` and which `clang++` cannot begin to parse —
cppcheck still reports:

```
nope.cpp:2:36: error: Memory is allocated but not initialized: p [uninitdata]
```

So cppcheck is, today, the **only** instrument in this container that
can say anything at all about the 1,386 KDE translation units clang
cannot build.

**And it must not be trusted as coverage.** cppcheck without the real
headers does not know what `Q_OBJECT` expands to, what `QString`'s
methods do, or which branches a missing `#define` removes. It will
parse approximately, skip what it cannot resolve, and *report zero for
the skipped part* — which is the exact failure mode this tier exists to
prevent, with a different tool's name on it. A cppcheck sweep over
header-less KDE is a **third** kind of evidence, weaker than
`flagsrc="guess"`, and it needs its own visibility accounting
(`--check-config` / `missingInclude` counted per TU) before any number
from it is quoted.

That driver is **NOT BUILT** and **NOT MEASURED** here. It is named as
the largest single opportunity this tier has not taken.

---

## 6. What the ledger knows about KDE: nothing

`docs/port_plan.json` holds **35,050 records**. Filtering by path
prefix:

```
sys 14951, contrib 8319, crypto 4278, lib 2827, usr.sbin 1142,
usr.bin 732, tools 641, sbin 533, stand 484, tests 346, cddl 196,
libexec 170, include 167, bin 162, share 78, kerberos5 12,
secure 6, krb5 5, gnu 1
kde  -> 0
```

`confidence.py`'s `ledger_scope()` takes its DENOMINATOR from that
file. With zero KDE records it returns `(0, 0)`, and `verdict()`
returns **`NO LEDGER ENTRIES`** for every `kde/` scope.

This is the correct answer and it should be left alone until somebody
adds the records: a scope the ledger does not know about cannot have a
visibility fraction, and inventing a denominator from `rglob` would
produce a number that looks like the C tier's and does not mean the
same thing.

Consequence for record naming, implemented in
`cxx_analyze.record_path()`:

* a file under `hbsd/src/` is named **relative to `hbsd/src`**, exactly
  as `analyze.py` names it, so `confidence.py --scope lib/msun` finds
  these records and the ledger lookup works;
* a KDE file is named **relative to the repository root**
  (`kde/kwin/src/main.cpp`), which is unambiguous and honestly scores
  `NO LEDGER ENTRIES`.

---

## 7. Summary of every headline number and its sample size

| claim | number | how |
|---|---|---|
| KDE projects that configure | 0 of 12 | census, `cmake -S/-B` each |
| `compile_commands.json` obtainable today | 0 | census |
| KDE `.cpp` clang can parse, bare | 0 of 50 | sample, seed 1729 |
| KDE `.cpp` clang can parse, best effort | **6 of 1,392** | **census** |
| KDE `.cpp` needing Qt headers | ≥ 85% | census, textual, LOWER BOUND |
| hbsd C++ files | 5,779 | census |
| ...vendored LLVM | 5,010 (87%) | census |
| ...ParanoidBSD's own | 259 (4.5%) | census |
| hbsd C++ parse rate, bare | 2 of 50 (4%) | sample, seed 1729 |
| hbsd C++ parse rate, guessed flags | 76 of 200 (38%) | sample, seed 7 |
| ledger records under `kde/` | 0 of 35,050 | census |
| KDE TUs `cxx_analyze.py` BUILT | **6 of 1,392** | census, real run, exit 2 |
| KDE sweep wall clock, all ERROR | 3m 26s | `--jobs 6`, 4 cores |
| hbsd own+googletest: OK / ERROR / TIMEOUT | 113 / 141 / 9 of 263 | census of those scopes |
| analyser findings, hand-checked | 10 of 10 | see §8 |
| ...false positives | **7 of 10 (70%)** | hand-check |
| clang-tidy distinct sites, hand-checked | 15 of 15 | see §8 |
| ...false positives | **10 of 15 (67%)** | hand-check |
| checkers confirmed FIRING on a planted defect | 11 of 25 | probe |
| checkers listed but not exercised by the probe | 14 of 25 | — |
| wanted checkers unavailable in clang 18 | 1 (`cplusplus.SelfAssignment`) | probe |


---

## 8. False-positive rate, hand-checked

**Every finding either tool produced on real code was hand-checked —
this is a census of the findings, not a sample of them.** The corpus,
however, is a severe caveat: the only C++ that built in quantity was
**vendored googletest**, which is macro- and type-erasure-heavy and
unusually hostile to both instruments. **The false-positive rate on
ParanoidBSD's own C++ is NOT MEASURED**, because too little of it
builds here.

### cxx_analyze.py — 10 findings, 10 hand-checked

| site | checker | verdict |
|---|---|---|
| gmock-matchers.h:1677 | NewDeleteLeaks | **FP** — refcounted `buffer_.shared` freed by `MatcherBase::Destroy()`; the analyser lost it through the type-erased union |
| gmock-matchers.h:1626 | NewDeleteLeaks | **FP** — same |
| sample3_unittest.cc:128 | NewDeleteLeaks | **FP** — needs `q0_` (a default-constructed fixture member) to be non-empty; the invariant is in the fixture, not the function |
| sample3_unittest.cc:128 (2nd path) | NewDeleteLeaks | **FP** — same site, second path |
| sample3_unittest.cc:134 | NewDeleteLeaks | **FP** — same |
| sample5_unittest.cc:173 | NewDeleteLeaks | **FP** — same |
| predefined_ops.h:158 | core.CallAndMessage | **FP** — inside libstdc++'s own header, not this tree |
| sample3_unittest.cc:96 | NewDeleteLeaks | **TRUE** — `new_q` leaks on the `ASSERT_EQ` early-return path, before `delete new_q` |
| gtest-unittest-api_test.cc:243 | NewDeleteLeaks | **TRUE** — `GetSortedTestSuites()` does `new const TestSuite*[]`; `ASSERT_EQ(4, ...)` in `TearDown()` returns before `delete[]` |
| sample10_unittest.cc:101 | unix.Malloc | **TRUE but DELIBERATE** — the test is named `LeaksWater` and the comment says it is meant to fail under `--check_for_leaks` |

**False positives 7 of 10 = 70%.** Real defects 2. Correct-but-intended 1.

Six of the seven false positives are ONE pattern: googletest's
`ASSERT_*` macros expand to an early `return`, combined with a fixture
member the analyser cannot constrain. That is the "invariant lives
across two functions and a struct field" class `report.py`'s `agree()`
docstring already names in C.

### cxx_tidy.py — 15 distinct sites, 15 hand-checked

| site | check | verdict |
|---|---|---|
| gtest-matchers.h:397 | bugprone-sizeof-expression | **FP** — `sizeof(M) <= sizeof(Buffer)` is the intentional inlining test |
| gtest-matchers.h:408 | bugprone-casting-through-void | **FP** — placement-new storage access, with a comment saying it is explicit to avoid aliasing UB |
| gtest-printers.h:319 | bugprone-casting-through-void | **FP** — same |
| gtest.h:1624 | misc-unconventional-assign-operator | **FP** — deliberate, commented: the streaming trick needs `void operator=() const` |
| gtest-port.h:1973 | bugprone-signed-char-misuse | **FP** — `ch == low_byte` IS the single-byte range test |
| gtest-port.h:1985 | bugprone-inc-dec-in-conditions | **FP** — `it != str.begin() && IsSpace(*--it)`; short-circuit makes it correct |
| sample3:128 ×2, :134, sample5:173 | clang-analyzer-NewDeleteLeaks | **FP** ×4 — same four as above |
| gtest-matchers.h:299 | performance-noexcept-move-constructor | **TRUE** — `MatcherBase(MatcherBase&&)` is not `noexcept`, so a `vector` reallocation copies |
| gtest-matchers.h:304 | performance-noexcept-move-constructor | **TRUE** — same for move assignment |
| sample10:50 | misc-new-delete-overloads | **TRUE** — `operator new(size_t)` with only the SIZED `operator delete` |
| sample3:96 | clang-analyzer-NewDeleteLeaks | **TRUE** — same real leak the analyser found |
| sample10:101 | clang-analyzer-unix.Malloc | **TRUE but DELIBERATE** |

**False positives 10 of 15 = 67%.** Real defects 4. Correct-but-intended 1.

### Two checks were narrowed BECAUSE of this hand-check

The first clang-tidy run produced **231 findings at 114 distinct
sites**. Hand-checking 20 of them changed the check list:

* **`cert-err33-c`: 139 of 231 findings — 60% of everything the tool
  said.** Ten of ten hand-checked were `fprintf(stderr, ...)`,
  `putc(' ', file)` and `fputs(...)` in `usr.bin/dtc`. **Zero were
  defects.** This is exactly the class `analyze.py` refuses for
  `security.insecureAPI.*`. Rather than delete the check and lose the
  part that IS a defect, its `CheckedFunctions` option was narrowed to
  the allocation, I/O-integrity, conversion and threading functions.
  Verified on a seven-call probe: the stock list flags six including
  `fprintf`/`printf`/`putc`; the narrowed list flags exactly `malloc`,
  `realloc`, `fread`. Cost, stated: an unchecked `snprintf` truncation
  is no longer reported.
* **`bugprone-assignment-in-if-condition`: dropped.** Four of four were
  the deliberate parenthesised BSD idiom `if (!(valid = input.consume(\'"\')))`,
  which is what the extra parentheses are for. Cost, stated: a genuine
  `if (x = y)` typo is no longer reported.

After both changes: **231 findings -> 87, 114 distinct sites -> 15.**
The remaining rate is the 67% above.

`bugprone-casting-through-void` was 2 of 2 false positives and was
**kept**, because two samples from a type-erasure-heavy test framework
is not enough evidence to remove a genuine defect class. That is a
decision on the record rather than a silent one.
