module;

#define _DEFAULT_SOURCE

#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <cassert>
#include <cctype>
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cstdlib>

export module pbsd.lib.libc.inet.b0324;

export namespace pbsd::lib_libc_inet::b0324 {

typedef unsigned char u_char;
typedef unsigned long u_long;

/*-
 * SPDX-License-Identifier: (BSD-3-Clause AND ISC)
 *
 * Copyright (c) 1983, 1990, 1993
 *    The Regents of the University of California.  All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Portions Copyright (c) 1993 by Digital Equipment Corporation.
 * 
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies, and that
 * the name of Digital Equipment Corporation not be used in advertising or
 * publicity pertaining to distribution of the document or software without
 * specific, written prior permission.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND DIGITAL EQUIPMENT CORP. DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS.   IN NO EVENT SHALL DIGITAL EQUIPMENT
 * CORPORATION BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

/*
 * Copyright (c) 2004 by Internet Systems Consortium, Inc. ("ISC")
 * Portions Copyright (c) 1996-1999 by Internet Software Consortium.
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

/*%
 * Ascii internet address interpretation routine.
 * The value returned is in network order.
 */
in_addr_t		/* XXX should be struct in_addr :( */
inet_addr(const char *cp) {
	struct in_addr val;

	if (inet_aton(cp, &val))
		return (val.s_addr);
	return (INADDR_NONE);
}

/*%
 * Check whether "cp" is a valid ascii representation
 * of an Internet address and convert to a binary address.
 * Returns 1 if the address is valid, 0 if not.
 * This replaces inet_addr, the return value from which
 * cannot distinguish between failure and a local broadcast address.
 */
int
inet_aton(const char *cp, struct in_addr *addr) {
	u_long val;
	int base, n;
	char c;
	u_int8_t parts[4];
	u_int8_t *pp = parts;
	int digit;

	c = *cp;
	for (;;) {
		/*
		 * Collect number up to ``.''.
		 * Values are specified as for C:
		 * 0x=hex, 0=octal, isdigit=decimal.
		 */
		if (!isdigit((unsigned char)c))
			return (0);
		val = 0; base = 10; digit = 0;
		if (c == '0') {
			c = *++cp;
			if (c == 'x' || c == 'X')
				base = 16, c = *++cp;
			else {
				base = 8;
				digit = 1 ;
			}
		}
		for (;;) {
			if (isascii(c) && isdigit((unsigned char)c)) {
				if (base == 8 && (c == '8' || c == '9'))
					return (0);
				val = (val * base) + (c - '0');
				c = *++cp;
				digit = 1;
			} else if (base == 16 && isascii(c) && 
				   isxdigit((unsigned char)c)) {
				val = (val << 4) |
					(c + 10 - (islower((unsigned char)c) ? 'a' : 'A'));
				c = *++cp;
				digit = 1;
			} else
				break;
		}
		if (c == '.') {
			/*
			 * Internet format:
			 *	a.b.c.d
			 *	a.b.c	(with c treated as 16 bits)
			 *	a.b	(with b treated as 24 bits)
			 */
			if (pp >= parts + 3 || val > 0xffU)
				return (0);
			*pp++ = val;
			c = *++cp;
		} else
			break;
	}
	/*
	 * Check for trailing characters.
	 */
	if (c != '\0' && (!isascii(c) || !isspace((unsigned char)c)))
		return (0);
	/*
	 * Did we get a valid digit?
	 */
	if (!digit)
		return (0);
	/*
	 * Concoct the address according to
	 * the number of parts specified.
	 */
	n = pp - parts + 1;
	switch (n) {
	case 1:				/*%< a -- 32 bits */
		break;

	case 2:				/*%< a.b -- 8.24 bits */
		if (val > 0xffffffU)
			return (0);
		val |= (uint32_t)parts[0] << 24;
		break;

	case 3:				/*%< a.b.c -- 8.8.16 bits */
		if (val > 0xffffU)
			return (0);
		val |= ((uint32_t)parts[0] << 24) | (parts[1] << 16);
		break;

	case 4:				/*%< a.b.c.d -- 8.8.8.8 bits */
		if (val > 0xffU)
			return (0);
		val |= ((uint32_t)parts[0] << 24) | (parts[1] << 16) |
		    (parts[2] << 8);
		break;
	}
	if (addr != NULL)
		addr->s_addr = htonl(val);
	return (1);
}

/*	$OpenBSD: inet_net_pton.c,v 1.14 2022/12/27 17:10:06 jmc Exp $	*/

/*
 * Copyright (c) 2012 by Gilles Chehade <gilles@openbsd.org>
 * Copyright (c) 1996,1999 by Internet Software Consortium.
 *
 * SPDX-License-Identifier: ISC
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND INTERNET SOFTWARE CONSORTIUM DISCLAIMS
 * ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL INTERNET SOFTWARE
 * CONSORTIUM BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

static int	inet_net_pton_ipv4(const char *, u_char *, size_t);
static int	inet_net_pton_ipv6(const char *, u_char *, size_t);

/*
 * static int
 * inet_net_pton(af, src, dst, size)
 *	convert network number from presentation to network format.
 *	accepts hex octets, hex strings, decimal octets, and /CIDR.
 *	"size" is in bytes and describes "dst".
 * return:
 *	number of bits, either imputed classfully or specified with /CIDR,
 *	or -1 if some failure occurred (check errno).  ENOENT means it was
 *	not a valid network specification.
 * author:
 *	Paul Vixie (ISC), June 1996
 */
int
inet_net_pton(int af, const char *src, void *dst, size_t size)
{
	switch (af) {
	case AF_INET:
		return (inet_net_pton_ipv4(src, dst, size));
	case AF_INET6:
		return (inet_net_pton_ipv6(src, dst, size));
	default:
		errno = EAFNOSUPPORT;
		return (-1);
	}
}

/*
 * static int
 * inet_net_pton_ipv4(src, dst, size)
 *	convert IPv4 network number from presentation to network format.
 *	accepts hex octets, hex strings, decimal octets, and /CIDR.
 *	"size" is in bytes and describes "dst".
 * return:
 *	number of bits, either imputed classfully or specified with /CIDR,
 *	or -1 if some failure occurred (check errno).  ENOENT means it was
 *	not an IPv4 network specification.
 * note:
 *	network byte order assumed.  this means 192.5.5.240/28 has
 *	0b11110000 in its fourth octet.
 * author:
 *	Paul Vixie (ISC), June 1996
 */
static int
inet_net_pton_ipv4(const char *src, u_char *dst, size_t size)
{
	static const char
		xdigits[] = "0123456789abcdef",
		digits[] = "0123456789";
	int n, ch, tmp, dirty, bits;
	const u_char *odst = dst;

	ch = (unsigned char)*src++;
	if (ch == '0' && (src[0] == 'x' || src[0] == 'X')
	    && isascii((unsigned char)src[1]) && isxdigit((unsigned char)src[1])) {
		/* Hexadecimal: Eat nybble string. */
		if (size == 0)
			goto emsgsize;
		tmp = 0, dirty = 0;
		src++;	/* skip x or X. */
		while ((ch = (unsigned char)*src++) != '\0' &&
		    isascii(ch) && isxdigit(ch)) {
			if (isupper(ch))
				ch = tolower(ch);
			n = strchr(xdigits, ch) - xdigits;
			assert(n >= 0 && n <= 15);
			if (dirty == 0)
				tmp = n;
			else
				tmp = (tmp << 4) | n;
			if (++dirty == 2) {
				if (size-- == 0)
					goto emsgsize;
				*dst++ = (u_char) tmp;
				dirty = 0;
			}
		}
		if (dirty) {  /* Odd trailing nybble? */
			if (size-- == 0)
				goto emsgsize;
			*dst++ = (u_char) (tmp << 4);
		}
	} else if (isascii(ch) && isdigit(ch)) {
		/* Decimal: eat dotted digit string. */
		for (;;) {
			tmp = 0;
			do {
				n = strchr(digits, ch) - digits;
				assert(n >= 0 && n <= 9);
				tmp *= 10;
				tmp += n;
				if (tmp > 255)
					goto enoent;
			} while ((ch = (unsigned char)*src++) != '\0' &&
				 isascii(ch) && isdigit(ch));
			if (size-- == 0)
				goto emsgsize;
			*dst++ = (u_char) tmp;
			if (ch == '\0' || ch == '/')
				break;
			if (ch != '.')
				goto enoent;
			ch = (unsigned char)*src++;
			if (!isascii(ch) || !isdigit(ch))
				goto enoent;
		}
	} else
		goto enoent;

	bits = -1;
	if (ch == '/' && isascii((unsigned char)src[0]) &&
	    isdigit((unsigned char)src[0]) && dst > odst) {
		/* CIDR width specifier.  Nothing can follow it. */
		ch = (unsigned char)*src++;	/* Skip over the /. */
		bits = 0;
		do {
			n = strchr(digits, ch) - digits;
			assert(n >= 0 && n <= 9);
			bits *= 10;
			bits += n;
			if (bits > 32)
				goto emsgsize;
		} while ((ch = (unsigned char)*src++) != '\0' &&
			 isascii(ch) && isdigit(ch));
		if (ch != '\0')
			goto enoent;
	}

	/* Fiery death and destruction unless we prefetched EOS. */
	if (ch != '\0')
		goto enoent;

	/* If nothing was written to the destination, we found no address. */
	if (dst == odst)
		goto enoent;
	/* If no CIDR spec was given, infer width from net class. */
	if (bits == -1) {
		if (*odst >= 240)	/* Class E */
			bits = 32;
		else if (*odst >= 224)	/* Class D */
			bits = 4;
		else if (*odst >= 192)	/* Class C */
			bits = 24;
		else if (*odst >= 128)	/* Class B */
			bits = 16;
		else			/* Class A */
			bits = 8;
		/* If imputed mask is narrower than specified octets, widen. */
		if (bits < ((dst - odst) * 8))
			bits = (dst - odst) * 8;
	}
	/* Extend network to cover the actual mask. */
	while (bits > ((dst - odst) * 8)) {
		if (size-- == 0)
			goto emsgsize;
		*dst++ = '\0';
	}
	return (bits);

 enoent:
	errno = ENOENT;
	return (-1);

 emsgsize:
	errno = EMSGSIZE;
	return (-1);
}


static int
inet_net_pton_ipv6(const char *src, u_char *dst, size_t size)
{
	struct in6_addr	 in6;
	int		 ret;
	int		 bits;
	size_t		 bytes;
	char		 buf[INET6_ADDRSTRLEN + sizeof("/128")];
	char		*sep;
	const char	*errstr;

	if (strlcpy(buf, src, sizeof buf) >= sizeof buf) {
		errno = EMSGSIZE;
		return (-1);
	}

	sep = strchr(buf, '/');
	if (sep != NULL)
		*sep++ = '\0';

	ret = inet_pton(AF_INET6, buf, &in6);
	if (ret != 1)
		return (-1);

	if (sep == NULL)
		bits = 128;
	else {
		bits = strtonum(sep, 0, 128, &errstr);
		if (errstr) {
			errno = EINVAL;
			return (-1);
		}
	}

	bytes = (bits + 7) / 8;
	if (bytes > size) {
		errno = EMSGSIZE;
		return (-1);
	}
	memcpy(dst, &in6.s6_addr, bytes);
	return (bits);
}

} // namespace pbsd::lib_libc_inet::b0324
