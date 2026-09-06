# What it takes to commit a mechanical port

As of FreeBSD oracle run 13, **88 of 120** lib/msun ports are both
IR-equivalent to their C original and exporting the same symbols.

Run 16 then widened the scope and that number stopped meaning what it looked
like it meant. Read the second section first.

## What the oracle proves

`compare_ir` compiles the original as C17 and the port as C++23 with
`-fno-exceptions -fno-rtti`, normalises away the dialect artefacts (`noundef`,
`common` linkage, `mustprogress`, name mangling, SSA numbering), and compares.
`exported_symbols` then checks, on the raw IR before normalisation, that the
two modules define the same externally-visible names.

Together that is a strong statement about the *source*: same computation, same
ABI surface.

## The flags: answered

Run 16 ran the same scope with the target's own CFLAGS on both sides:

```
under target flags: files=321 edits=71 IR equal 88/120  ABI equal 88
  flags used: -ffp-exception-behavior=maytrap -fno-math-errno
              ... and -Wno-error=overflow for e_powl.c
```

Identical to the run without them. The lib/msun ports verify under the flags
that actually ship, so step 4 below is satisfied and the remaining work to
commit one is steps 1 to 3 — the rename, the `COMMON_SRCS` entry, and
`CXXFLAGS` mirroring `CFLAGS`.

## The number is not representative, and that matters more

Run 16 also widened the scope to `lib/libc` for the first time:

| scope | files | edits | edits/file | IR equal | ABI equal |
|---|---:|---:|---:|---:|---:|
| `lib/msun` | 321 | 71 | **0.22** | 88/120 (73%) | 88 |
| `lib/libc` | 1,220 | 10,786 | **8.8** | 37/120 (31%) | 34 |

Forty times the edit density, and the verification rate falls from 73% to 31%.

This is the same thing the safe-tier split found and is now confirmed on a
second scope: **lib/msun verifies because almost nothing is done to it.**
Where the passes do real work, the ports stop being provably equivalent. The
88 is a property of the corpus, not of the conversion.

So "88 of 120 verified" should not be read as "73% of the port works". Read as
a rate for the tree, the honest figure is closer to 31%, and even that is over
a scope chosen for being tractable.

`lib/libc` also shows three ports that are IR-equal and not ABI-equal (37
against 34), which is the `math_private.h` class again in a different header.

**The `lib/libc` header guards did not close it.** Oracle run 17, with
`__BEGIN_DECLS` added to `libc_private.h`, `stdio/local.h`,
`locale/mblocal.h` and `locale/xlocale_private.h`, reports the same 37 and
34 as before. Whatever the three are, they are not declared in those four.

Counting them says there is work; it does not say where. The oracle now
names them: the `lib/libc` section prints each port that is IR-equal and not
ABI-equal together with the symbols that appear on one side and not the
other. The cause is always the same — a function that is external but
declared in no header, so C++ mangles it and the library would ship it under
a new name — so the symbol name is the header's name in disguise, and the
fix is a `__BEGIN_DECLS` in whichever header should have declared it.

## What it does not prove

**The flags.** `lib/msun/Makefile` sets, in `CFLAGS`:

```make
CFLAGS+=	-ffp-exception-behavior=maytrap
CFLAGS+=	-fno-math-errno
CFLAGS+=	-I${.CURDIR}/src -I${LIBC_SRCTOP}/include ...
CFLAGS.e_powl.c+= -Wno-error=overflow
```

`bsd.lib.mk` compiles a `.cpp` with `${CXX} ${CXXFLAGS}`. None of the above
reaches it. `-fno-math-errno` alone changes what the compiler is allowed to
assume about every libm call, and `-ffp-exception-behavior=maytrap` changes
floating-point semantics — which is the entire subject matter of this library.

The oracle compiles both sides with the same flags, so its comparison is
sound; it just is not comparing against the flags that ship. A port that
verifies at 88 could still be built differently from the C it replaces.

So committing a port to `lib/msun` requires, in the same change:

1. rename `s_rint.c` to `s_rint.cpp` on disk;
2. change the `COMMON_SRCS` entry to match — the list is hand-written, and
   `ARCH_SRCS` exclusion at `Makefile:172` matches on `${i:R}.c`, so a `.cpp`
   name changes that behaviour too;
3. add `CXXFLAGS` mirroring every `CFLAGS` line, including the per-file
   `CFLAGS.<name>` overrides;
4. re-run the oracle with those flags on both sides, not the defaults.

Step 4 is the one that turns 88 from a promising number into a shippable one.
Steps 1 to 3 without it would be committing on a measurement that does not
describe the build.

## Ordering

Nothing here should land before `buildworld` is green. While the tree still
has import damage in it, a build failure after committing ports cannot be
attributed: it could be the port or it could be the next missing directory.
Once world is green, a failure means the port, which is the only condition
under which changing 88 files is a reasonable thing to do.

**World is green** — run 12, `>>> World build completed on Sat Sep 5
08:25:07 UTC 2026`, twenty-seven minutes, zero errors, with SafeStack and
CFI linking for the first time.

**The precondition is now met.** Run 13 reports `== done: world` on
`044374598`, and `git merge-base --is-ancestor 8bfe42d67 044374598` is true:
that world was built on a tree containing the msun source-list fix. The
paragraph below is what stood before run 13 and is kept because it is the
reason the order was that way round.

Run 12 built `b60d5e091`, which is before the `lib/msun/Makefile` fix in
`8bfe42d67`.
Until then `WITHOUT_MACHDEP_OPTIMIZATIONS` was inert for msun on x86:
`SRCS` held both `e_fmod.c` and `e_fmod.S`, and bmake chose between them by
suffix-rule precedence (see `docs/ASSEMBLY.md`). So the green world was
built with an untested msun source list, and landing a `.c` → `.cpp` rename
on top of it would put two unattributable changes in the same build.

The order is: a world build on a tree that contains the msun fix, then the
port. Not the other way round, for exactly the reason this section already
gives.

## Which port, and what is left

### Steps 1 to 3, as of `8e77a408e`

Step 3 is done. `lib/msun/Makefile` carries a `CXXFLAGS` block mirroring
every semantic `CFLAGS` line — `-ffp-exception-behavior=maytrap`,
`-fno-math-errno`, the `-I` set — plus `CXXSTD=c++23` and
`-fno-exceptions -fno-rtti` to match what the oracle compiles with. It
landed on its own and was built green by run 13.

A step 0 that was not in the list turned up with it. The ARCH_SRCS pairing

```make
COMMON_SRCS:=  ${COMMON_SRCS:N${i:R}.c}
...
.if !empty(COMMON_SRCS:M${i:R}.c)
```

keys on `.c`, and `e_fmod.S` and `e_fmod.cpp` are the same object name. A
bmake matrix over (option on, option off) × (entry `.c`, entry `.cpp`)
shows **both** branches break on the rename: with the option off the
filter stops recognising the collision so the `.S` is never dropped, and
with it on the exclusion stops matching so the `.c` side is never dropped.
Either way the library defines `rint` twice. Both directions now match
`.cpp` as well, which is a no-op on the all-`.c` list of today.

So steps 1 and 2 — the rename and the `COMMON_SRCS` entry — are what
remains, for a file that has yet to be named.

### The oracle could not name one

Every figure the pipeline produced was a count. "88 of 120 verified" does
not identify a file, and the first rename was going to be a guess.

Two things were wrong with the number as well as with its shape:

* `--ir-limit 120` against a 321-file scope does not mean 120 files
  failed. It means the IR budget was spent on the first 120 eligible
  files in discovery order and the other 201 were recorded
  `skipped_budget` — never compiled, never compared. **88 of 120 was 88
  of the first third of lib/msun**, and the candidate set was that third.
  The whole job takes under four minutes of a 120-minute timeout, so the
  cap was costing two thirds of the scope for nothing; lib/msun's phases
  now run at 400.
* the committable set is stricter than either floor. A port is
  committable only if it is IR-equal **and** ABI-equal **and** measured
  under the flags lib/msun is actually built with. `run_freebsd_oracle.sh`
  now prints that set by name under `committable under target flags:`.

### The 25 that are not committable, and two wrong answers about them

199 ports are IR-equal and 174 are ABI-equal. The 25 in the gap are one
family — `fmaximum`, `fminimum`, their `_mag`/`_num`/`_mag_num` variants
and the `f`/`l` forms — each computing the same thing and exporting
`_Z8fmaximumdd` instead of `fmaximum`.

Two explanations were proposed and **both were wrong, the same way**:

1. *The `__ISO_C_VISIBLE >= 2023` guard in the tree's `math.h`.* Refuted in
   one command: the macro is 2023 under `-std=c++23` exactly as under
   `-std=c17`, and the declarations are already inside `__BEGIN_DECLS`.
2. *`oracle_include_flags()` uses `-idirafter`, so the host's `math.h`
   wins.* A probe on the FreeBSD runner agreed — host header alone does
   not declare `fmaximum`, `-I lib/msun/src` does, `-idirafter` does not.
   Refuted anyway: the oracle **also** passes `-I{src.parent}`, and
   `s_fmaximum.c` sits in the same directory as `math.h`. The tree's header
   was reachable the whole time; the probe was describing a command line
   nobody runs.

Each answered a question the oracle does not ask — the recurring failure in
this project, and the second one got as far as an edit to
`oracle_include_flags()` before the contradiction turned up.

So the diagnostic no longer probes an imagined command line.
`tools/pbsd_passes/why_mangled.py` runs the oracle's own
`oracle_include_flags()` and target flags over the real source and its real
staged port, and prints the symbol tables each side actually defines plus
whether `<math.h>` under those same flags declares the name.

### The answer: `-I` cancelled by `-idirafter` for the same directory

On the FreeBSD runner, with the oracle's real flags:

```
== hbsd/src/lib/msun/src/s_fminimum.c
   -> only in C  : fminimum
   -> only in C++: _Z8fminimumdd
   <math.h> under these flags: fminimum is NOT declared

== hbsd/src/lib/msun/src/s_cbrt.c
   C defines: cbrt     C++ defines: cbrt     -> same symbols
```

The declaration is missing even though `-I{src.parent}` points at the
directory `math.h` lives in. clang's own search list says why:

```
$ clang++ -E -v -I<msun/src> -idirafter<msun/src>
  #include <...> search starts here:
   2: .../include/c++/13
   3: .../include/x86_64-linux-gnu/c++/13
   9: .../hbsd/src/lib/msun/src
```

`msun/src` appears **once**, at 9, behind the C++ standard library. clang
deduplicates the search list and keeps the *later* entry, so naming a
directory on both `-I` and `-idirafter` does not add it twice — it **moves
it** from before the system headers to after them.

`oracle_include_flags()` did exactly that: `flags` opens with
`-I{src.parent}`, and `extras` ends with `msun/src` on `-idirafter`. For
every source in `lib/msun/src` — most of the library — those are the same
directory, so `#include <math.h>` stopped resolving to the tree's header.
The `fmaximum`/`fminimum` family is declared only in that newer header, so
it was declared on **neither** side; C emits a plain symbol and C++ mangles
the identical definition. That is the 25.

clang warns `duplicate directory ... is ignored` when this happens.
`-Wno-everything`, the first flag in the list, suppressed it.

Fixed by not repeating a directory already on `-I`.

### Run 27, after the fix

```
msun ratchet:  211 verified + 1 committed = 212  (was 199)
msun ABI:      210 ABI-equal + 1 committed = 211  (was 174)
under target flags: IR 205/284  ABI 204  committable 204  (was 173)
```

The gap this whole section is about has collapsed: 211 IR-equal against
210 ABI-equal, where it was 25. `why_mangled.py` now reports `C++ defines:
fminimum` and `-> same symbols` for the file that produced
`_Z8fminimumdd` — the fix confirmed on the file that showed the fault.

Floors raised to the measured 212 and 211.

**204 ports are committable** — IR-equal, ABI-equal, and measured under
`-ffp-exception-behavior=maytrap -fno-math-errno`. That is the set to take
the next port from, and it is 31 larger than it looked an hour ago for no
reason but a duplicated include path.

Take the file from that list. Anything else is a guess with a
forty-minute feedback loop.

## The first port: `lib/msun/src/k_cos.c` -> `k_cos.cpp`

Chosen from the oracle's committable list against four criteria, in order:

1. **Zero edits.** The pass pipeline changed nothing, so the staged `.cpp`
   is byte-for-byte the original and `git` records the change as a pure
   rename — `1 file changed, 0 insertions(+), 0 deletions(-)`. If the build
   breaks it is the build system, not a transformation.
2. **No assembly counterpart anywhere.** `k_cos` appears in no `ARCH_SRCS`
   on any of the six architectures, so this port is orthogonal to the
   `.c`/`.cpp` pairing fix landed just before it and the two cannot be
   confused for one another.
3. **Explicit C linkage.** `__kernel_cos(double,double)` is declared inside
   the `__BEGIN_DECLS` block in `math_private.h`, so C++ does not mangle it
   and libm ships the same symbol. This is the guard whose absence had six
   ld80/ld128 ports verified and uncommittable.
4. **Unconditional `COMMON_SRCS`.** It is built on all six architectures,
   so the matrix exercises it rather than one target.

### A committed port leaves the candidate set

`discover_sources()` globs `*.c`. The moment `k_cos.c` becomes `k_cos.cpp`
it is no longer discovered, no longer compared, and `ir_equal` drops by one
— so the ratchet would have read the first successful port in this
project's history as *"a port that was proved IR-equivalent no longer is"*
and failed the job.

The floors are now checked against `verified + committed`. A committed port
is the stronger statement of the two: it was verified **and** it built. A
genuine regression — a file that is still a `.c` and stopped verifying —
lands below the floor and still fails, which is the whole point of the
ratchet. Verified by running the block against the Linux report, where
nothing verifies: `FAIL 0 ports verify and 1 are committed, 1 against a
floor of 88`.

### It builds

Run 18, `stage=world` on `9670b66bd`:

```
>>> Kernel(s)  HARDENEDBSD built in 780 seconds, ncpu: 4, make -j4
== done: world
```

**The first file in PBSD compiled as C++ builds and links.** libm is built
with `k_cos.cpp` in `COMMON_SRCS`, at `-std=c++23` with
`-ffp-exception-behavior=maytrap -fno-math-errno -fno-exceptions -fno-rtti`.

A green world is not by itself proof that the port was *built*, though. The
port is a rename plus one line of a hand-written `SRCS` list, and getting
that line wrong drops the file out of `SRCS` silently rather than failing —
libm would be built without it and stay green until something called the
missing symbol. `build_boot_image.sh` therefore asserts after `buildworld`
that every `.cpp` under `lib` has its object in the objdir, and names the
file if it does not. Tested both ways.

### The oracle numbers across the port

| run | commit | verified | ABI-equal | committable | committed |
|---|---|---:|---:|---:|---:|
| 21 | `0c7f5735a` | 199 | 174 | 174 | 0 |
| 23 | `8c147e8c7` | 198 + 1 | 173 + 1 | 173 | 1 |
| 27 | `7f5782656` | 211 + 1 | 210 + 1 | 204 | 1 |

Run 23 is the load-bearing one. With the floors at 199 and 174 and the port
committed, `verified` alone is 198 and `abi_equal` alone is 173 — both one
below their floor. The `+ committed` term is what makes 199 and 174, and
without it landing a port would have read as losing one.

## The hundred-file batch, and a fifth thing a port must not break

Run 28 built the world on the hundred-file batch and stopped nine minutes
in:

```
lib/msun/src/s_cos.cpp:48:10: fatal error: 'e_rem_pio2.c' file not found
   48 | #include "e_rem_pio2.c"
```

`lib/msun` compiles some sources twice — once on their own, and once
textually, because a second source inlines them by name. There are
twenty-eight such `#include`s in the library. **A file on the receiving
end of one is not an independent translation unit**: its name is part of
another file's source text, so its extension is not free.

Five of the hundred were such files:

| file | included by |
|---|---|
| `e_rem_pio2.c` | `s_cos`, `s_sin`, `s_tan`, `s_sincos` |
| `e_rem_pio2f.c` | `s_cosf`, `s_sinf`, `s_tanf`, `s_sincosf` |
| `k_cosf.c` | `s_cosf`, `s_sinf`, `s_sinpif`, `s_cospif` |
| `k_sinf.c` | `s_cosf`, `s_sinf`, `s_sinpif`, `s_cospif` |
| `k_tanf.c` | `s_tanf`, `s_tanpif` |

Reverted rather than rewritten. Editing the `#include` lines to `.cpp`
would make `s_sinpif.c`, `s_cospif.c` and `s_tanpif.c` — still C —
include a `.cpp`; the cluster should move in one commit when it moves.

**None of the checks that were run could see this.** The renamed target
still exists, `SRCS` still resolves, no two objects collide, and a
zero-edit port is byte-identical to the C. The twelve-combination
`bmake` matrix was the right check for the failure mode it was aimed at
and blind to this one. Worse, the signal was in the commit message: it
recorded that two test harnesses "read msun sources by hardcoded `.c`
name" and fixed those, without asking *why* a source is ever named by a
hardcoded string — which is that something includes it.

So step 3 above gains a clause, and there is now a gate for it:

3. …and check that nothing `#include`s the file by its old name.
   `tools/check_source_includes.py --gate` fails on an `#include "X.c"`
   where `X.c` is gone and `X.cpp` is there instead. That is a port and
   nothing else, `.c` -> `.cpp` being the only rename this project
   performs, so the rule has no judgement in it. It also lists, without
   failing, the twenty-three source `#include`s that do not resolve in
   the tree as imported — `contrib/netbsd-tests` reaching for NetBSD
   kernel sources, `crypto/krb5`'s `ev.c` naming four back ends,
   `lib/csu`'s `ignore_init.c` — so a port breaking one of those would
   be visible without gating on vendor code.

Run against the tree at the broken commit it reports seventeen findings
naming all five files and every includer, and exits 1; against the
reverted tree it exits 0. Ninety-five of the hundred stand.


## lib/libc, measured whole, and one header worth twenty-five ports

`lib/libc` had been reported at `--ir-limit 120` against 1,220 files
since the scope was added, which made "37 of 120" a statement about the
first tenth of the library in discovery order. At 1500 it is **123 of
574**, and the committable list is long enough to work from.

Widening it also changed what the gap is made of. Of the 38 ports that
were IR-equal and not ABI-equal, **25 were one directory**:

```
quad/adddi3.c   only in C: __adddi3   only in C++: _Z8__adddi3ll
quad/anddi3.c   only in C: __anddi3   only in C++: _Z8__anddi3ll
... 23 more, every one the same shape
```

`lib/libc/quad/quad.h` declared seven of the twenty-five functions,
outside any linkage guard, and the other eighteen were declared nowhere.
Both halves give the same result under C++ — a definition with no C
prototype gets C++ linkage — so libc would ship `__adddi3` under a name
no caller and no compiler-generated reference can find. The same shape as
`lib/msun/src/math_private.h` in run 13, where one missing
`__BEGIN_DECLS` was worth fourteen.

| run | commit | libc IR | libc ABI | committable |
|---|---|---:|---:|---:|
| 30 | `4ec39cefc` | 123/574 | 85 | 85 |
| 31 | `5a0a1136f` | 123/574 | 110 | 110 |

**+25, exactly the size of the directory.** Two prototypes are not what a
reader would guess and are as the `.c` files have them: `__fixsfdi`
returns `long long` rather than `quad_t`, and `notdi2.c` defines
`__one_cmpldi2` rather than `__notdi2`.

Checked before the run rather than after: every `.c` in the directory
compiled for the oracle's own target as C and as C++, 26 files, both
languages, clean — that is the check for prototype/definition agreement,
and it is the entire risk of adding eighteen prototypes to a header.
Then the symbols from `clang++` on the actual sources, `_Z8__adddi3xx`
becoming `__adddi3` and five more. The oracle then agreed to the port.

The thirteen that remain are each their own header: `arm/gen/fabs.c`,
`gen/dirfd.c`, `gen/dup3.c`, `nameser/ns_netint.c`, powerpc64's `strcpy`
and `strncpy`, `rpc/des_soft.c`, `rpc/rpcsec_gss_stub.c`, `stdlib`'s
`hdestroy_r`, `insque` and `remque`, `string/memset_explicit.c` and
`sys/pipe.c`.


## The screening, as a tool rather than by hand

Every property the hundred-file batch needed was found by hand, and the
one it missed cost a fifty-minute build. `tools/check_port_candidates.py`
applies all of them at once:

```
$ python3 tools/check_port_candidates.py --scope lib/libc --show-rejected
```

| reason | what it means |
|---|---|
| `included by name elsewhere` | something `#include`s it. Renaming it breaks every includer — this is run 28. |
| `no Makefile in the scope names it` | not built here; there is nothing to port. |
| `its SRCS line is inside an .if/.for` | built for some configurations only, so a failure would be about which one ran. |
| `same basename as …` | a `.PATH` may prefer the other copy; renaming one changes which bmake picks. |
| `named by <Makefile>` | a consumer outside the scope, with its own `SRCS` and its own flags — `lib/libgcc_s` is the case the msun batch hit. |

With `--report` it also requires `ir.equal`, `ir.abi_equal` and
`edits == 0` from the oracle's `pass_report.json`. Without it the tool says
so: those three are the oracle's half and it only runs on FreeBSD.

Run against the current tree it reproduces what this session paid for.
All five files that stopped run 28 — `e_rem_pio2.c`, `e_rem_pio2f.c`,
`k_cosf.c`, `k_sinf.c`, `k_tanf.c` — come back "included by name
elsewhere", and the `lib/libgcc_s` twelve come back naming that Makefile.
It also found two things the manual pass did not: `s_scalbn.c` is
`#include`d as well as named by `lib/libgcc_s`, and **`lib/libc/gen/assert.c`
is named by four Makefiles outside libc** — `cddl/lib/libspl`,
`lib/libfido2`, `lib/libsm` and `stand/libsa`. That last one was on the
shortlist for libc's first port.

`lib/msun`: 96 of 225 `.c` files pass the tree-side checks.
`lib/libc`: 592 of 1216.


## lib/libc has floors now, and run 33 is why it could

A floor on `ir_equal` alone would have failed the first time a port
succeeded: a landed port stops being a `.c`, `discover_sources()` stops
finding it, and the count drops by one. `lib/msun`'s ratchet solves that
by counting `verified + committed`, and `lib/libc` got the same term
before it got a floor. Oracle run 33 is the proof rather than the
argument:

| run | | verified | committed | total |
|---|---|---:|---:|---:|
| 32 | before `gen/isatty.cpp` | 123 | 0 | 123 |
| 33 | after | 122 | 1 | 123 |

The sum held across the port. So:

```
docs/migration/freebsd_libc_verified_floor.txt   123
docs/migration/freebsd_libc_abi_floor.txt        110
```

Both are checked after everything else has printed, for the reason
`lib/msun`'s are: the run that fails a floor is exactly the run whose
numbers someone wants to read.

Exercised against four synthetic reports before it went near the runner —
at the floor, one below the IR floor, one below the ABI floor, and above
both. The first three of those found nothing; the fourth check found the
bug. `sys.exit(1)` had gone into a block whose `import sys` I had added to
**a different `PY2` heredoc** — there are three in that script — so the
gate would have raised `NameError` the first time it fired, on the
runner, six minutes into a job, on a run that was already failing. A
regex checking "is `sys` imported" said yes because it was reading the
wrong heredoc.


## The first lib/libc batch: 27 files

Derived rather than chosen. `check_port_candidates.py --scope lib/libc`
gives what the tree permits (577 of 1,216); oracle run 33's
`committable: 109, of which 85 are zero-edit` gives what the oracle
verified; the batch is the intersection, and it is 27.

```
compat-43/setrgid.cpp  setruid.cpp
gen/_rand48.cpp  drand48.cpp  erand48.cpp  frexp.cpp  isinf.cpp
gen/isnan.cpp  jrand48.cpp  lcong48.cpp  lrand48.cpp  mrand48.cpp
gen/nrand48.cpp  siglist.cpp  srand48.cpp
locale/iswctype.cpp
secure/strlcat_chk.cpp
stdio/dprintf.cpp  fcloseall.cpp
stdlib/imaxabs.cpp  imaxdiv.cpp  labs.cpp  ldiv.cpp  llabs.cpp  lldiv.cpp
string/ffsll.cpp
uuid/uuid_hash.cpp
```

Every one is a pure rename: `27 files changed, 0 insertions(+), 0
deletions(-)`, plus one `SRCS` token each.

### The 12 the intersection removed, and why the tool had to learn one

39 files survived the first intersection. Twelve of them should not have,
and eleven were one Makefile:

| file | named by |
|---|---|
| `quad/`: `adddi3` `anddi3` `cmpdi2` `iordi3` `lshldi3` `muldi3` `negdi2` `notdi2` `subdi3` `xordi3` | `KQSRCS`, `lib/libc/Makefile:171` |
| `string/ffsl.c` | `KSRCS`, same |
| `stdlib/div.c` | `JEMALLOCSRCS` (a basename collision — jemalloc's is `contrib/jemalloc/src/div.c`) |

`KQSRCS` and `KSRCS` feed `make libkern`, a hand-run target that copies
those sources into `sys/libkern`. Nothing adds either to `SRCS`, so they
are not build lists — but they match `*SRCS*=`, and being at the top level
of the file they read as **unconditional** `SRCS` entries. That then
masked the real ones: every `SRCS` line in `quad/Makefile.inc` is inside
an `.if` on `LIBC_ARCH`, and the tool's "unconditional wins" rule
discarded the conditional finding. Ten quad files were portable on the
strength of a list that builds nothing.

The rule now is reachability: a `SRCS`-shaped variable counts only if it
reaches `SRCS` by `${VAR}` reference. See the `unbuilt` row in the table
above.

### Verified before the build, not by it

* `check_source_includes.py --gate` — nothing `#include`s a renamed name.
* `check_libc_srcs.py --gate` — all 1,051 sources resolve on disk.
* `bmake -V SRCS` over **six architectures × two configurations**
  (defaults, and `SRCCONF=src.conf.pbsd` where
  `WITHOUT_MACHDEP_OPTIMIZATIONS` takes the other branch of
  `Makefile:159`): 28 `.cpp` on every one of the twelve, no ported name
  still present as `.c`, no `.cpp` missing, and no two sources sharing an
  object stem.
* the `SRCS` **set**, with extensions normalised, is byte-identical to the
  set before the batch — 1,425 entries on amd64 under `src.conf.pbsd`. The
  batch changed extensions, not membership.

`LIBC_NONSHARED_SRCS` is worth knowing about before the next batch:
`Makefile:152` derives its objects with `${LIBC_NONSHARED_SRCS:S/.c$/.o/}`,
which is keyed on `.c` and would silently produce nothing for a `.cpp`. It
resolves to the eleven `iconv` sources, none of which is in this batch,
and all of which the screener rejects anyway.


## What certifies a port, after runs 35 and 38

Two files in the 27-file batch had `ir.equal`, `abi_equal` and
`edits == 0`, passed the tree-side screener, and did not build. Each
exposed something the oracle structurally cannot see.

| run | file | what stopped it | why certification missed it |
|---|---|---|---|
| 35 | `gen/siglist.cpp` | `error: array designators are a C99 extension` | the oracle passes `-Wno-everything`; the build is `-Werror` |
| 38 | `gen/isnan.cpp` | `ld.lld: version script assignment of 'FBSD_1.0' to symbol '__isnan' failed` | the whole file is inside `#ifdef PIC`; the oracle passes no `-DPIC`, so it **compared two empty modules** |

The second is the sharper lesson. Nothing was wrong with the oracle's
comparison — it compared what it compiled, and it compiled nothing. A
vacuous check reports a pass unless it is built not to.

So a port now has to survive four things, not two:

| | |
|---|---|
| `check_port_candidates.py` | the tree permits the rename (6 rules) |
| the oracle | `ir.equal`, `abi_equal`, `edits == 0` |
| `check_port_cxx_warnings.py` | **no diagnostic under C++23 that C17 does not also raise** — a differential, so the four `lib/msun` files whose C originals already warn are not called regressions |
| `check_port_symbols.py` | **the same exported symbols**, compiled with the build's `-D` set (`-DPIC` first among them), and **an empty module is a failure, not a pass** |

Both new tools take `--candidates`, which applies the identical comparison
to `.c` files that have not been ported. That turns them from a regression
check on the last batch into a pre-flight for the next one.

Run over the 60 still-unported zero-edit committable candidates:

```
  MANGLES  lib/libc/gen/isnan.c    only in C++: _Z7__isnand, _Z8__isnanff
  58 clean, 1 would mangle, 0 vacuous

  lib/libc/gen/siglist.c   new under C++: -Wc99-designator
  60 candidates, 1 introduces a C++-only diagnostic
```

Each pre-flight names exactly the file that broke its run, and nothing
else. **58 of the 60 are clean under all four.**

`errlst.c` is the control worth keeping in mind: it is also `#ifdef PIC`
and it is in the batch, and with `-DPIC` it compares clean, because
`errlst.h` declares `sys_errlist` inside `__BEGIN_DECLS`. The guard is not
the rule — emitting nothing is.
