/*
 * oracle.c -- concatenated reference implementations for PBSD batch b0124.
 */

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * 8)
#endif

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PRINTF (void)printf

typedef struct alist_s {
	struct alist_s *al_next;
	int al_not;
	int al_family;
	unsigned char al_i6addr[16];
	unsigned char al_i6mask[16];
} alist_t;

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
 * $Id$
 */

void
ref_printifname(char *format, char *name, void *ifp)
{
	PRINTF("%s%s", format, name);
	if ((ifp == NULL) && strcmp(name, "-") && strcmp(name, "*"))
		PRINTF("(!)");
}

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
