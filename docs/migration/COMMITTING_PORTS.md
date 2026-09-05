# What it takes to commit a mechanical port

As of FreeBSD oracle run 13, **88 of 120** lib/msun ports are both
IR-equivalent to their C original and exporting the same symbols. That is the
committable set by the oracle's reckoning. It is not yet the shippable set,
and this is the gap.

## What the oracle proves

`compare_ir` compiles the original as C17 and the port as C++23 with
`-fno-exceptions -fno-rtti`, normalises away the dialect artefacts (`noundef`,
`common` linkage, `mustprogress`, name mangling, SSA numbering), and compares.
`exported_symbols` then checks, on the raw IR before normalisation, that the
two modules define the same externally-visible names.

Together that is a strong statement about the *source*: same computation, same
ABI surface.

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
