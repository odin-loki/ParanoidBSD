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
