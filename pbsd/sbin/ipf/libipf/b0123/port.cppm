module;

#include <stdlib.h>

export module pbsd.sbin.ipf.libipf.b0123;

export namespace pbsd::sbin_ipf_libipf::b0123 {

#define USE_INET6

#define AF_UNSPEC	0
#define AF_INET		2
#define AF_INET6	28

typedef struct mbuf mb_t;

const char *
familyname(int family)
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
ftov(int version)
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
vtof(int version)
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
freembt(mb_t *m)
{

	free(m);
}

} /* namespace pbsd::sbin_ipf_libipf::b0123 */
