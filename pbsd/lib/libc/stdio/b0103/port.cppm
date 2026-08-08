module;

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <cstdarg>
#include <cstdio>
#include <cwchar>
#include <clocale>

#if defined(__has_include)
#if __has_include(<xlocale.h>)
#include <xlocale.h>
#endif
#elif defined(__FreeBSD__) || defined(__APPLE__)
#include <xlocale.h>
#endif

#ifndef _XLOCALE_H_
#ifndef vfwscanf_l
static int
vfwscanf_l(std::FILE *stream, ::locale_t loc, const wchar_t *fmt, std::va_list ap)
{
	::locale_t old = uselocale(loc);
	int r = ::vfwscanf(stream, fmt, ap);

	uselocale(old);
	return r;
}
#endif

#ifndef vfwprintf_l
static int
vfwprintf_l(std::FILE *stream, ::locale_t loc, const wchar_t *fmt, std::va_list ap)
{
	::locale_t old = uselocale(loc);
	int r = ::vfwprintf(stream, fmt, ap);

	uselocale(old);
	return r;
}
#endif

#ifndef vswscanf_l
static int
vswscanf_l(const wchar_t *str, ::locale_t loc, const wchar_t *fmt, std::va_list ap)
{
	::locale_t old = uselocale(loc);
	int r = ::vswscanf(str, fmt, ap);

	uselocale(old);
	return r;
}
#endif
#endif

export module pbsd.lib.libc.stdio.b0103;

export namespace pbsd::lib_libc_stdio::b0103 {

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
fwscanf(FILE * __restrict fp, const wchar_t * __restrict fmt, ...)
{
	std::va_list ap;
	int r;

	std::va_start(ap, fmt);
	r = vfwscanf(fp, fmt, ap);
	std::va_end(ap);

	return (r);
}
int
fwscanf_l(FILE * __restrict fp, locale_t locale, const wchar_t * __restrict fmt, ...)
{
	std::va_list ap;
	int r;

	std::va_start(ap, fmt);
	r = vfwscanf_l(fp, locale, fmt, ap);
	std::va_end(ap);

	return (r);
}

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
fwprintf(FILE * __restrict fp, const wchar_t * __restrict fmt, ...)
{
	int ret;
	std::va_list ap;

	std::va_start(ap, fmt);
	ret = vfwprintf(fp, fmt, ap);
	std::va_end(ap);

	return (ret);
}
int
fwprintf_l(FILE * __restrict fp, locale_t locale, const wchar_t * __restrict fmt, ...)
{
	int ret;
	std::va_list ap;

	std::va_start(ap, fmt);
	ret = vfwprintf_l(fp, locale, fmt, ap);
	std::va_end(ap);

	return (ret);
}

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
swscanf(const wchar_t * __restrict str, const wchar_t * __restrict fmt, ...)
{
	std::va_list ap;
	int r;

	std::va_start(ap, fmt);
	r = vswscanf(str, fmt, ap);
	std::va_end(ap);

	return (r);
}
int
swscanf_l(const wchar_t * __restrict str, locale_t locale,
		const wchar_t * __restrict fmt, ...)
{
	std::va_list ap;
	int r;

	std::va_start(ap, fmt);
	r = vswscanf_l(str, locale, fmt, ap);
	std::va_end(ap);

	return (r);
}

} /* namespace pbsd::lib_libc_stdio::b0103 */
