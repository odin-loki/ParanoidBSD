/* $NetBSD: lttf2.c,v 1.1 2011/01/17 10:08:35 matt Exp $ */
/* $NetBSD: gexf2.c,v 1.2 2004/09/27 10:16:24 he Exp $ */
/* $NetBSD: getf2.c,v 1.1 2011/01/17 10:08:35 matt Exp $ */
/* $NetBSD: letf2.c,v 1.1 2011/01/17 10:08:35 matt Exp $ */

/*
 * Written by Matt Thomas, 2011.  This file is in the Public Domain.
 *	(lttf2.c, getf2.c, letf2.c)
 * Written by Ben Harris, 2000.  This file is in the Public Domain.
 *	(gexf2.c)
 */

/*
 * PBSD C++23 port of lib/libc/softfloat/{lttf2,gexf2,getf2,letf2}.c.
 *
 * softfloat-for-gcc.h, milieu.h and softfloat.h are not part of this batch:
 * the types below mirror their declarations and the three comparison
 * primitives are declared with C language linkage so that this module binds
 * to the same softfloat.c definitions the original objects bound to.
 */

module;

#define FLOATX80
#define FLOAT128

#include <cstdint>

export module pbsd.lib.libc.softfloat.b0016;

namespace pbsd::lib_libc_softfloat::b0016 {

/* milieu.h */
export using flag = int;
export using bits16 = std::uint16_t;
export using bits32 = std::uint32_t;
export using bits64 = std::uint64_t;

/* softfloat.h */
#ifdef FLOATX80
export struct floatx80 {
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	bits16 high;
	bits64 low;
#else
	bits64 low;
	bits16 high;
#endif
};
#endif /* FLOATX80 */

#ifdef FLOAT128
export struct float128 {
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	bits64 high, low;
#else
	bits64 low, high;
#endif
};
#endif /* FLOAT128 */

extern "C" {
#ifdef FLOAT128
flag float128_lt(float128, float128);
flag float128_le(float128, float128);
#endif
#ifdef FLOATX80
flag floatx80_le(floatx80, floatx80);
#endif
}

#ifdef FLOAT128

export flag __lttf2(float128, float128);

flag
__lttf2(float128 a, float128 b)
{

	/* libgcc1.c says -(a < b) */
	return -float128_lt(a, b);
}

#endif /* FLOAT128 */

#ifdef FLOATX80

export flag __gexf2(floatx80, floatx80);

flag
__gexf2(floatx80 a, floatx80 b)
{

	/* libgcc1.c says (a >= b) - 1 */
	return floatx80_le(b, a) - 1;
}
#endif /* FLOATX80 */

#ifdef FLOAT128

export flag __getf2(float128, float128);

flag
__getf2(float128 a, float128 b)
{

	/* libgcc1.c says (a >= b) - 1 */
	return float128_le(b, a) - 1;
}

#endif /* FLOAT128 */

#ifdef FLOAT128

export flag __letf2(float128, float128);

flag
__letf2(float128 a, float128 b)
{

	/* libgcc1.c says 1 - (a <= b) */
	return 1 - float128_le(a, b);
}

#endif /* FLOAT128 */

} /* namespace pbsd::lib_libc_softfloat::b0016 */
