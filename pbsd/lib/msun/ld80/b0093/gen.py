#!/usr/bin/env python3
"""Generate oracle.c and port.cppm for batch b0093 (temporary)."""

from pathlib import Path

ROOT = Path("/home/odin/pbsd/hbsd/src/lib/msun")
OUT = Path("/home/odin/pbsd/pbsd/lib/msun/ld80/b0093")

SECTIONS = [
    ("s_cexpl.c", "cexpl", "ref_cexpl", None, False),
    ("s_cospil.c", "cospil", "ref_cospil", "cospil", True),
    ("s_sinpil.c", "sinpil", "ref_sinpil", "sinpil", False),
    ("s_tanpil.c", "tanpil", "ref_tanpil", "tanpil", True),
]

HEADER_INLINE = {
    "k_cospil.h": (ROOT / "ld80/k_cospil.h").read_text(),
    "k_sinpil.h": (ROOT / "ld80/k_sinpil.h").read_text(),
    "k_expl.h": (ROOT / "ld80/k_expl.h").read_text(),
}

SKIP_INCLUDES = {
    "fpmath.h",
    "math.h",
    "math_private.h",
    "ieeefp.h",
}

SUPPORT_C = r'''/*
 * oracle.c -- batch b0093 reference implementation.
 *
 * The original HardenedBSD C sources concatenated verbatim (with ref_
 * prefixes on the four exported functions).  Support code required for
 * compilation is provided above the concatenated sources.
 */

#define _GNU_SOURCE
#include <complex.h>
#include <float.h>
#include <math.h>
#include <stdint.h>

#ifndef LONG_BIT
#define LONG_BIT (8 * sizeof(long))
#endif

union IEEEl2bits {
	long double	e;
	struct {
		unsigned int	manl	:32;
		unsigned int	manh	:32;
		unsigned int	exp	:15;
		unsigned int	sign	:1;
		unsigned int	junkl	:16;
		unsigned int	junkh	:32;
	} bits;
	struct {
		unsigned long	man	:64;
		unsigned int	expsign	:16;
		unsigned long	junk	:48;
	} xbits;
};

#define	LDBL_NBIT	0x80000000

#define	EXTRACT_LDBL80_WORDS(ix0,ix1,d)				\
do {								\
  union IEEEl2bits ew_u;					\
  ew_u.e = (d);							\
  (ix0) = ew_u.xbits.expsign;					\
  (ix1) = ew_u.xbits.man;					\
} while (0)

#define	INSERT_LDBL80_WORDS(d,ix0,ix1)				\
do {								\
  union IEEEl2bits iw_u;					\
  iw_u.xbits.expsign = (ix0);					\
  iw_u.xbits.man = (ix1);					\
  (d) = iw_u.e;							\
} while (0)

#define	SET_LDBL_EXPSIGN(d,v)					\
do {								\
  union IEEEl2bits se_u;					\
  se_u.e = (d);							\
  se_u.xbits.expsign = (v);					\
  (d) = se_u.e;							\
} while (0)

#define	GET_FLOAT_WORD(i,d)					\
do {								\
  union { float value; unsigned int word; } gf_u;			\
  gf_u.value = (d);						\
  (i) = gf_u.word;						\
} while (0)

#define	SET_FLOAT_WORD(d,i)					\
do {								\
  union { float value; unsigned int word; } sf_u;			\
  sf_u.word = (i);						\
  (d) = sf_u.value;						\
} while (0)

#define	ENTERI()
#define	RETURNI(x)	return (x)

#define	_2sumF(a, b) do {	\
	__typeof(a) __w;	\
				\
	__w = (a) + (b);	\
	(b) = ((a) - __w) + (b); \
	(a) = __w;		\
} while (0)

#define	FFLOORL80(x, j0, ix, lx) do {			\
	j0 = ix - 0x3fff + 1;				\
	if ((j0) < 32) {				\
		(lx) = ((lx) >> 32) << 32;		\
		(lx) &= ~((((lx) << 32)-1) >> (j0));	\
	} else {					\
		uint64_t _m;				\
		_m = (uint64_t)-1 >> (j0);		\
		if ((lx) & _m) (lx) &= ~_m;		\
	}						\
	INSERT_LDBL80_WORDS((x), (ix), (lx));		\
} while (0)

#ifdef __i386__
#define	LD80C(m, ex, v) {						\
	.xbits.man = __CONCAT(m, ULL),					\
	.xbits.expsign = (0x3fff + (ex)) | ((v) < 0 ? 0x8000 : 0),	\
}
#else
#define	LD80C(m, ex, v)	{ .e = (v), }
#endif

static inline long double
rnintl(long double x)
{
	return (x + 0x1.8p64L / 2 - 0x1.8p64L / 2);
}

#define	irint(x)	((int)(x))

'''

def read_kernel(path: str) -> str:
    text = (ROOT / path).read_text()
    return text.split('"math_private.h"\n', 1)[1]


KERNEL_COS = read_kernel("ld80/k_cosl.c")
KERNEL_SIN = read_kernel("ld80/k_sinl.c")
KERNEL_TAN = read_kernel("ld80/k_tanl.c")


def transform_source(text: str, ref_name: str | None, section: str | None) -> str:
    out = []
    for line in text.splitlines(keepends=True):
        stripped = line.strip()
        if stripped.startswith("#include"):
            if any(h in line for h in SKIP_INCLUDES):
                continue
            for hdr, body in HEADER_INLINE.items():
                if f'"{hdr}"' in line:
                    out.append(f"/* inlined {hdr} */\n")
                    out.append(body)
                    if not body.endswith("\n"):
                        out.append("\n")
                    break
            else:
                out.append(line)
            continue
        out.append(line)

    joined = "".join(out)
    if ref_name:
        orig = ref_name.replace("ref_", "")
        joined = joined.replace(f"\n{orig} (", f"\n{ref_name} (")
        joined = joined.replace(f"\n{orig}(", f"\n{ref_name}(")
    return joined


def section_preamble(section: str | None) -> str:
    if section is None:
        return ""
    s = section
    return (
        f"#define __kernel_cospil __kernel_cospil_{s}\n"
        f"#define __kernel_sinpil __kernel_sinpil_{s}\n"
        f"#define vzero vzero_{s}\n"
    )


def section_epilogue(section: str | None) -> str:
    if section is None:
        return ""
    return (
        f"#undef __kernel_cospil\n"
        f"#undef __kernel_sinpil\n"
        f"#undef vzero\n"
        f"#undef pi_hi\n"
        f"#undef pi_lo\n"
    )


def build_oracle() -> None:
    parts = [SUPPORT_C, "/* === kernel support === */\n", KERNEL_COS, KERNEL_SIN, KERNEL_TAN]
    for fname, _, ref, section, rename_pi in SECTIONS:
        src = (ROOT / "ld80" / fname).read_text()
        parts.append(f"\n/* ================================================================== */\n")
        parts.append(f"/* lib/msun/ld80/{fname} */\n")
        parts.append(f"/* ================================================================== */\n\n")
        parts.append(section_preamble(section, rename_pi))
        parts.append(transform_source(src, ref, section))
        parts.append(section_epilogue(section))
    (OUT / "oracle.c").write_text("".join(parts))


PORT_PREFIX = r'''// PBSD port of HardenedBSD lib/msun/ld80 -- batch b0093.
//
// Sources ported here (faithfully, bug-for-bug):
//   lib/msun/ld80/s_cexpl.c
//   lib/msun/ld80/s_cospil.c
//   lib/msun/ld80/s_sinpil.c
//   lib/msun/ld80/s_tanpil.c

module;

#define _GNU_SOURCE
#include <complex.h>
#include <cstdint>
#include <math.h>

#ifndef LONG_BIT
#define LONG_BIT (8 * sizeof(long))
#endif

export module pbsd.lib.msun.ld80.b0093;

export namespace pbsd::lib_msun_ld80::b0093 {

'''

PORT_SUFFIX = r'''
} /* export namespace pbsd::lib_msun_ld80::b0093 */
'''


def build_port() -> None:
    parts = [PORT_PREFIX, SUPPORT_C.replace("oracle.c", "port.cppm"), "/* === kernel support === */\n",
             KERNEL_COS, KERNEL_SIN, KERNEL_TAN]
    for fname, _, _, section in SECTIONS:
        src = (ROOT / "ld80" / fname).read_text()
        parts.append(f"\n/* lib/msun/ld80/{fname} */\n\n")
        parts.append(section_preamble(section))
        parts.append(transform_source(src, None, section))
        parts.append(section_epilogue(section))
    parts.append(PORT_SUFFIX)
    (OUT / "port.cppm").write_text("".join(parts))


if __name__ == "__main__":
    build_oracle()
    build_port()
