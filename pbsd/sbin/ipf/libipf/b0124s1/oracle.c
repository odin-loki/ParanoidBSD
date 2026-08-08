/*
 * oracle.c -- concatenated reference implementations for PBSD batch b0124s1.
 */

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * 8)
#endif

#include <stdio.h>
#include <string.h>

#define PRINTF (void)printf

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
