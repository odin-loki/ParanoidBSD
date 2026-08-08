/*
 * Differential harness for batch b0153s1 (localeconv.c).
 */

import pbsd.lib.libc.locale.b0153s1;

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <locale.h>

namespace P = pbsd::lib_libc_locale::b0153s1;

extern "C" {
struct ref_xlocale {
	long		retain_count;
	void		(*destructor)(void *);
	void		*components[6];
	int		monetary_locale_changed;
	int		using_monetary_locale;
	int		numeric_locale_changed;
	int		using_numeric_locale;
	int		using_time_locale;
	int		using_messages_locale;
	struct lconv	lconv;
	char		*csym;
};

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
} pbsd_localeconv_data_t;

void		pbsd_reset_hooks(void);
pbsd_localeconv_data_t	*pbsd_get_localeconv_data(void);
void		ref_set_localeconv_flags(int mon, int num);
void		ref_get_localeconv_flags(int *mon, int *num);
struct ref_xlocale	*ref_get_test_locale(void);
int		ref_lconv_equal(const struct lconv *, const struct lconv *);
struct lconv	*ref_localeconv_l(struct ref_xlocale *);
struct lconv	*ref_localeconv(void);
}

enum {
	F_LOCALECONV_L, F_LOCALECONV, F_COUNT
};

static const char *const fname[F_COUNT] = {
	"localeconv_l", "localeconv"
};

static long long ncase[F_COUNT];
static long long nfail[F_COUNT];
static int nprinted[F_COUNT];

static constexpr long long SWEEP = 200000;
static uint64_t rng = 0xB0153S01ULL;

static uint64_t
rnd(void)
{
	rng ^= rng << 13;
	rng ^= rng >> 7;
	rng ^= rng << 17;
	return (rng);
}

static uint32_t
u32(uint32_t m)
{
	return ((uint32_t)(rnd() % m));
}

static void
report(int f, const char *why)
{
	nfail[f]++;
	if (nprinted[f]++ < 8)
		std::printf("  FAIL %-22s : %s\n", fname[f], why);
}

static void
case_inc(int f)
{
	ncase[f]++;
}

static bool
lconv_eq(const struct lconv *a, const struct lconv *b)
{
	return (ref_lconv_equal(a, b) != 0);
}

static void
rnd_fill(char *buf, size_t cap, bool allow_empty)
{
	size_t n;

	if (cap == 0)
		return;
	if (allow_empty)
		n = (size_t)u32((uint32_t)cap);
	else
		n = 1 + (size_t)u32((uint32_t)cap - 1);
	for (size_t i = 0; i < n; i++)
		buf[i] = (char)(rnd() & 0xffu);
	buf[n] = '\0';
}

static void
rnd_char_field(char *buf)
{
	buf[0] = (char)(rnd() & 0xffu);
	buf[1] = '\0';
}

static void
randomize_localeconv_data(pbsd_localeconv_data_t *d)
{
	rnd_fill(d->m_int_curr_symbol, sizeof(d->m_int_curr_symbol), true);
	rnd_fill(d->m_currency_symbol, sizeof(d->m_currency_symbol), true);
	rnd_fill(d->m_mon_decimal_point, sizeof(d->m_mon_decimal_point), true);
	rnd_fill(d->m_mon_thousands_sep, sizeof(d->m_mon_thousands_sep), true);
	rnd_fill(d->m_mon_grouping, sizeof(d->m_mon_grouping), true);
	rnd_fill(d->m_positive_sign, sizeof(d->m_positive_sign), true);
	rnd_fill(d->m_negative_sign, sizeof(d->m_negative_sign), true);
	rnd_char_field(d->m_int_frac_digits);
	rnd_char_field(d->m_frac_digits);
	rnd_char_field(d->m_p_cs_precedes);
	rnd_char_field(d->m_p_sep_by_space);
	rnd_char_field(d->m_n_cs_precedes);
	rnd_char_field(d->m_n_sep_by_space);
	rnd_char_field(d->m_p_sign_posn);
	rnd_char_field(d->m_n_sign_posn);
	rnd_char_field(d->m_int_p_cs_precedes);
	rnd_char_field(d->m_int_n_cs_precedes);
	rnd_char_field(d->m_int_p_sep_by_space);
	rnd_char_field(d->m_int_n_sep_by_space);
	rnd_char_field(d->m_int_p_sign_posn);
	rnd_char_field(d->m_int_n_sign_posn);
	rnd_fill(d->n_decimal_point, sizeof(d->n_decimal_point), true);
	rnd_fill(d->n_thousands_sep, sizeof(d->n_thousands_sep), true);
	rnd_fill(d->n_grouping, sizeof(d->n_grouping), true);
}

static void
prep_flags(int mon, int num)
{
	ref_set_localeconv_flags(mon, num);
	P::set_localeconv_flags(mon, num);
}

static bool
run_localeconv_l(int f, P::port_locale_t ploc, struct ref_xlocale *rloc)
{
	struct lconv *pr, *rr;
	std::ptrdiff_t po, ro;
	int pm, pn, rm, rn;

	case_inc(f);
	pr = P::localeconv_l(ploc);
	rr = ref_localeconv_l(rloc);
	if (pr == nullptr || rr == nullptr) {
		report(f, "null return");
		return (false);
	}
	po = reinterpret_cast<char *>(pr) -
	    reinterpret_cast<char *>(P::test_locale());
	ro = reinterpret_cast<char *>(rr) -
	    reinterpret_cast<char *>(ref_get_test_locale());
	if (po != ro) {
		report(f, "return offset");
		return (false);
	}
	if (!lconv_eq(pr, rr)) {
		report(f, "lconv fields");
		return (false);
	}
	P::get_localeconv_flags(&pm, &pn);
	ref_get_localeconv_flags(&rm, &rn);
	if (pm != rm || pn != rn) {
		report(f, "changed flags");
		return (false);
	}
	return (true);
}

static bool
run_localeconv(int f)
{
	struct lconv *pr, *rr;
	std::ptrdiff_t po, ro;
	int pm, pn, rm, rn;

	case_inc(f);
	pr = P::localeconv();
	rr = ref_localeconv();
	if (pr == nullptr || rr == nullptr) {
		report(f, "null return");
		return (false);
	}
	po = reinterpret_cast<char *>(pr) -
	    reinterpret_cast<char *>(P::test_locale());
	ro = reinterpret_cast<char *>(rr) -
	    reinterpret_cast<char *>(ref_get_test_locale());
	if (po != ro) {
		report(f, "return offset");
		return (false);
	}
	if (!lconv_eq(pr, rr)) {
		report(f, "lconv fields");
		return (false);
	}
	P::get_localeconv_flags(&pm, &pn);
	ref_get_localeconv_flags(&rm, &rn);
	if (pm != rm || pn != rn) {
		report(f, "changed flags");
		return (false);
	}
	return (true);
}

static void
fill_edge_data(pbsd_localeconv_data_t *d, int variant)
{
	std::memset(d, 0, sizeof(*d));
	switch (variant) {
	case 0:
		/* empty strings */
		break;
	case 1:
		std::strcpy(d->m_int_curr_symbol, "A");
		std::strcpy(d->m_currency_symbol, "\x80");
		d->m_mon_decimal_point[0] = (char)0xff;
		d->m_mon_decimal_point[1] = '\0';
		std::strcpy(d->m_mon_thousands_sep, "\x80\xff");
		d->m_mon_grouping[0] = (char)0x80;
		d->m_mon_grouping[1] = '\0';
		d->m_positive_sign[0] = '\0';
		d->m_negative_sign[0] = (char)0xff;
		d->m_negative_sign[1] = '\0';
		break;
	case 2:
		std::strcpy(d->m_int_curr_symbol, "USD ");
		std::strcpy(d->m_currency_symbol, "$");
		std::strcpy(d->m_mon_decimal_point, ".");
		std::strcpy(d->m_mon_thousands_sep, ",");
		d->m_mon_grouping[0] = '\3';
		d->m_mon_grouping[1] = '\0';
		d->m_positive_sign[0] = '\0';
		std::strcpy(d->m_negative_sign, "-");
		d->m_int_frac_digits[0] = '2';
		d->m_frac_digits[0] = '2';
		d->m_p_cs_precedes[0] = '\1';
		d->m_p_sep_by_space[0] = '\0';
		d->m_n_cs_precedes[0] = '\1';
		d->m_n_sep_by_space[0] = '\0';
		d->m_p_sign_posn[0] = '\1';
		d->m_n_sign_posn[0] = '\1';
		d->m_int_p_cs_precedes[0] = '\1';
		d->m_int_n_cs_precedes[0] = '\1';
		d->m_int_p_sep_by_space[0] = '\0';
		d->m_int_n_sep_by_space[0] = '\0';
		d->m_int_p_sign_posn[0] = '\1';
		d->m_int_n_sign_posn[0] = '\1';
		std::strcpy(d->n_decimal_point, ".");
		d->n_thousands_sep[0] = '\0';
		d->n_grouping[0] = '\0';
		break;
	case 3:
		d->m_int_curr_symbol[0] = (char)0x80;
		d->m_int_curr_symbol[1] = (char)0xff;
		d->m_int_curr_symbol[2] = '\0';
		d->m_currency_symbol[0] = '\0';
		d->m_mon_decimal_point[0] = (char)0x81;
		d->m_mon_decimal_point[1] = '\0';
		d->m_mon_thousands_sep[0] = (char)0xfe;
		d->m_mon_thousands_sep[1] = '\0';
		d->m_mon_grouping[0] = (char)0xff;
		d->m_mon_grouping[1] = '\0';
		d->m_positive_sign[0] = (char)0x80;
		d->m_positive_sign[1] = '\0';
		d->m_negative_sign[0] = (char)0xfe;
		d->m_negative_sign[1] = '\0';
		d->m_int_frac_digits[0] = (char)0x80;
		d->m_frac_digits[0] = (char)0xff;
		d->m_p_cs_precedes[0] = (char)0xfe;
		d->m_p_sep_by_space[0] = (char)0x81;
		d->m_n_cs_precedes[0] = (char)0x80;
		d->m_n_sep_by_space[0] = (char)0xff;
		d->m_p_sign_posn[0] = (char)0xfe;
		d->m_n_sign_posn[0] = (char)0x80;
		d->m_int_p_cs_precedes[0] = (char)0xff;
		d->m_int_n_cs_precedes[0] = (char)0x81;
		d->m_int_p_sep_by_space[0] = (char)0x80;
		d->m_int_n_sep_by_space[0] = (char)0xfe;
		d->m_int_p_sign_posn[0] = (char)0xff;
		d->m_int_n_sign_posn[0] = (char)0x80;
		d->n_decimal_point[0] = (char)0xfe;
		d->n_decimal_point[1] = '\0';
		d->n_thousands_sep[0] = (char)0x80;
		d->n_thousands_sep[1] = '\0';
		d->n_grouping[0] = (char)0xff;
		d->n_grouping[1] = '\0';
		break;
	default:
		randomize_localeconv_data(d);
		break;
	}
}

static void
edge_localeconv_l(void)
{
	pbsd_localeconv_data_t *d = pbsd_get_localeconv_data();
	static const int combos[] = { 1, 1, 1, 0, 0, 1, 0, 0 };
	static const int combos2[] = { 1, 0, 1, 0, 0, 1, 1, 0 };

	for (int v = 0; v < 5; v++) {
		fill_edge_data(d, v);
		for (int i = 0; i < 8; i++) {
			prep_flags(combos[i], combos2[i]);
			run_localeconv_l(F_LOCALECONV_L, P::test_locale(),
			    ref_get_test_locale());
		}
		prep_flags(1, 1);
		run_localeconv_l(F_LOCALECONV_L, nullptr, nullptr);
	}
}

static void
edge_localeconv(void)
{
	pbsd_localeconv_data_t *d = pbsd_get_localeconv_data();
	static const int combos[] = { 1, 1, 1, 0, 0, 1, 0, 0 };
	static const int combos2[] = { 1, 0, 1, 0, 0, 1, 1, 0 };

	for (int v = 0; v < 5; v++) {
		fill_edge_data(d, v);
		for (int i = 0; i < 8; i++) {
			prep_flags(combos[i], combos2[i]);
			run_localeconv(F_LOCALECONV);
		}
	}
}

static void
sweep_localeconv_l(void)
{
	pbsd_localeconv_data_t *d = pbsd_get_localeconv_data();

	for (long long i = 0; i < SWEEP; i++) {
		randomize_localeconv_data(d);
		prep_flags((int)u32(2), (int)u32(2));
		run_localeconv_l(F_LOCALECONV_L, P::test_locale(),
		    ref_get_test_locale());
		if (u32(16) == 0)
			run_localeconv_l(F_LOCALECONV_L, nullptr, nullptr);
	}
}

static void
sweep_localeconv(void)
{
	pbsd_localeconv_data_t *d = pbsd_get_localeconv_data();

	for (long long i = 0; i < SWEEP; i++) {
		randomize_localeconv_data(d);
		prep_flags((int)u32(2), (int)u32(2));
		run_localeconv(F_LOCALECONV);
	}
}

int
main()
{
	pbsd_reset_hooks();

	edge_localeconv_l();
	sweep_localeconv_l();
	edge_localeconv();
	sweep_localeconv();

	std::printf("\n%-22s %12s %12s\n", "function", "cases", "failures");
	long long total_fail = 0;
	for (int i = 0; i < F_COUNT; i++) {
		std::printf("%-22s %12lld %12lld\n", fname[i], ncase[i],
		    nfail[i]);
		total_fail += nfail[i];
	}
	return (total_fail == 0 ? 0 : 1);
}
