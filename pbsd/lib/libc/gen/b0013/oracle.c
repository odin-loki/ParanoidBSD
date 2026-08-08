/*
 * b0013 oracle -- the specification.
 *
 * lib/libc/gen/_rand48.c concatenated, with every externally visible name
 * given a `ref_' prefix.  No function body has been modified.
 *
 * lib/libc/gen/rand48.h is a private header that is not reachable from this
 * directory, so its contents are reproduced below unchanged apart from the
 * ref_ renaming of the three globals it declares.
 */

#include <stdint.h>

#define	RAND48_SEED_0	(0x330e)
#define	RAND48_SEED_1	(0xabcd)
#define	RAND48_SEED_2	(0x1234)
#define	RAND48_MULT_0	(0xe66d)
#define	RAND48_MULT_1	(0xdeec)
#define	RAND48_MULT_2	(0x0005)
#define	RAND48_ADD	(0x000b)

typedef uint64_t uint48;

extern uint48 ref__rand48_seed;
extern uint48 ref__rand48_mult;
extern uint48 ref__rand48_add;

#define	TOUINT48(x, y, z)						\
	((uint48)(x) + (((uint48)(y)) << 16) + (((uint48)(z)) << 32))

#define	RAND48_SEED	TOUINT48(RAND48_SEED_0, RAND48_SEED_1, RAND48_SEED_2)
#define	RAND48_MULT	TOUINT48(RAND48_MULT_0, RAND48_MULT_1, RAND48_MULT_2)

/* ======================= lib/libc/gen/_rand48.c ======================= */

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

uint48 ref__rand48_seed = RAND48_SEED;
uint48 ref__rand48_mult = RAND48_MULT;
uint48 ref__rand48_add = RAND48_ADD;
