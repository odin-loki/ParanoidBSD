/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
 *
 * sys/netpfil/ipfw/ip_fw_table_algo.c: ipv6_writemask() wrote only the
 * words the prefix covers, and every caller ANDs a whole 16-byte
 * address against it.
 *
 *   cbmc -DOLD --unwind 8 tools/verify/probes/ipv6_writemask.c
 *       -> 2 of 2 failed, FAILED
 *   cbmc       --unwind 8 tools/verify/probes/ipv6_writemask.c
 *       -> 0 of 2, SUCCESSFUL
 *
 * The original:
 *
 *     for (cp = (uint32_t *)addr6; mask >= 32; mask -= 32)
 *             *cp++ = 0xFFFFFFFF;
 *     if (mask > 0)
 *             *cp = htonl(mask ? ~((1 << (32 - mask)) - 1) : 0);
 *
 * leaves words alone in two ways.  A prefix of 64 writes two words and
 * stops; a prefix that is a multiple of 32 below 128 also skips the
 * `if (mask > 0)' word, where zero is exactly the value that word
 * needs.  APPLY_MASK, in sys/netinet/ip_fw.h, is four unconditional
 * __u6_addr32 ANDs, so the caller's key is ANDed with stack.
 *
 * Two callers matter and they are the two halves of one table:
 * tei_to_chash_ent() masks on INSERT, hash_ip6_slow() masks on LOOKUP,
 * each with its own `struct in6_addr mask6;' on its own frame.  A
 * `cidr:hash' ipfw table can therefore store an IPv6 prefix under one
 * key and look for it under another, and nothing reports it.
 *
 * The two assertions below are that pair: every word of the mask is
 * written, and two calls with the same prefix length agree.
 */

unsigned int nondet_uint(void);

#define	NWORDS	4

static unsigned int written[NWORDS];

static unsigned int
htonl_model(unsigned int x)
{
	/* Byte order does not change which words get written. */
	return (x);
}

static void
ipv6_writemask(unsigned int *addr6, unsigned char mask)
{
	unsigned int *cp;
#ifndef OLD
	unsigned int *end;
#endif
	int i;

	for (i = 0; i < NWORDS; i++)
		written[i] = 0;

#ifdef OLD
	for (cp = addr6; mask >= 32; mask -= 32) {
		written[cp - addr6] = 1;
		*cp++ = 0xFFFFFFFFu;
	}
	if (mask > 0) {
		written[cp - addr6] = 1;
		*cp = htonl_model(~((1u << (32 - mask)) - 1));
	}
#else
	end = addr6 + NWORDS;
	for (cp = addr6; mask >= 32 && cp < end; mask -= 32) {
		written[cp - addr6] = 1;
		*cp++ = 0xFFFFFFFFu;
	}
	if (cp < end) {
		written[cp - addr6] = 1;
		*cp++ = htonl_model(mask ? ~((1u << (32 - mask)) - 1) : 0);
	}
	while (cp < end) {
		written[cp - addr6] = 1;
		*cp++ = 0;
	}
#endif
}

int
main(void)
{
	unsigned int insert[NWORDS], lookup[NWORDS];
	unsigned int mlen = nondet_uint();
	int all_written = 1;
	int i;

	__CPROVER_assume(mlen <= 128);

	/* The mask each caller declares is an uninitialised local. */
	for (i = 0; i < NWORDS; i++) {
		insert[i] = nondet_uint();
		lookup[i] = nondet_uint();
	}

	ipv6_writemask(insert, (unsigned char)mlen);
	for (i = 0; i < NWORDS; i++)
		if (!written[i])
			all_written = 0;
	__CPROVER_assert(all_written,
	    "ipv6_writemask writes every word of the mask");

	ipv6_writemask(lookup, (unsigned char)mlen);
	for (i = 0; i < NWORDS; i++)
		__CPROVER_assert(insert[i] == lookup[i],
		    "two masks of the same prefix length agree");

	return (0);
}
