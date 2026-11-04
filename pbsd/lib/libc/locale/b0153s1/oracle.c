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

/*
 * ORACLE: lib/libc/locale/localeconv.c with every function renamed with a
 * ref_ prefix.  Function bodies are byte-for-byte the originals.
 *
 * <locale.h> is deliberately NOT included: the host <locale.h> would collide
 * with the FreeBSD declarations of struct lconv and locale_t that the original
 * translation unit compiled against.  Those declarations, along with the ones
 * from "lmonetary.h", "lnumeric.h" and "xlocale_private.h", are supplied
 * below exactly as FreeBSD spells them.
 */

#include <stddef.h>

/* <locale.h> */
struct lconv {
	char	*decimal_point;
	char	*thousands_sep;
	char	*grouping;
	char	*int_curr_symbol;
	char	*currency_symbol;
	char	*mon_decimal_point;
	char	*mon_thousands_sep;
	char	*mon_grouping;
	char	*positive_sign;
	char	*negative_sign;
	char	int_frac_digits;
	char	frac_digits;
	char	p_cs_precedes;
	char	p_sep_by_space;
	char	n_cs_precedes;
	char	n_sep_by_space;
	char	p_sign_posn;
	char	n_sign_posn;
	char	int_p_cs_precedes;
	char	int_n_cs_precedes;
	char	int_p_sep_by_space;
	char	int_n_sep_by_space;
	char	int_p_sign_posn;
	char	int_n_sign_posn;
};

/* "lmonetary.h" */
struct lc_monetary_T {
	const char	*int_curr_symbol;
	const char	*currency_symbol;
	const char	*mon_decimal_point;
	const char	*mon_thousands_sep;
	const char	*mon_grouping;
	const char	*positive_sign;
	const char	*negative_sign;
	const char	*int_frac_digits;
	const char	*frac_digits;
	const char	*p_cs_precedes;
	const char	*p_sep_by_space;
	const char	*n_cs_precedes;
	const char	*n_sep_by_space;
	const char	*p_sign_posn;
	const char	*n_sign_posn;
	const char	*int_p_cs_precedes;
	const char	*int_p_sep_by_space;
	const char	*int_n_cs_precedes;
	const char	*int_n_sep_by_space;
	const char	*int_p_sign_posn;
	const char	*int_n_sign_posn;
};

/* "lnumeric.h" */
struct lc_numeric_T {
	const char	*decimal_point;
	const char	*thousands_sep;
	const char	*grouping;
};

/* "xlocale_private.h" */
struct _xlocale {
	struct lconv		 lconv;
	int			 monetary_locale_changed;
	int			 numeric_locale_changed;
	struct lc_monetary_T	*__mon;
	struct lc_numeric_T	*__num;
};

typedef struct _xlocale *locale_t;

/* Test-visible stand-in for the process/thread locale. */
struct _xlocale *__ref_current_locale = NULL;

static locale_t
__get_locale(void)
{
	return __ref_current_locale;
}

static struct lc_monetary_T *
__get_current_monetary_locale(locale_t loc)
{
	return loc->__mon;
}

static struct lc_numeric_T *
__get_current_numeric_locale(locale_t loc)
{
	return loc->__num;
}

/* <machine/atomic.h> */
#define atomic_load_acq_int(p)		__atomic_load_n((p), __ATOMIC_ACQUIRE)
#define atomic_store_rel_int(p, v)	__atomic_store_n((p), (v), __ATOMIC_RELEASE)

#define FIX_LOCALE(l)	do { if (NULL == l) l = __get_locale(); } while (0)

/* 
 * The localeconv() function constructs a struct lconv from the current
 * monetary and numeric locales.
 *
 * Because localeconv() may be called many times (especially by library
 * routines like printf() & strtod()), the appropriate members of the
 * lconv structure are computed only when the monetary or numeric 
 * locale has been changed.
 */

/*
 * Return the current locale conversion.
 */
struct lconv *
ref_localeconv_l(locale_t loc)
{
	FIX_LOCALE(loc);
    struct lconv *ret = &loc->lconv;

    if (atomic_load_acq_int(&loc->monetary_locale_changed) != 0) {
	/* LC_MONETARY part */
        struct lc_monetary_T * mptr; 

#define M_ASSIGN_STR(NAME) (ret->NAME = (char*)mptr->NAME)
#define M_ASSIGN_CHAR(NAME) (ret->NAME = mptr->NAME[0])

	mptr = __get_current_monetary_locale(loc);
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

	nptr = __get_current_numeric_locale(loc);
	N_ASSIGN_STR(decimal_point);
	N_ASSIGN_STR(thousands_sep);
	N_ASSIGN_STR(grouping);
	atomic_store_rel_int(&loc->numeric_locale_changed, 0);
    }

    return ret;
}
struct lconv *
ref_localeconv(void)
{
	return ref_localeconv_l(__get_locale());
}
