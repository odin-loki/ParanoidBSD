module;

#define __IEEEFP_NOINLINES__ 1
#include <ieeefp.h>

export module pbsd.lib.libc.amd64.gen.b0056;

export namespace pbsd::lib_libc_amd64_gen::b0056 {

/*
 * infinity.c
 */

/*
 * FreeBSD's <math.h> declares these union types together with the two
 * objects defined below.  The host C++ standard library does not provide
 * them, so the declarations are reproduced here unchanged so that the
 * objects can be defined with their original types.
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
extern const union __infinity_un __infinity;

/* bytes for NaN */
extern const union __nan_un __nan;

fp_rnd_t fpsetround(fp_rnd_t m);
fp_prec_t fpsetprec(fp_prec_t m);
fp_except_t fpsetmask(fp_except_t m);

} // namespace pbsd::lib_libc_amd64_gen::b0056

namespace pbsd::lib_libc_amd64_gen::b0056 {

fp_rnd_t
fpsetround(fp_rnd_t m)
{
	return (__fpsetround(m));
}

fp_prec_t
fpsetprec(fp_prec_t m)
{
	return (__fpsetprec(m));
}

fp_except_t
fpsetmask(fp_except_t m)
{
	return (__fpsetmask(m));
}

extern const union __infinity_un __infinity = { { 0, 0, 0, 0, 0, 0, 0xf0, 0x7f } };

extern const union __nan_un __nan = { { 0, 0, 0xc0, 0xff } };

} // namespace pbsd::lib_libc_amd64_gen::b0056
