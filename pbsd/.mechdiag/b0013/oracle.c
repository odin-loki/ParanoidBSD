/* ---- nrand48.c ---- */
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

#include "rand48.h"

long
ref_nrand48(unsigned short xseed[3])
{
	uint48 tmp;

	DORAND48(tmp, xseed);
	return ((tmp >> 17) & 0x7fffffff);
}

/* ---- erand48.c ---- */
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

#include "rand48.h"

double
ref_erand48(unsigned short xseed[3])
{
	uint48 tmp;

	ERAND48_BEGIN;
	DORAND48(tmp, xseed);
	ERAND48_END(tmp);
}

/* ---- jrand48.c ---- */
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

#include "rand48.h"

long
ref_jrand48(unsigned short xseed[3])
{
	uint48 tmp;

	DORAND48(tmp, xseed);
	return ((int)((tmp >> 16) & 0xffffffff));
}

