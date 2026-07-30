/*
 * oracle.c -- reference implementation for PBSD batch b0022.
 *
 * The original HardenedBSD C sources of this batch, concatenated, with every
 * externally visible name given a "ref_" prefix.  Initialisers and function
 * bodies are unmodified.
 */

/*
 * infinity.c
 */

#include <math.h>

/*
 * FreeBSD's <math.h> declares "union __infinity_un" and "union __nan_un";
 * this host's <math.h> does not, so the missing type definitions are added
 * here.  They are copied from FreeBSD's <math.h> verbatim.
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
const union __infinity_un ref___infinity = { { 0, 0, 0, 0, 0, 0, 0xf0, 0x7f } };

/* bytes for NaN */
const union __nan_un ref___nan = { { 0, 0, 0xc0, 0xff } };
