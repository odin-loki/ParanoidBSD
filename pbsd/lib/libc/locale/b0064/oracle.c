/*
 * oracle.c -- reference implementation for PBSD batch b0064.
 *
 * Original HardenedBSD source (mbsinit.c) concatenated verbatim.  Every
 * function has been renamed with a ref_ prefix; intra-batch call sites were
 * updated to match.  No function body has been altered in any other way.
 *
 * Private libc headers (mblocal.h, xlocale_private.h) are not part of this
 * batch.  The declarations and mock locale substrate below stand in for them
 * so the unmodified function bodies compile and link.  FreeBSD uses a
 * 128-byte mbstate_t; glibc's is smaller, so __mbstate_t is defined before
 * <wchar.h> to match FreeBSD.
 */

#define __mbstate_t_defined 1
typedef union {
	char		__mbstate8[128];
	long long	_mbstateL;
} __mbstate_t;
typedef __mbstate_t mbstate_t;

#include <stdint.h>
#include <wchar.h>

typedef struct xlocale *locale_t;

enum {
	XLC_CTYPE = 1,
};

struct xlocale_ctype {
	int		(*__mbsinit)(const mbstate_t *);
};

struct xlocale {
	void		*components[6];
};

#define XLOCALE_CTYPE(x)	((struct xlocale_ctype *)(x)->components[XLC_CTYPE])

struct xlocale_ctype	ref_c_ctype;
struct xlocale		ref_c_locale;
struct xlocale_ctype	ref_global_ctype;
struct xlocale		ref_global_locale;

static int		ref_has_thread_locale;
static locale_t		ref_thread_locale;

static inline locale_t
get_real_locale(locale_t locale)
{

	switch ((intptr_t)locale) {
	case 0:
		return (&ref_c_locale);
	case -1:
		return (&ref_global_locale);
	default:
		return (locale);
	}
}

#define FIX_LOCALE(l)	(l = get_real_locale(l))

static inline locale_t
__get_locale(void)
{

	if (!ref_has_thread_locale || ref_thread_locale == NULL)
		return (&ref_global_locale);
	return (ref_thread_locale);
}

void
ref_set_thread_locale(int on, locale_t loc)
{

	ref_has_thread_locale = on;
	ref_thread_locale = loc;
}

void
ref_locale_init(void)
{

	ref_c_locale.components[XLC_CTYPE] = &ref_c_ctype;
	ref_global_locale.components[XLC_CTYPE] = &ref_global_ctype;
}

locale_t
ref_c_locale_ptr(void)
{

	return (&ref_c_locale);
}

locale_t
ref_global_locale_ptr(void)
{

	return (&ref_global_locale);
}

/* ------------------------------------------------------------------ */
/* hbsd/src/lib/libc/locale/mbsinit.c					*/
/* ------------------------------------------------------------------ */

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
ref_mbsinit_l(const mbstate_t *ps, locale_t locale)
{
	FIX_LOCALE(locale);
	return (XLOCALE_CTYPE(locale)->__mbsinit(ps));
}
int
ref_mbsinit(const mbstate_t *ps)
{
	return ref_mbsinit_l(ps, __get_locale());
}
