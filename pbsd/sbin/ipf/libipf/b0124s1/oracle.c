/*
 * Reference oracle for batch b0124s1.
 *
 * Sources concatenated verbatim from HardenedBSD, every function renamed
 * with a ref_ prefix.  Function bodies are UNMODIFIED.  Only the includes
 * and the defines that "ipf.h" would have supplied have been added.
 */

#include <stdio.h>
#include <string.h>

#ifndef	LONG_BIT
#define	LONG_BIT	(sizeof(long) * 8)
#endif

/* from sbin/ipf/libipf/ipf.h */
#ifndef	PRINTF
#define	PRINTF	(void)printf
#endif
#ifndef	FPRINTF
#define	FPRINTF	(void)fprintf
#endif

/* ------------------------------------------------------------------------ */
/* sbin/ipf/libipf/printifname.c                                            */
/* ------------------------------------------------------------------------ */

/*
 * Copyright (C) 2012 by Darren Reed.
 *
 * See the IPFILTER.LICENCE file for details on licencing.
 *
 * $Id$
 */

void
ref_printifname(char *format, char *name, void *ifp)
{
	PRINTF("%s%s", format, name);
	if ((ifp == NULL) && strcmp(name, "-") && strcmp(name, "*"))
		PRINTF("(!)");
}
