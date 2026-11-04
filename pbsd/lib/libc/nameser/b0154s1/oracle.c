/*
 * b0154s1 oracle: the original HardenedBSD C sources for
 *
 *	lib/libc/nameser/ns_netint.c
 *
 * concatenated, with every function renamed with a "ref_" prefix so that it
 * can be linked next to the host libc and next to the C++23 port.  Function
 * bodies are UNMODIFIED except for the mechanical renaming of calls to other
 * ref_ functions in this file.
 */

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE 1
#endif

#include <sys/types.h>

#include <netinet/in.h>
#include <arpa/nameser.h>

/* ------------------------------------------------------------------------ */
/* lib/libc/nameser/ns_netint.c						    */
/* ------------------------------------------------------------------------ */

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
ref_ns_get16(const u_char *src) {
	u_int dst;

	NS_GET16(dst, src);
	return (dst);
}

u_long
ref_ns_get32(const u_char *src) {
	u_long dst;

	NS_GET32(dst, src);
	return (dst);
}

void
ref_ns_put16(u_int src, u_char *dst) {
	NS_PUT16(src, dst);
}

void
ref_ns_put32(u_long src, u_char *dst) {
	NS_PUT32(src, dst);
}

/*! \file */
