/*
 * port.cppm -- PBSD batch b0064
 *
 * C++23 module port of the HardenedBSD source
 *
 *	hbsd/src/lib/libc/locale/mbsinit.c
 *
 * The translation is deliberately literal: control flow, integer types,
 * signedness, evaluation order and pointer arithmetic are preserved
 * exactly as in the C original.  Nothing has been "improved".
 *
 * Private libc headers (mblocal.h, xlocale_private.h) are not part of this
 * batch.  The declarations and mock locale substrate below stand in for them.
 */

module;

#include <cstdint>

namespace {

typedef union {
	char		__mbstate8[128];
	long long	_mbstateL;
} pbsd_mbstate_t;

struct pbsd_xlocale_ctype {
	int		(*__mbsinit)(const pbsd_mbstate_t *);
};

struct pbsd_locale {
	void		*components[6];
};

using pbsd_locale_t = pbsd_locale *;

enum {
	XLC_CTYPE = 1,
};

#define XLOCALE_CTYPE(x)	((struct pbsd_xlocale_ctype *)(x)->components[XLC_CTYPE])

pbsd_xlocale_ctype	port_c_ctype;
pbsd_locale		port_c_locale;
pbsd_xlocale_ctype	port_global_ctype;
pbsd_locale		port_global_locale;

int			port_has_thread_locale;
pbsd_locale_t		port_thread_locale;

static inline pbsd_locale_t
get_real_locale(pbsd_locale_t locale)
{

	switch ((intptr_t)locale) {
	case 0:
		return (&port_c_locale);
	case -1:
		return (&port_global_locale);
	default:
		return (locale);
	}
}

#define FIX_LOCALE(l)	(l = get_real_locale(l))

static inline pbsd_locale_t
__get_locale(void)
{

	if (!port_has_thread_locale || port_thread_locale == NULL)
		return (&port_global_locale);
	return (port_thread_locale);
}

} // namespace

export module pbsd.lib.libc.locale.b0064;

export namespace pbsd::lib_libc_locale::b0064 {

using pbsd_mbstate_t = ::pbsd_mbstate_t;
using pbsd_locale = ::pbsd_locale;
using pbsd_locale_t = ::pbsd_locale_t;

extern "C" {

void
pbsd_set_thread_locale(int on, pbsd_locale_t loc)
{

	port_has_thread_locale = on;
	port_thread_locale = loc;
}

void
pbsd_locale_init(void)
{

	port_c_locale.components[XLC_CTYPE] = &port_c_ctype;
	port_global_locale.components[XLC_CTYPE] = &port_global_ctype;
}

pbsd_locale_t
pbsd_c_locale(void)
{

	return (&port_c_locale);
}

pbsd_locale_t
pbsd_global_locale(void)
{

	return (&port_global_locale);
}

} // extern "C"

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2002-2004 Tim J. Robbins.
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
mbsinit_l(const pbsd_mbstate_t *ps, pbsd_locale_t locale)
{
	FIX_LOCALE(locale);
	return (XLOCALE_CTYPE(locale)->__mbsinit(ps));
}
int
mbsinit(const pbsd_mbstate_t *ps)
{
	return mbsinit_l(ps, __get_locale());
}

using ::pbsd_set_thread_locale;
using ::pbsd_locale_init;
using ::pbsd_c_locale;
using ::pbsd_global_locale;

} // export namespace pbsd::lib_libc_locale::b0064
