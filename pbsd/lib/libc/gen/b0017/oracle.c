/*
 * Copyright (c) 1993 Martin Birgmeier
 * All rights reserved.
 *
 * You may redistribute unmodified or modified versions of this source
 * code provided that the above copyright notice and this and the
 * following conditions are retained.
 *
 * This software is provided ``as is'', and comes with no warranties
 * of any kind. I shall in no event be liable for anything that happens
 * to anyone/anything when using this software.
 */

/*
 * Reference oracle for batch b0017: the original C sources concatenated with a
 * ref_ prefix on each function name.  Function bodies are unmodified.
 *
 *	lib/libc/gen/mrand48.c
 *	lib/libc/gen/lcong48.c
 *	lib/libc/gen/srand48.c
 *	lib/libc/gen/seed48.c
 */

#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>

/*
 * Definitions taken verbatim from lib/libc/gen/rand48.h.  That header is
 * included by every source in this batch but also pulls in the FreeBSD-private
 * fpmath.h for the erand48() bit fiddling, which none of these four functions
 * use; only the parts the bodies below expand are reproduced here.
 */
#define	RAND48_SEED_0	(0x330e)
#define	RAND48_SEED_1	(0xabcd)
#define	RAND48_SEED_2	(0x1234)
#define	RAND48_MULT_0	(0xe66d)
#define	RAND48_MULT_1	(0xdeec)
#define	RAND48_MULT_2	(0x0005)
#define	RAND48_ADD	(0x000b)

typedef uint64_t uint48;

#define	TOUINT48(x, y, z)						\
	((uint48)(x) + (((uint48)(y)) << 16) + (((uint48)(z)) << 32))

#define	RAND48_SEED	TOUINT48(RAND48_SEED_0, RAND48_SEED_1, RAND48_SEED_2)
#define	RAND48_MULT	TOUINT48(RAND48_MULT_0, RAND48_MULT_1, RAND48_MULT_2)

#define	LOADRAND48(l, x) do {						\
	(l) = TOUINT48((x)[0], (x)[1], (x)[2]);				\
} while (0)

#define	STORERAND48(l, x) do {						\
	(x)[0] = (unsigned short)(l);					\
	(x)[1] = (unsigned short)((l) >> 16);				\
	(x)[2] = (unsigned short)((l) >> 32);				\
} while (0)

#define	_DORAND48(l) do {						\
	(l) = (l) * _rand48_mult + _rand48_add;				\
} while (0)

/*
 * Definitions taken verbatim from lib/libc/gen/_rand48.c, which owns the state
 * the four functions in this batch share.
 */
uint48 _rand48_seed = RAND48_SEED;
uint48 _rand48_mult = RAND48_MULT;
uint48 _rand48_add = RAND48_ADD;

long
ref_mrand48(void)
{
	_DORAND48(_rand48_seed);
	return ((int)((_rand48_seed >> 16) & 0xffffffff));
}

void
ref_lcong48(unsigned short p[7])
{
	LOADRAND48(_rand48_seed, &p[0]);
	LOADRAND48(_rand48_mult, &p[3]);
	_rand48_add = p[6];
}

void
ref_srand48(long seed)
{
	_rand48_seed = TOUINT48(RAND48_SEED_0, (unsigned short)seed,
	    (unsigned short)(seed >> 16));
	_rand48_mult = RAND48_MULT;
	_rand48_add = RAND48_ADD;
}

unsigned short *
ref_seed48(unsigned short xseed[3])
{
	static unsigned short sseed[3];

	STORERAND48(_rand48_seed, sseed);
	LOADRAND48(_rand48_seed, xseed);
	_rand48_mult = RAND48_MULT;
	_rand48_add = RAND48_ADD;
	return (sseed);
}
