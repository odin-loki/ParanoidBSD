module;

#include <cstddef>
#include <cstdlib>

export module pbsd.sbin.ipf.libipf.b0124s3;

export namespace pbsd::sbin_ipf_libipf::b0124s3 {

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
alist_free(alist_t *hosts)
{
	alist_t *a, *next;

	for (a = hosts; a != NULL; a = next) {
		next = a->al_next;
		std::free(a);
	}
}

} // namespace pbsd::sbin_ipf_libipf::b0124s3
