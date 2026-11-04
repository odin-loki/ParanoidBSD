/* ---- nexf2.c ---- */
/* $NetBSD: nexf2.c,v 1.2 2004/09/27 10:16:24 he Exp $ */

/*
 * Written by Ben Harris, 2000.  This file is in the Public Domain.
 */

#include "softfloat-for-gcc.h"
#include "milieu.h"
#include "softfloat.h"

#ifdef FLOATX80

flag ref___nexf2(floatx80, floatx80);

flag
ref___nexf2(floatx80 a, floatx80 b)
{

	/* libgcc1.c says a != b */
	return !floatx80_eq(a, b);
}
#endif /* FLOATX80 */

/* ---- gttf2.c ---- */
/* $NetBSD: gttf2.c,v 1.1 2011/01/17 10:08:35 matt Exp $ */

/*
 * Written by Matt Thomas, 2011.  This file is in the Public Domain.
 */

#include "softfloat-for-gcc.h"
#include "milieu.h"
#include "softfloat.h"

#ifdef FLOAT128

flag ref___gttf2(float128, float128);

flag
ref___gttf2(float128 a, float128 b)
{

	/* libgcc1.c says a > b */
	return float128_lt(b, a);
}

#endif /* FLOAT128 */

/* ---- netf2.c ---- */
/* $NetBSD: netf2.c,v 1.1 2011/01/17 10:08:35 matt Exp $ */

/*
 * Written by Matt Thomas, 2011.  This file is in the Public Domain.
 */

#include "softfloat-for-gcc.h"
#include "milieu.h"
#include "softfloat.h"

#ifdef FLOAT128

flag ref___netf2(float128, float128);

flag
ref___netf2(float128 a, float128 b)
{

	/* libgcc1.c says a != b */
	return !float128_eq(a, b);
}

#endif /* FLOAT128 */

/* ---- eqtf2.c ---- */
/* $NetBSD: eqtf2.c,v 1.1 2011/01/17 10:08:35 matt Exp $ */

/*
 * Written by Matt Thomas, 2011.  This file is in the Public Domain.
 */

#include "softfloat-for-gcc.h"
#include "milieu.h"
#include "softfloat.h"

#ifdef FLOAT128
flag ref___eqtf2(float128, float128);

flag
ref___eqtf2(float128 a, float128 b)
{

	/* libgcc1.c says !(a == b) */
	return !float128_eq(a, b);
}
#endif /* FLOAT128 */

