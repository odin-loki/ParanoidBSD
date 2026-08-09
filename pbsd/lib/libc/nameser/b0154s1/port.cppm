// b0154s1 — C++23 port of the HardenedBSD libc nameser helpers.
//
// Sources ported in this module:
//	lib/libc/nameser/ns_netint.c
//
// The port is behaviour-preserving down to integer signedness, evaluation
// order, truncating assignments and pointer arithmetic.  Nothing is fixed,
// tidied or hardened.

module;

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE 1
#endif

#include <sys/types.h>

#include <netinet/in.h>
#include <arpa/nameser.h>

#include <stdint.h>

export module pbsd.lib.libc.nameser.b0154s1;

export namespace pbsd::lib_libc_nameser::b0154s1 {

/* ---------------------------------------------------------------------- */
/* lib/libc/nameser/ns_netint.c						  */
/* ---------------------------------------------------------------------- */

/*-
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2004 by Internet Systems Consortium, Inc. ("ISC")
 * Copyright (c) 1996,1999 by Internet Software Consortium.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS.  IN NO EVENT SHALL ISC BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/* Public. */

u_int
ns_get16(const u_char *src) {
	u_int dst;

	{
		const u_char *t_cp = (const u_char *)(src);
		dst = ((uint16_t)t_cp[0] << 8)
		    | ((uint16_t)t_cp[1])
		    ;
		src += NS_INT16SZ;
	}
	return (dst);
}

u_long
ns_get32(const u_char *src) {
	u_long dst;

	{
		const u_char *t_cp = (const u_char *)(src);
		dst = ((uint32_t)t_cp[0] << 24)
		    | ((uint32_t)t_cp[1] << 16)
		    | ((uint32_t)t_cp[2] << 8)
		    | ((uint32_t)t_cp[3])
		    ;
		src += NS_INT32SZ;
	}
	return (dst);
}

void
ns_put16(u_int src, u_char *dst) {
	uint16_t t_s = (uint16_t)(src);
	u_char *t_cp = (u_char *)(dst);
	*t_cp++ = t_s >> 8;
	*t_cp   = t_s;
	dst += NS_INT16SZ;
}

void
ns_put32(u_long src, u_char *dst) {
	uint32_t t_l = (uint32_t)(src);
	u_char *t_cp = (u_char *)(dst);
	*t_cp++ = t_l >> 24;
	*t_cp++ = t_l >> 16;
	*t_cp++ = t_l >> 8;
	*t_cp   = t_l;
	dst += NS_INT32SZ;
}

/*! \file */

} // namespace pbsd::lib_libc_nameser::b0154s1
