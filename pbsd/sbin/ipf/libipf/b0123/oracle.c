/*
 * oracle.c -- reference implementations for batch b0123.
 *
 * Original HardenedBSD libipf sources, concatenated verbatim except for a
 * ref_ prefix on each ported function.  Supporting definitions required for
 * standalone compilation are provided above the function bodies; function
 * bodies are otherwise unmodified.
 */

#include <stdlib.h>

#define USE_INET6

#define AF_UNSPEC	0
#define AF_INET		2
#define AF_INET6	28

typedef struct mbuf mb_t;

const char *
ref_familyname(int family)
{
	if (family == AF_INET)
		return ("inet");
#ifdef USE_INET6
	if (family == AF_INET6)
		return ("inet6");
#endif
	return ("unknown");
}

int
ref_ftov(int version)
{
#ifdef USE_INET6
	if (version == AF_INET6)
		return (6);
#endif
	if (version == AF_INET)
		return (4);
	if (version == AF_UNSPEC)
		return (0);
	return (-1);
}

int
ref_vtof(int version)
{
#ifdef USE_INET6
	if (version == 6)
		return (AF_INET6);
#endif
	if (version == 4)
		return (AF_INET);
	if (version == 0)
		return (AF_UNSPEC);
	return (-1);
}

/*
 * Copyright (C) 2012 by Darren Reed.
 *
 * See the IPFILTER.LICENCE file for details on licencing.
 *
 * $Id: freembt.c,v 1.3.2.2 2012/07/22 08:04:24 darren_r Exp $
 */

void
ref_freembt(mb_t *m)
{

	free(m);
}
