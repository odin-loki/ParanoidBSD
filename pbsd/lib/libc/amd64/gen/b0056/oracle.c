/*
 * oracle.c -- reference specification for PBSD batch b0056.
 *
 * The four HardenedBSD sources are concatenated below.  Every function is
 * renamed with a ref_ prefix; bodies are otherwise unmodified.  The host
 * <math.h> does not declare union __infinity_un / union __nan_un, so those
 * types are reproduced here from FreeBSD's <math.h> before the infinity.c
 * section.  <ieeefp.h> is supplied via -I on the compiler command line.
 */

/* --- fpsetround.c --- */
#define __IEEEFP_NOINLINES__ 1
#include <ieeefp.h>

fp_rnd_t ref_fpsetround(fp_rnd_t m)
{
	return (__fpsetround(m));
}

/* --- fpsetprec.c --- */
#define __IEEEFP_NOINLINES__ 1
#include <ieeefp.h>

fp_prec_t ref_fpsetprec(fp_prec_t m)
{
	return (__fpsetprec(m));
}

/* --- fpsetmask.c --- */
#define __IEEEFP_NOINLINES__ 1
#include <ieeefp.h>

fp_except_t ref_fpsetmask(fp_except_t m)
{
	return (__fpsetmask(m));
}

/* --- infinity.c --- */
/*
 * infinity.c
 */

#include <math.h>

/*
 * FreeBSD's <math.h> declares these union types; this host's does not.
 */
union __infinity_un {
	unsigned char	__uc[8];
	double		__ud;
};

union __nan_un {
	unsigned char	__uc[sizeof(float)];
	float		__uf;
};

/* bytes for +Infinity on a 387 */
const union __infinity_un ref___infinity = { { 0, 0, 0, 0, 0, 0, 0xf0, 0x7f } };

/* bytes for NaN */
const union __nan_un ref___nan = { { 0, 0, 0xc0, 0xff } };
