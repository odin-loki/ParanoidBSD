/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
 *
 * sys/netpfil/ipfilter/netinet/ip_htable.c: ipf_htable_create() has
 * four exits after KMALLOC(iph) and only one of them frees it.
 *
 *   gcc -fsanitize=address -DOLD tools/verify/probes/htable_create_leak.c
 *       -> "SUMMARY: AddressSanitizer: 32 byte(s) leaked in 2
 *           allocation(s)."
 *   gcc -fsanitize=address       tools/verify/probes/htable_create_leak.c
 *       -> clean
 *
 * iph is not on softh->ipf_htables[] until the last statement of the
 * function, so until then it is the only reference.  The KMALLOCS
 * failure path does the right thing:
 *
 *	if (iph->iph_table == NULL) {
 *		KFREE(iph);
 *		...
 *		return (ENOMEM);
 *	}
 *
 * and the two size checks immediately above it, which return EINVAL,
 * did not.  Both rest on iph_size, a size_t COPYIN'd as part of the
 * caller's iphtable_t, so a caller holding the ipfilter ioctl leaks one
 * table per oversized request, without bound.
 *
 * (The `iph_size == 0' half of the first test is dead -- htab.iph_size
 * < 1 is rejected before the allocation -- but `iph_size >
 * ipf_htable_size_max' is not, and that is the one this models.)
 */

#include <stdlib.h>

#define	SIZE_MAX_MODEL	((size_t)-1)

struct iphtable {
	size_t	 iph_size;
	void	**iph_table;
};

static size_t htable_size_max = 64;

static int
ipf_htable_create(size_t requested_size)
{
	struct iphtable *iph;

	iph = malloc(sizeof(*iph));
	if (iph == NULL)
		return (12);		/* ENOMEM */
	iph->iph_size = requested_size;	/* COPYIN'd from the caller */

	if ((iph->iph_size == 0) || (iph->iph_size > htable_size_max)) {
#ifndef OLD
		free(iph);
#endif
		return (22);		/* EINVAL */
	}
	if (iph->iph_size > (SIZE_MAX_MODEL / sizeof(*iph->iph_table))) {
#ifndef OLD
		free(iph);
#endif
		return (22);		/* EINVAL */
	}

	iph->iph_table = calloc(iph->iph_size, sizeof(*iph->iph_table));
	if (iph->iph_table == NULL) {
		free(iph);		/* this one always did */
		return (12);
	}

	/* Success: iph goes on softh->ipf_htables[] and is not ours. */
	free(iph->iph_table);
	free(iph);
	return (0);
}

int
main(void)
{
	/* Each of the four ways out, once. */
	ipf_htable_create(0);			/* first EINVAL */
	ipf_htable_create(htable_size_max + 1);	/* first EINVAL, live half */
	ipf_htable_create(8);			/* success */
	return (0);
}
