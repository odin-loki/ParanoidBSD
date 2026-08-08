/*
 * Copyright (C) 2012 by Darren Reed.
 *
 * See the IPFILTER.LICENCE file for details on licencing.
 *
 * $Id$
 */

module;

#include <stdio.h>
#include <string.h>

/*
 * ipf.h routes all of libipf's user visible output through these macros.
 * They are reproduced here verbatim so the port keeps calling printf(3)
 * with exactly the same arguments as the C original.
 */
#define	PRINTF	(void)printf
#define	FPRINTF	(void)fprintf

export module pbsd.sbin.ipf.libipf.b0124s1;

export namespace pbsd::sbin_ipf_libipf::b0124s1 {

void
printifname(char *format, char *name, void *ifp)
{
	PRINTF("%s%s", format, name);
	if ((ifp == NULL) && strcmp(name, "-") && strcmp(name, "*"))
		PRINTF("(!)");
}

} // namespace pbsd::sbin_ipf_libipf::b0124s1
