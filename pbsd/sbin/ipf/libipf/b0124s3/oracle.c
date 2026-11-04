/*
 * oracle.c -- reference implementation for PBSD batch b0124s3.
 */

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * 8)
#endif

#include <stddef.h>
#include <stdlib.h>

typedef struct alist_s {
	struct alist_s *al_next;
	int al_not;
	int al_family;
	unsigned char al_i6addr[16];
	unsigned char al_i6mask[16];
} alist_t;

/*
 * Copyright (C) 2012 by Darren Reed.
 *
 * See the IPFILTER.LICENCE file for details on licencing.
 *
 * $Id: alist_free.c,v 1.3.2.2 2012/07/22 08:04:24 darren_r Exp $
 */

void
ref_alist_free(alist_t *hosts)
{
	alist_t *a, *next;

	for (a = hosts; a != NULL; a = next) {
		next = a->al_next;
		free(a);
	}
}
