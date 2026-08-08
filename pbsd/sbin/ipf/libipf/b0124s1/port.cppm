module;

#include <cstdio>
#include <cstring>

export module pbsd.sbin.ipf.libipf.b0124s1;

export namespace pbsd::sbin_ipf_libipf::b0124s1 {

#define PRINTF (void)std::printf

/*
 * Copyright (C) 2012 by Darren Reed.
 *
 * See the IPFILTER.LICENCE file for details on licencing.
 *
 * $Id$
 */

void
printifname(char *format, char *name, void *ifp)
{
	PRINTF("%s%s", format, name);
	if ((ifp == NULL) && std::strcmp(name, "-") && std::strcmp(name, "*"))
		PRINTF("(!)");
}

} // namespace pbsd::sbin_ipf_libipf::b0124s1
