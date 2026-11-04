module;

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>

export module pbsd.sbin.ipf.libipf.b0124;

export namespace pbsd::sbin_ipf_libipf::b0124 {

#define PRINTF (void)std::printf

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
printifname(char *format, char *name, void *ifp)
{
	PRINTF("%s%s", format, name);
	if ((ifp != NULL) && std::strcmp(name, "-") && std::strcmp(name, "*"))
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
resetlexer(void)
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
alist_free(alist_t *hosts)
{
	alist_t *a, *next;

	for (a = hosts; a != NULL; a = next) {
		next = a->al_next;
		std::free(a);
	}
}

/*
 * Copyright (C) 2012 by Darren Reed.
 *
 * See the IPFILTER.LICENCE file for details on licencing.
 *
 * $Id: msgdsize.c,v 1.2.4.3 2012/07/22 08:04:24 darren_r Exp $
 */

std::size_t
msgdsize(mb_t *orig)
{
	std::size_t sz = 0;
	mb_t *m;

	for (m = orig; m != NULL; m = m->mb_next)
		sz += m->mb_len;
	return (sz);
}

} // namespace pbsd::sbin_ipf_libipf::b0124
