/*
 * PBSD batch b0153s1 -- reference oracle.
 *
 * Original HardenedBSD source concatenated, every function renamed with
 * a ref_ prefix.  Function bodies are UNMODIFIED except for internal static
 * call targets renamed to match.
 *
 * Source:
 *   hbsd/src/lib/libc/locale/localeconv.c
 */

#ifndef LONG_BIT
#define LONG_BIT	(sizeof(long) * CHAR_BIT)
#endif

#include <limits.h>
#include <locale.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

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
	const char	*int_n_cs_precedes;
	const char	*int_p_sep_by_space;
	const char	*int_n_sep_by_space;
	const char	*int_p_sign_posn;
	const char	*int_n_sign_posn;
};

struct lc_numeric_T {
	const char	*decimal_point;
	const char	*thousands_sep;
	const char	*grouping;
};

struct xlocale_component {
	long		retain_count;
	void		(*destructor)(void *);
	char		locale[32];
	char		version[12];
};

struct _xlocale {
	long		retain_count;
	void		(*destructor)(void *);
	struct xlocale_component *components[6];
	int		monetary_locale_changed;
	int		using_monetary_locale;
	int		numeric_locale_changed;
	int		using_numeric_locale;
	int		using_time_locale;
	int		using_messages_locale;
	struct lconv	lconv;
	char		*csym;
};

typedef struct _xlocale *locale_t;

#define FIX_LOCALE(loc)		if ((loc) == NULL) (loc) = ref___get_locale()
#define __get_locale()		ref___get_locale()

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

typedef struct {
	char		m_int_curr_symbol[16];
	char		m_currency_symbol[16];
	char		m_mon_decimal_point[16];
	char		m_mon_thousands_sep[16];
	char		m_mon_grouping[16];
	char		m_positive_sign[16];
	char		m_negative_sign[16];
	char		m_int_frac_digits[2];
	char		m_frac_digits[2];
	char		m_p_cs_precedes[2];
	char		m_p_sep_by_space[2];
	char		m_n_cs_precedes[2];
	char		m_n_sep_by_space[2];
	char		m_p_sign_posn[2];
	char		m_n_sign_posn[2];
	char		m_int_p_cs_precedes[2];
	char		m_int_n_cs_precedes[2];
	char		m_int_p_sep_by_space[2];
	char		m_int_n_sep_by_space[2];
	char		m_int_p_sign_posn[2];
	char		m_int_n_sign_posn[2];
	char		n_decimal_point[16];
	char		n_thousands_sep[16];
	char		n_grouping[16];
	struct lc_monetary_T	monetary;
	struct lc_numeric_T	numeric;
} pbsd_localeconv_data_t;

typedef struct {
	pbsd_localeconv_data_t	data;
} pbsd_localeconv_hook_t;

pbsd_localeconv_hook_t	pbsd_localeconv_hook;

struct _xlocale	ref_test_locale;

static void
pbsd_localeconv_wire_pointers(pbsd_localeconv_data_t *d)
{
	d->monetary.int_curr_symbol = d->m_int_curr_symbol;
	d->monetary.currency_symbol = d->m_currency_symbol;
	d->monetary.mon_decimal_point = d->m_mon_decimal_point;
	d->monetary.mon_thousands_sep = d->m_mon_thousands_sep;
	d->monetary.mon_grouping = d->m_mon_grouping;
	d->monetary.positive_sign = d->m_positive_sign;
	d->monetary.negative_sign = d->m_negative_sign;
	d->monetary.int_frac_digits = d->m_int_frac_digits;
	d->monetary.frac_digits = d->m_frac_digits;
	d->monetary.p_cs_precedes = d->m_p_cs_precedes;
	d->monetary.p_sep_by_space = d->m_p_sep_by_space;
	d->monetary.n_cs_precedes = d->m_n_cs_precedes;
	d->monetary.n_sep_by_space = d->m_n_sep_by_space;
	d->monetary.p_sign_posn = d->m_p_sign_posn;
	d->monetary.n_sign_posn = d->m_n_sign_posn;
	d->monetary.int_p_cs_precedes = d->m_int_p_cs_precedes;
	d->monetary.int_n_cs_precedes = d->m_int_n_cs_precedes;
	d->monetary.int_p_sep_by_space = d->m_int_p_sep_by_space;
	d->monetary.int_n_sep_by_space = d->m_int_n_sep_by_space;
	d->monetary.int_p_sign_posn = d->m_int_p_sign_posn;
	d->monetary.int_n_sign_posn = d->m_int_n_sign_posn;
	d->numeric.decimal_point = d->n_decimal_point;
	d->numeric.thousands_sep = d->n_thousands_sep;
	d->numeric.grouping = d->n_grouping;
}

void
pbsd_reset_hooks(void)
{
	pbsd_localeconv_data_t *d = &pbsd_localeconv_hook.data;

	memset(d, 0, sizeof(*d));
	strcpy(d->m_int_curr_symbol, "USD ");
	strcpy(d->m_currency_symbol, "$");
	strcpy(d->m_mon_decimal_point, ".");
	strcpy(d->m_mon_thousands_sep, ",");
	strcpy(d->m_mon_grouping, "\3");
	strcpy(d->m_positive_sign, "");
	strcpy(d->m_negative_sign, "-");
	d->m_int_frac_digits[0] = '2';
	d->m_int_frac_digits[1] = '\0';
	d->m_frac_digits[0] = '2';
	d->m_frac_digits[1] = '\0';
	d->m_p_cs_precedes[0] = '\1';
	d->m_p_cs_precedes[1] = '\0';
	d->m_p_sep_by_space[0] = '\0';
	d->m_p_sep_by_space[1] = '\0';
	d->m_n_cs_precedes[0] = '\1';
	d->m_n_cs_precedes[1] = '\0';
	d->m_n_sep_by_space[0] = '\0';
	d->m_n_sep_by_space[1] = '\0';
	d->m_p_sign_posn[0] = '\1';
	d->m_p_sign_posn[1] = '\0';
	d->m_n_sign_posn[0] = '\1';
	d->m_n_sign_posn[1] = '\0';
	d->m_int_p_cs_precedes[0] = '\1';
	d->m_int_p_cs_precedes[1] = '\0';
	d->m_int_n_cs_precedes[0] = '\1';
	d->m_int_n_cs_precedes[1] = '\0';
	d->m_int_p_sep_by_space[0] = '\0';
	d->m_int_p_sep_by_space[1] = '\0';
	d->m_int_n_sep_by_space[0] = '\0';
	d->m_int_n_sep_by_space[1] = '\0';
	d->m_int_p_sign_posn[0] = '\1';
	d->m_int_p_sign_posn[1] = '\0';
	d->m_int_n_sign_posn[0] = '\1';
	d->m_int_n_sign_posn[1] = '\0';
	strcpy(d->n_decimal_point, ".");
	strcpy(d->n_thousands_sep, "");
	strcpy(d->n_grouping, "");
	pbsd_localeconv_wire_pointers(d);
	memset(&ref_test_locale, 0, sizeof(ref_test_locale));
}

pbsd_localeconv_data_t *
pbsd_get_localeconv_data(void)
{
	return (&pbsd_localeconv_hook.data);
}

static void __attribute__((constructor))
ref_oracle_init(void)
{
	pbsd_reset_hooks();
}

void
ref_set_localeconv_flags(int mon, int num)
{
	ref_test_locale.monetary_locale_changed = mon;
	ref_test_locale.numeric_locale_changed = num;
}

struct _xlocale *
ref_get_test_locale(void)
{
	return (&ref_test_locale);
}

void
ref_get_localeconv_flags(int *mon, int *num)
{
	*mon = ref_test_locale.monetary_locale_changed;
	*num = ref_test_locale.numeric_locale_changed;
}

locale_t
ref___get_locale(void)
{
	return (&ref_test_locale);
}

struct lc_monetary_T *
__get_current_monetary_locale(locale_t loc)
{
	(void)loc;
	return ((struct lc_monetary_T *)&pbsd_localeconv_hook.data.monetary);
}

struct lc_numeric_T *
__get_current_numeric_locale(locale_t loc)
{
	(void)loc;
	return ((struct lc_numeric_T *)&pbsd_localeconv_hook.data.numeric);
}

int
ref_lconv_equal(const struct lconv *a, const struct lconv *b)
{
#define CMPF(F) (a->F == b->F || (a->F != NULL && b->F != NULL && \
    strcmp(a->F, b->F) == 0))
	if (!CMPF(decimal_point) || !CMPF(thousands_sep) || !CMPF(grouping) ||
	    !CMPF(int_curr_symbol) || !CMPF(currency_symbol) ||
	    !CMPF(mon_decimal_point) || !CMPF(mon_thousands_sep) ||
	    !CMPF(mon_grouping) || !CMPF(positive_sign) || !CMPF(negative_sign))
		return (0);
#undef CMPF
	return (a->int_frac_digits == b->int_frac_digits &&
	    a->frac_digits == b->frac_digits &&
	    a->p_cs_precedes == b->p_cs_precedes &&
	    a->p_sep_by_space == b->p_sep_by_space &&
	    a->n_cs_precedes == b->n_cs_precedes &&
	    a->n_sep_by_space == b->n_sep_by_space &&
	    a->p_sign_posn == b->p_sign_posn &&
	    a->n_sign_posn == b->n_sign_posn &&
	    a->int_p_cs_precedes == b->int_p_cs_precedes &&
	    a->int_n_cs_precedes == b->int_n_cs_precedes &&
	    a->int_p_sep_by_space == b->int_p_sep_by_space &&
	    a->int_n_sep_by_space == b->int_n_sep_by_space &&
	    a->int_p_sign_posn == b->int_p_sign_posn &&
	    a->int_n_sign_posn == b->int_n_sign_posn);
}

/* localeconv.c */
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
