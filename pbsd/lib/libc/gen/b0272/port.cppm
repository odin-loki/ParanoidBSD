module;

#include <cerrno>
#include <climits>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <float.h>

export module pbsd.lib.libc.gen.b0272;

export namespace pbsd::lib_libc_gen::b0272 {

#ifndef MAXPATHLEN
#define MAXPATHLEN PATH_MAX
#endif

union IEEEf2bits {
	float f;
	struct {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		unsigned int man : 23;
		unsigned int exp : 8;
		unsigned int sign : 1;
#else /* _BIG_ENDIAN */
		unsigned int sign : 1;
		unsigned int exp : 8;
		unsigned int man : 23;
#endif
	} bits;
};

union IEEEd2bits {
	double d;
	struct {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		unsigned int manl : 32;
		unsigned int manh : 20;
		unsigned int exp : 11;
		unsigned int sign : 1;
#else /* _BIG_ENDIAN */
		unsigned int sign : 1;
		unsigned int exp : 11;
		unsigned int manh : 20;
		unsigned int manl : 32;
#endif
	} bits;
};

union IEEEl2bits {
	long double e;
	struct {
		unsigned int manl : 32;
		unsigned int manh : 32;
		unsigned int exp : 15;
		unsigned int sign : 1;
		unsigned int junkl : 16;
		unsigned int junkh : 32;
	} bits;
	struct {
		unsigned long man : 64;
		unsigned int expsign : 16;
		unsigned long junk : 48;
	} xbits;
};

#define LDBL_NBIT 0x80000000
#define mask_nbit_l(u) ((u).bits.manh &= ~LDBL_NBIT)

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2004 David Schultz <das@FreeBSD.ORG>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

int
__isinf(double d)
{
	union IEEEd2bits u;

	u.d = d;
	return (u.bits.exp == 2047 && u.bits.manl == 0 && u.bits.manh == 0);
}

int
__isinff(float f)
{
	union IEEEf2bits u;

	u.f = f;
	return (u.bits.exp == 255 && u.bits.man == 0);
}

int
__isinfl(long double e)
{
	union IEEEl2bits u;

	u.e = e;
	mask_nbit_l(u);
#if LDBL_MANT_DIG == 53
	return (u.bits.exp == 2047 && u.bits.manl == 0 && u.bits.manh == 0);
#else
	return (u.bits.exp == 32767 && u.bits.manl == 0 && u.bits.manh == 0);
#endif
}

/*	$OpenBSD: basename.c,v 1.14 2005/08/08 08:05:33 espie Exp $	*/

/*
 * Copyright (c) 1997, 2004 Todd C. Miller <Todd.Miller@courtesan.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

char *
__freebsd11_basename_r(const char *path, char *bname)
{
	const char *endp, *startp;
	size_t len;

	/* Empty or NULL string gets treated as "." */
	if (path == NULL || *path == '\0') {
		bname[0] = '.';
		bname[1] = '\0';
		return (bname);
	}

	/* Strip any trailing slashes */
	endp = path + strlen(path) - 1;
	while (endp > path && *endp == '/')
		endp--;

	/* All slashes becomes "/" */
	if (endp == path && *endp == '/') {
		bname[0] = '/';
		bname[1] = '\0';
		return (bname);
	}

	/* Find the start of the base */
	startp = endp;
	while (startp > path && *(startp - 1) != '/')
		startp--;

	len = endp - startp + 1;
	if (len >= MAXPATHLEN) {
		errno = ENAMETOOLONG;
		return (NULL);
	}
	memcpy(bname, startp, len);
	bname[len] = '\0';
	return (bname);
}

char *
__freebsd11_basename(char *path)
{
	static char *bname = NULL;

	if (bname == NULL) {
		bname = (char *)malloc(MAXPATHLEN);
		if (bname == NULL)
			return (NULL);
	}
	return (__freebsd11_basename_r(path, bname));
}

} // namespace pbsd::lib_libc_gen::b0272
