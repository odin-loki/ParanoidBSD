/*
 * PBSD batch b0317 -- C++23 port of
 *     lib/libc/stdio/tempnam.c
 *     lib/libc/stdio/vsnprintf.c
 *     lib/libc/stdio/xprintf_quote.c
 *
 * Faithful translation: behaviour, signedness, evaluation order and pointer
 * arithmetic are preserved exactly.  Original copyright headers are retained
 * with each function.
 */

module;

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <cassert>
#include <cctype>
#include <cerrno>
#include <climits>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdarg.h>

export module pbsd.lib.libc.stdio.b0317;

export namespace pbsd::lib_libc_stdio::b0317 {

#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif

#ifndef P_tmpdir
#define P_tmpdir "/tmp/"
#endif

#ifndef _PATH_TMP
#define _PATH_TMP "/tmp/"
#endif

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

#define FAKE_FILE { ._file = -1 }

#define FIX_LOCALE(loc)							\
	do {								\
		if ((loc) == NULL)					\
			(loc) = __get_locale();				\
	} while (0)

typedef va_list __va_list;
typedef void *locale_t;

export struct printf_info {
	int prec;
	int width;
	int pad;
	int showsign;
	int alt;
};

export struct __printf_io {
	int _dummy;
};

#define PA_POINTER	5

extern "C" {
extern locale_t __get_locale(void);
extern int __vfprintf(FILE *, locale_t, int, const char *, __va_list);
extern char *secure_getenv(const char *);
extern char *_mktemp(char *);
extern int __printf_out(struct __printf_io *, const struct printf_info *,
    const char *, int);
extern void __printf_flush(struct __printf_io *);
extern void *malloc(size_t);
extern void free(void *);
}

/* ====================================================================== */
/* lib/libc/stdio/tempnam.c                                               */
/* ====================================================================== */

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1988, 1993
 *	The Regents of the University of California.  All rights reserved.
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

char *
tempnam(const char *dir, const char *pfx)
{
	int sverrno;
	char *f, *name;

	if (!(name = (char *)::malloc(MAXPATHLEN)))
		return(NULL);

	if (!pfx)
		pfx = "tmp.";

	if ((f = secure_getenv("TMPDIR")) != NULL) {
		(void)snprintf(name, MAXPATHLEN, "%s%s%sXXXXXX", f,
		    *(f + strlen(f) - 1) == '/'? "": "/", pfx);
		if ((f = _mktemp(name)))
			return(f);
	}

	if ((f = (char *)dir)) {
		(void)snprintf(name, MAXPATHLEN, "%s%s%sXXXXXX", f,
		    *(f + strlen(f) - 1) == '/'? "": "/", pfx);
		if ((f = _mktemp(name)))
			return(f);
	}

	f = P_tmpdir;
	(void)snprintf(name, MAXPATHLEN, "%s%sXXXXXX", f, pfx);
	if ((f = _mktemp(name)))
		return(f);

	f = _PATH_TMP;
	(void)snprintf(name, MAXPATHLEN, "%s%sXXXXXX", f, pfx);
	if ((f = _mktemp(name)))
		return(f);

	sverrno = errno;
	::free(name);
	errno = sverrno;
	return(NULL);
}

/* ====================================================================== */
/* lib/libc/stdio/vsnprintf.c                                             */
/* ====================================================================== */

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
vsnprintf_l(char * __restrict str, size_t n, locale_t locale, 
    const char * __restrict fmt, __va_list ap)
{
	FILE f = FAKE_FILE;
	size_t on;
	int serrno = errno;
	int ret;
	char dummy[2];
	FIX_LOCALE(locale);

	on = n;
	if (n != 0)
		n--;
	if (n > INT_MAX) {
		errno = EOVERFLOW;
		*str = '\0';
		return (EOF);
	}
	/* Stdio internals do not deal correctly with zero length buffer */
	if (n == 0) {
		if (on > 0)
	  		*str = '\0';
		str = dummy;
		n = 1;
	}
	f._flags = __SWR | __SSTR;
	f._bf._base = f._p = (unsigned char *)str;
	f._bf._size = f._w = n;
	ret = __vfprintf(&f, locale, serrno, fmt, ap);
	if (on > 0)
		*f._p = '\0';
	return (ret);
}
int
vsnprintf(char * __restrict str, size_t n, const char * __restrict fmt,
    __va_list ap)
{
	return vsnprintf_l(str, n, __get_locale(), fmt, ap);
}

/* ====================================================================== */
/* lib/libc/stdio/xprintf_quote.c                                         */
/* ====================================================================== */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2005 Poul-Henning Kamp
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
__printf_arginfo_quote(const struct printf_info *pi __unused, size_t n, int *argt)
{

	assert(n >= 1);
	argt[0] = PA_POINTER;
	return (1);
}

int
__printf_render_quote(struct __printf_io *io, const struct printf_info *pi __unused, const void *const *arg)
{
	const char *str, *p, *t, *o;
	char r[5];
	int i, ret;

	str = *((const char *const *)arg[0]);
	if (str == NULL)
		return (__printf_out(io, pi, "\"(null)\"", 8));
	if (*str == '\0')
		return (__printf_out(io, pi, "\"\"", 2));

	for (i = 0, p = str; *p; p++)
		if (isspace(*p) || *p == '\\' || *p == '"')
			i++;
	if (!i) 
		return (__printf_out(io, pi, str, strlen(str)));
	
	ret = __printf_out(io, pi, "\"", 1);
	for (t = p = str; *p; p++) {
		o = NULL;
		if (*p == '\\')
			o = "\\\\";
		else if (*p == '\n')
			o = "\\n";
		else if (*p == '\r')
			o = "\\r";
		else if (*p == '\t')
			o = "\\t";
		else if (*p == ' ')
			o = " ";
		else if (*p == '"')
			o = "\\\"";
		else if (isspace(*p)) {
			sprintf(r, "\\%03o", *p);
			o = r;
		} else
			continue;
		if (p != t)
			ret += __printf_out(io, pi, t, p - t);
		ret += __printf_out(io, pi, o, strlen(o));
		t = p + 1;
	}
	if (p != t)
		ret += __printf_out(io, pi, t, p - t);
	ret += __printf_out(io, pi, "\"", 1);
	__printf_flush(io);
	return(ret);
}

} /* namespace pbsd::lib_libc_stdio::b0317 */
