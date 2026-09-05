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

Fixed by not repeating a directory already on `-I`. Both floors may move on
the next run, and can only move up: the previous numbers were measured
against a header the shipping build never uses. Neither floor is raised
here on that prediction — the ratchet still fails on a drop, which is what
it is for.

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

Run 23 is the load-bearing one. With the floors at 199 and 174 and the port
committed, `verified` alone is 198 and `abi_equal` alone is 173 — both one
below their floor. The `+ committed` term is what makes 199 and 174, and
without it landing a port would have read as losing one.
