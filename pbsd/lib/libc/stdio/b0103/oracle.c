/*
 * b0103 oracle -- the specification.
 *
 * hbsd/src/lib/libc/stdio/fwscanf.c, fwprintf.c, and swscanf.c concatenated,
 * with every function renamed with a `ref_' prefix.  Function bodies are
 * UNMODIFIED.
 */

#define _GNU_SOURCE

#include <stddef.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <wchar.h>

#if defined(__has_include)
#if __has_include(<xlocale.h>)
#include <xlocale.h>
#endif
#elif defined(__FreeBSD__) || defined(__APPLE__)
#include <xlocale.h>
#endif

#include <locale.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * CHAR_BIT)
#endif

#ifndef _XLOCALE_H_
#ifndef vfwscanf_l
static int
vfwscanf_l(FILE *stream, locale_t loc, const wchar_t *fmt, va_list ap)
{
	locale_t old = uselocale(loc);
	int r = vfwscanf(stream, fmt, ap);

	uselocale(old);
	return r;
}
#endif

#ifndef vfwprintf_l
static int
vfwprintf_l(FILE *stream, locale_t loc, const wchar_t *fmt, va_list ap)
{
	locale_t old = uselocale(loc);
	int r = vfwprintf(stream, fmt, ap);

	uselocale(old);
	return r;
}
#endif

#ifndef vswscanf_l
static int
vswscanf_l(const wchar_t *str, locale_t loc, const wchar_t *fmt, va_list ap)
{
	locale_t old = uselocale(loc);
	int r = vswscanf(str, fmt, ap);

	uselocale(old);
	return r;
}
#endif
#endif

/* ======================= fwscanf.c ======================= */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2002 Tim J. Robbins
 * All rights reserved.
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
ref_fwscanf(FILE * __restrict fp, const wchar_t * __restrict fmt, ...)
{
	va_list ap;
	int r;

	va_start(ap, fmt);
	r = vfwscanf(fp, fmt, ap);
	va_end(ap);

	return (r);
}
int
ref_fwscanf_l(FILE * __restrict fp, locale_t locale, const wchar_t * __restrict fmt, ...)
{
	va_list ap;
	int r;

	va_start(ap, fmt);
	r = vfwscanf_l(fp, locale, fmt, ap);
	va_end(ap);

	return (r);
}

/* ======================= fwprintf.c ======================= */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2002 Tim J. Robbins
 * All rights reserved.
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
ref_fwprintf(FILE * __restrict fp, const wchar_t * __restrict fmt, ...)
{
	int ret;
	va_list ap;

	va_start(ap, fmt);
	ret = vfwprintf(fp, fmt, ap);
	va_end(ap);

	return (ret);
}
int
ref_fwprintf_l(FILE * __restrict fp, locale_t locale, const wchar_t * __restrict fmt, ...)
{
	int ret;
	va_list ap;

	va_start(ap, fmt);
	ret = vfwprintf_l(fp, locale, fmt, ap);
	va_end(ap);

	return (ret);
}

/* ======================= swscanf.c ======================= */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2002 Tim J. Robbins
 * All rights reserved.
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
ref_swscanf(const wchar_t * __restrict str, const wchar_t * __restrict fmt, ...)
{
	va_list ap;
	int r;

	va_start(ap, fmt);
	r = vswscanf(str, fmt, ap);
	va_end(ap);

	return (r);
}
int
ref_swscanf_l(const wchar_t * __restrict str, locale_t locale,
		const wchar_t * __restrict fmt, ...)
{
	va_list ap;
	int r;

	va_start(ap, fmt);
	r = vswscanf_l(str, locale, fmt, ap);
	va_end(ap);

	return (r);
}
