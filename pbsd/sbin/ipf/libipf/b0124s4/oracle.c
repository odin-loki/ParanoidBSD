/*
 * oracle.c -- concatenated reference implementations for PBSD batch b0124s4.
 */

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * 8)
#endif

#include <stddef.h>

typedef struct mb_s {
	struct mb_s *mb_next;
	char *mb_data;
	void *mb_ifp;
	int mb_len;
	int mb_flags;
	unsigned long mb_buf[2048];
} mb_t;

/*
 * Copyright (C) 2012 by Darren Reed.
 *
 * See the IPFILTER.LICENCE file for details on licencing.
 *
 * $Id: msgdsize.c,v 1.2.4.3 2012/07/22 08:04:24 darren_r Exp $
 */

size_t
ref_msgdsize(mb_t *orig)
{
	size_t sz = 0;
	mb_t *m;

	for (m = orig; m != NULL; m = m->mb_next)
		sz += m->mb_len;
	return (sz);
}
