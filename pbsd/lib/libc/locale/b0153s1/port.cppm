/*
 * pbsd.lib.libc.locale.b0153s1 - C++23 port of batch b0153s1.
 *
 * Source:
 *	lib/libc/locale/localeconv.c
 */

module;

#include <locale.h>
#include <stdint.h>
#include <string.h>

export module pbsd.lib.libc.locale.b0153s1;

struct port_xlocale_component {
	long		retain_count;
	void		(*destructor)(void *);
	char		locale[32];
	char		version[12];
};

struct port_xlocale {
	long		retain_count;
	void		(*destructor)(void *);
	port_xlocale_component *components[6];
	int		monetary_locale_changed;
	int		using_monetary_locale;
	int		numeric_locale_changed;
	int		using_numeric_locale;
	int		using_time_locale;
	int		using_messages_locale;
	struct lconv	lconv;
	char		*csym;
};

typedef port_xlocale *port_locale_t;

namespace pbsd::lib_libc_locale::b0153s1 {

static port_xlocale port_test_locale;

static inline int
atomic_load_acq_int(volatile int *p)
{
	return (*p);
}

static inline void
atomic_store_rel_int(volatile int *p, int v)
{
	*p = v;
}

port_locale_t
__get_locale(void)
{
	return (&port_test_locale);
}

} /* namespace pbsd::lib_libc_locale::b0153s1 */

#define FIX_LOCALE(loc)	do {						\
	if ((loc) == NULL)						\
		(loc) = pbsd::lib_libc_locale::b0153s1::__get_locale();	\
} while (0)

extern "C" {
struct lc_monetary_T;
struct lc_numeric_T;
struct lc_monetary_T *__get_current_monetary_locale(locale_t);
struct lc_numeric_T *__get_current_numeric_locale(locale_t);
}

export namespace pbsd::lib_libc_locale::b0153s1 {

void
set_localeconv_flags(int mon, int num)
{
	port_test_locale.monetary_locale_changed = mon;
	port_test_locale.numeric_locale_changed = num;
}

port_locale_t
test_locale(void)
{
	return (&port_test_locale);
}

void
get_localeconv_flags(int *mon, int *num)
{
	*mon = port_test_locale.monetary_locale_changed;
	*num = port_test_locale.numeric_locale_changed;
}

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2001 Alexey Zelkin <phantom@FreeBSD.org>
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
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

struct lconv *
localeconv_l(port_locale_t loc)
{
	FIX_LOCALE(loc);
    struct lconv *ret = &loc->lconv;

    if (atomic_load_acq_int(&loc->monetary_locale_changed) != 0) {
	/* LC_MONETARY part */
        struct lc_monetary_T * mptr; 

#define M_ASSIGN_STR(NAME) (ret->NAME = (char*)mptr->NAME)
#define M_ASSIGN_CHAR(NAME) (ret->NAME = mptr->NAME[0])

	mptr = __get_current_monetary_locale((locale_t)loc);
	M_ASSIGN_STR(int_curr_symbol);
	M_ASSIGN_STR(currency_symbol);
	M_ASSIGN_STR(mon_decimal_point);
	M_ASSIGN_STR(mon_thousands_sep);
	M_ASSIGN_STR(mon_grouping);
	M_ASSIGN_STR(positive_sign);
	M_ASSIGN_STR(negative_sign);
	M_ASSIGN_CHAR(int_frac_digits);
	M_ASSIGN_CHAR(frac_digits);
	M_ASSIGN_CHAR(p_cs_precedes);
	M_ASSIGN_CHAR(p_sep_by_space);
	M_ASSIGN_CHAR(n_cs_precedes);
	M_ASSIGN_CHAR(n_sep_by_space);
	M_ASSIGN_CHAR(p_sign_posn);
	M_ASSIGN_CHAR(n_sign_posn);
	M_ASSIGN_CHAR(int_p_cs_precedes);
	M_ASSIGN_CHAR(int_n_cs_precedes);
	M_ASSIGN_CHAR(int_p_sep_by_space);
	M_ASSIGN_CHAR(int_n_sep_by_space);
	M_ASSIGN_CHAR(int_p_sign_posn);
	M_ASSIGN_CHAR(int_n_sign_posn);
	atomic_store_rel_int(&loc->monetary_locale_changed, 0);
    }

    if (atomic_load_acq_int(&loc->numeric_locale_changed) != 0) {
	/* LC_NUMERIC part */
        struct lc_numeric_T * nptr; 

#define N_ASSIGN_STR(NAME) (ret->NAME = (char*)nptr->NAME)

	nptr = __get_current_numeric_locale((locale_t)loc);
	N_ASSIGN_STR(decimal_point);
	N_ASSIGN_STR(thousands_sep);
	N_ASSIGN_STR(grouping);
	atomic_store_rel_int(&loc->numeric_locale_changed, 0);
    }

    return ret;
}
struct lconv *
localeconv(void)
{
	return localeconv_l(__get_locale());
}

} /* export namespace pbsd::lib_libc_locale::b0153s1 */
