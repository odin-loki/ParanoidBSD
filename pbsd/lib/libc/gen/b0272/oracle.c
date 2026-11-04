/*
 * oracle.c -- reference implementation for batch b0272.
 *
 * Original C sources, concatenated, with every function renamed with a
 * "ref_" prefix.  Function bodies are UNMODIFIED.
 *
 * Sources:
 *   hbsd/src/lib/libc/gen/isinf.c
 *   hbsd/src/lib/libc/gen/basename_compat.c
 */

#include <errno.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * CHAR_BIT)
#endif

#ifndef MAXPATHLEN
#define MAXPATHLEN PATH_MAX
#endif

#ifndef _LITTLE_ENDIAN
#define _LITTLE_ENDIAN __ORDER_LITTLE_ENDIAN__
#endif
#ifndef _BIG_ENDIAN
#define _BIG_ENDIAN __ORDER_BIG_ENDIAN__
#endif
#ifndef _BYTE_ORDER
#define _BYTE_ORDER __BYTE_ORDER__
#endif

#ifndef _IEEE_WORD_ORDER
#define _IEEE_WORD_ORDER _BYTE_ORDER
#endif

/* --- fpmath.h / _fpmath.h (amd64) --- */

union IEEEf2bits {
	float f;
	struct {
#if _BYTE_ORDER == _LITTLE_ENDIAN
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

#define DBL_MANH_SIZE 20
#define DBL_MANL_SIZE 32

union IEEEd2bits {
	double d;
	struct {
#if _BYTE_ORDER == _LITTLE_ENDIAN
#if _IEEE_WORD_ORDER == _LITTLE_ENDIAN
		unsigned int manl : 32;
#endif
		unsigned int manh : 20;
		unsigned int exp : 11;
		unsigned int sign : 1;
#if _IEEE_WORD_ORDER == _BIG_ENDIAN
		unsigned int manl : 32;
#endif
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

/* ------------------------------------------------------------------ */
/* isinf.c                                                            */
/* ------------------------------------------------------------------ */

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
ref___isinf(double d)
{
	union IEEEd2bits u;

	u.d = d;
	return (u.bits.exp == 2047 && u.bits.manl == 0 && u.bits.manh == 0);
}

int
ref___isinff(float f)
{
	union IEEEf2bits u;

	u.f = f;
	return (u.bits.exp == 255 && u.bits.man == 0);
}

int
ref___isinfl(long double e)
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

/* ------------------------------------------------------------------ */
/* basename_compat.c                                                  */
/* ------------------------------------------------------------------ */

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
ref___freebsd11_basename_r(const char *path, char *bname)
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
ref___freebsd11_basename(char *path)
{
	static char *bname = NULL;

	if (bname == NULL) {
		bname = (char *)malloc(MAXPATHLEN);
		if (bname == NULL)
			return (NULL);
	}
	return (ref___freebsd11_basename_r(path, bname));
}
