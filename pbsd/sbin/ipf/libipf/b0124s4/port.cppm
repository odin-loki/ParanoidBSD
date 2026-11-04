module;

#include <cstddef>

export module pbsd.sbin.ipf.libipf.b0124s4;

export namespace pbsd::sbin_ipf_libipf::b0124s4 {

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

std::size_t
msgdsize(mb_t *orig)
{
	std::size_t sz = 0;
	mb_t *m;

	for (m = orig; m != NULL; m = m->mb_next)
		sz += m->mb_len;
	return (sz);
}

} // namespace pbsd::sbin_ipf_libipf::b0124s4
