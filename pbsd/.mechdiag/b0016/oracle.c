/* ---- lttf2.c ---- */
/* $NetBSD: lttf2.c,v 1.1 2011/01/17 10:08:35 matt Exp $ */

/*
 * Written by Matt Thomas, 2011.  This file is in the Public Domain.
 */

#include "softfloat-for-gcc.h"
#include "milieu.h"
#include "softfloat.h"

#ifdef FLOAT128

flag ref___lttf2(float128, float128);

flag
ref___lttf2(float128 a, float128 b)
{

	/* libgcc1.c says -(a < b) */
	return -float128_lt(a, b);
}

#endif /* FLOAT128 */

/* ---- gexf2.c ---- */
/* $NetBSD: gexf2.c,v 1.2 2004/09/27 10:16:24 he Exp $ */

/*
 * Written by Ben Harris, 2000.  This file is in the Public Domain.
 */

#include "softfloat-for-gcc.h"
#include "milieu.h"
#include "softfloat.h"

#ifdef FLOATX80

flag ref___gexf2(floatx80, floatx80);

flag
ref___gexf2(floatx80 a, floatx80 b)
{

	/* libgcc1.c says (a >= b) - 1 */
	return floatx80_le(b, a) - 1;
}
#endif /* FLOATX80 */

/* ---- getf2.c ---- */
/* $NetBSD: getf2.c,v 1.1 2011/01/17 10:08:35 matt Exp $ */

/*
 * Written by Matt Thomas, 2011.  This file is in the Public Domain.
 */

#include "softfloat-for-gcc.h"
#include "milieu.h"
#include "softfloat.h"

#ifdef FLOAT128

flag ref___getf2(float128, float128);

flag
ref___getf2(float128 a, float128 b)
{

	/* libgcc1.c says (a >= b) - 1 */
	return float128_le(b, a) - 1;
}

#endif /* FLOAT128 */

/* ---- letf2.c ---- */
/* $NetBSD: letf2.c,v 1.1 2011/01/17 10:08:35 matt Exp $ */

/*
 * Written by Matt Thomas, 2011.  This file is in the Public Domain.
 */

#include "softfloat-for-gcc.h"
#include "milieu.h"
#include "softfloat.h"

#ifdef FLOAT128

flag ref___letf2(float128, float128);

flag
ref___letf2(float128 a, float128 b)
{

	/* libgcc1.c says 1 - (a <= b) */
	return 1 - float128_le(a, b);
}

#endif /* FLOAT128 */

