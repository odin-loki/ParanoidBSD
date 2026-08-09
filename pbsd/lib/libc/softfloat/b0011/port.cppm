/*
 * PBSD migration batch b0011 -- C++23 port of HardenedBSD
 * lib/libc/softfloat/{eqdf2,negdf2,gesf2,lesf2}.c
 *
 * The softfloat comparison primitives float64_eq() and float32_le() are not
 * part of this batch; as in the original C sources they are resolved at link
 * time against the softfloat runtime.
 */

module;

#include <cstdint>

export module pbsd.lib.libc.softfloat.b0011;

extern "C" {
char float32_le(std::uint32_t, std::uint32_t);
char float64_eq(std::uint64_t, std::uint64_t);
}

export namespace pbsd::lib_libc_softfloat::b0011 {

/* softfloat types (milieu.h / softfloat.h, bits64 configuration) */
using flag = char;
using bits32 = std::uint32_t;
using bits64 = std::uint64_t;
using float32 = bits32;
using float64 = bits64;

/*
 * In the bits64 configuration float64 already is a 64-bit integer, so
 * mangling between bit pattern and float64 is the identity.
 */
constexpr float64
FLOAT64_MANGLE(std::uint64_t a) noexcept
{

	return a;
}

/* $NetBSD: eqdf2.c,v 1.1 2000/06/06 08:15:02 bjh21 Exp $ */

/*
 * Written by Ben Harris, 2000.  This file is in the Public Domain.
 */

flag __eqdf2(float64, float64);

flag
__eqdf2(float64 a, float64 b)
{

	/* libgcc1.c says !(a == b) */
	return !float64_eq(a, b);
}

/* $NetBSD: negdf2.c,v 1.1 2000/06/06 08:15:07 bjh21 Exp $ */

/*
 * Written by Ben Harris, 2000.  This file is in the Public Domain.
 */

float64 __negdf2(float64);

float64
__negdf2(float64 a)
{

	/* libgcc1.c says -a */
	return a ^ FLOAT64_MANGLE(0x8000000000000000ULL);
}

/* $NetBSD: gesf2.c,v 1.1 2000/06/06 08:15:05 bjh21 Exp $ */

/*
 * Written by Ben Harris, 2000.  This file is in the Public Domain.
 */

flag __gesf2(float32, float32);

flag
__gesf2(float32 a, float32 b)
{

	/* libgcc1.c says (a >= b) - 1 */
	return float32_le(b, a) - 1;
}

/* $NetBSD: lesf2.c,v 1.1 2000/06/06 08:15:06 bjh21 Exp $ */

/*
 * Written by Ben Harris, 2000.  This file is in the Public Domain.
 */

flag __lesf2(float32, float32);

flag
__lesf2(float32 a, float32 b)
{

	/* libgcc1.c says 1 - (a <= b) */
	return 1 - float32_le(a, b);
}

} /* namespace pbsd::lib_libc_softfloat::b0011 */
