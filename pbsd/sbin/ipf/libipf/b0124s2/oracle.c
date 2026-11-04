/*
 * oracle.c -- concatenated reference implementations for PBSD batch b0124s2.
 */

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * 8)
#endif

#include <stddef.h>

/*
 * Copyright (C) 2012 by Darren Reed.
 *
 * See the IPFILTER.LICENCE file for details on licencing.
 *
 * $Id$
 */

long	string_start = -1;
long	string_end = -1;
char	*string_val = NULL;
long	pos = 0;


void
ref_resetlexer(void)
{
	string_start = -1;
	string_end = -1;
	string_val = NULL;
	pos = 0;
}
