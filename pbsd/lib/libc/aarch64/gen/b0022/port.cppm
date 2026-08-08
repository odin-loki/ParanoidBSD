/*
 * infinity.c
 */

export module pbsd.lib.libc.aarch64.gen.b0022;

export namespace pbsd::lib_libc_aarch64_gen::b0022 {

/*
 * FreeBSD's <math.h> declares these two union types together with the two
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

/* bytes for +Infinity on aarch64 */
const union __infinity_un __infinity = { { 0, 0, 0, 0, 0, 0, 0xf0, 0x7f } };

/* bytes for NaN */
const union __nan_un __nan = { { 0, 0, 0xc0, 0xff } };

} // namespace pbsd::lib_libc_aarch64_gen::b0022
