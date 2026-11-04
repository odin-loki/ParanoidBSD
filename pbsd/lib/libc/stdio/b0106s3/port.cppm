module;

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <cerrno>
#include <climits>
#include <cstdarg>
#include <locale.h>

export module pbsd.lib.libc.stdio.b0106s3;

export namespace pbsd::lib_libc_stdio::b0106s3 {

struct pbsd_shim_bufdesc {
	unsigned char *_base;
	int _size;
};

struct FILE {
	unsigned char *_p;
	int _r;
	int _w;
	short _flags;
	short _file;
	pbsd_shim_bufdesc _bf;
	int _lbfsize;
	int _orientation;
};

#define __SLBF	0x0001
#define __SNBF	0x0002
#define __SRD	0x0004
#define __SWR	0x0008
#define __SRW	0x0010
#define __SEOF	0x0020
#define __SERR	0x0040
#define __SMBF	0x0080
#define __SAPP	0x0100
#define __SSTR	0x0200

#define FAKE_FILE { ._file = -1 }

#define FIX_LOCALE(loc)							\
	do {								\
		if ((loc) == NULL)					\
			(loc) = __get_locale();				\
	} while (0)

typedef va_list __va_list;

extern "C" locale_t __get_locale(void);
extern "C" int __vfprintf(FILE *, locale_t, int, const char *, __va_list);

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Chris Torek.
 *
 * Copyright (c) 2011 The FreeBSD Foundation
 *
 * Portions of this software were developed by David Chisnall
 * under sponsorship from the FreeBSD Foundation.
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

int
vsprintf_l(char * __restrict str, locale_t locale,
    const char * __restrict fmt, __va_list ap)
{
	FILE f = FAKE_FILE;
	int serrno = errno;
	int ret;
	FIX_LOCALE(locale);

	f._flags = __SWR | __SSTR;
	f._bf._base = f._p = (unsigned char *)str;
	f._bf._size = f._w = INT_MAX;
	ret = __vfprintf(&f, locale, serrno, fmt, ap);
	*f._p = 0;
	return (ret);
}
int
vsprintf(char * __restrict str, const char * __restrict fmt, __va_list ap)
{
	return vsprintf_l(str, __get_locale(), fmt, ap);
}

} /* namespace pbsd::lib_libc_stdio::b0106s3 */
