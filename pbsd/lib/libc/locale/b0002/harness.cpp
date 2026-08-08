/*
 * Differential harness for batch b0002.
 */

import pbsd.lib.libc.locale.b0002;

#include <climits>
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace port = pbsd::lib_libc_locale::b0002;

typedef union {
	char		__bytes[128];
	long long	__align;
} ref_mbstate_t;

extern "C" {
typedef struct xlocale *ref_locale_t;

extern struct xlocale ref_global_locale;
extern struct xlocale_ctype ref_global_ctype;

size_t ref_c16rtomb_l(char *, char16_t, ref_mbstate_t *, ref_locale_t);
size_t ref_c16rtomb(char *, char16_t, ref_mbstate_t *);
size_t ref_c32rtomb_l(char *, char32_t, ref_mbstate_t *, ref_locale_t);
size_t ref_c32rtomb(char *, char32_t, ref_mbstate_t *);
size_t ref_mbrtoc16_l(char16_t *, const char *, size_t, ref_mbstate_t *, ref_locale_t);
size_t ref_mbrtoc16(char16_t *, const char *, size_t, ref_mbstate_t *);
size_t ref_mbrtoc32_l(char32_t *, const char *, size_t, ref_mbstate_t *, ref_locale_t);
size_t ref_mbrtoc32(char32_t *, const char *, size_t, ref_mbstate_t *);
extern int ref_iconv_open_fail;
}

struct xlocale_ctype {
	ref_mbstate_t	mbrtoc16;
	ref_mbstate_t	mbrtoc32;
	ref_mbstate_t	c16rtomb;
	ref_mbstate_t	c32rtomb;
};

struct xlocale {
	void		*components[6];
};

struct Stats {
	const char		*name;
	unsigned long long	cases;
	unsigned long long	fails;
};

static constexpr unsigned char GUARD = 0x7f;
static constexpr size_t OUT_CAP = 32;
static constexpr unsigned long long RANDOM_ITERS = 200000;

static Stats st_c16rtomb_l = { "c16rtomb_l", 0, 0 };
static Stats st_c16rtomb = { "c16rtomb", 0, 0 };
static Stats st_c32rtomb_l = { "c32rtomb_l", 0, 0 };
static Stats st_c32rtomb = { "c32rtomb", 0, 0 };
static Stats st_mbrtoc16_l = { "mbrtoc16_l", 0, 0 };
static Stats st_mbrtoc16 = { "mbrtoc16", 0, 0 };
static Stats st_mbrtoc32_l = { "mbrtoc32_l", 0, 0 };
static Stats st_mbrtoc32 = { "mbrtoc32", 0, 0 };

static uint32_t rng = 0xB0002002u;

static uint32_t
xorshift32()
{
	uint32_t x = rng;

	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	rng = x;
	return (x);
}

static void
init_locales()
{
	port::init_locale();
	ref_global_locale.components[1] = &ref_global_ctype;
}

static void
zero_state(port::mbstate_t *ps)
{
	memset(ps, 0, sizeof(*ps));
}

static void
zero_state(ref_mbstate_t *ps)
{
	memset(ps, 0, sizeof(*ps));
}

static void
fill_guard(unsigned char *buf, size_t n)
{
	memset(buf, GUARD, n);
}

static bool
guards_intact(const unsigned char *buf, size_t n)
{
	for (size_t i = 0; i < n; i++) {
		if (buf[i] != GUARD)
			return (false);
	}
	return (true);
}

static void
cp_to_surrogate(uint32_t cp, char16_t *hi, char16_t *lo)
{
	uint32_t t = cp - 0x10000;

	*hi = (char16_t)(0xD800 + (t >> 10));
	*lo = (char16_t)(0xDC00 + (t & 0x3FF));
}

static size_t
utf8_encode(uint32_t cp, unsigned char *out, size_t cap)
{
	if (cp < 0x80 && cap >= 1) {
		out[0] = (unsigned char)cp;
		return (1);
	}
	if (cp < 0x800 && cap >= 2) {
		out[0] = (unsigned char)(0xC0 | (cp >> 6));
		out[1] = (unsigned char)(0x80 | (cp & 0x3F));
		return (2);
	}
	if (cp < 0x10000 && cap >= 3) {
		out[0] = (unsigned char)(0xE0 | (cp >> 12));
		out[1] = (unsigned char)(0x80 | ((cp >> 6) & 0x3F));
		out[2] = (unsigned char)(0x80 | (cp & 0x3F));
		return (3);
	}
	if (cp <= 0x10FFFF && cap >= 4) {
		out[0] = (unsigned char)(0xF0 | (cp >> 18));
		out[1] = (unsigned char)(0x80 | ((cp >> 12) & 0x3F));
		out[2] = (unsigned char)(0x80 | ((cp >> 6) & 0x3F));
		out[3] = (unsigned char)(0x80 | (cp & 0x3F));
		return (4);
	}
	return (0);
}

static void
report_fail(Stats &st, const char *tag)
{
	st.fails++;
	if (st.fails <= 5)
		std::printf("FAIL %s [%s]\n", st.name, tag);
}

static bool
compare_rtomb_l(Stats &st, char16_t c, bool use_l, bool null_ps)
{
	unsigned char pout[OUT_CAP], rout[OUT_CAP];
	port::mbstate_t pstate;
	ref_mbstate_t rstate;
	size_t pr, rr;
	int perrno, rerrno;

	st.cases++;
	zero_state(&pstate);
	zero_state(&rstate);
	fill_guard(pout, sizeof(pout));
	fill_guard(rout, sizeof(rout));

	errno = 0;
	if (use_l) {
		pr = port::c16rtomb_l((char *)(pout + 4), c,
		    null_ps ? nullptr : &pstate, port::global_locale());
		perrno = errno;
		errno = 0;
		rr = ref_c16rtomb_l((char *)(rout + 4), c,
		    null_ps ? nullptr : &rstate, &ref_global_locale);
		rerrno = errno;
	} else {
		pr = port::c16rtomb((char *)(pout + 4), c,
		    null_ps ? nullptr : &pstate);
		perrno = errno;
		errno = 0;
		rr = ref_c16rtomb((char *)(rout + 4), c,
		    null_ps ? nullptr : &rstate);
		rerrno = errno;
	}

	if (pr != rr || perrno != rerrno)
		return (report_fail(st, "ret"), false);
	if (!guards_intact(pout, 4) || !guards_intact(pout + 4 + 16, 12))
		return (report_fail(st, "port guard"), false);
	if (!guards_intact(rout, 4) || !guards_intact(rout + 4 + 16, 12))
		return (report_fail(st, "ref guard"), false);
	if (memcmp(pout, rout, sizeof(pout)) != 0)
		return (report_fail(st, "buffer"), false);
	return (true);
}

static bool
compare_c32rtomb_l(Stats &st, char32_t c, bool use_l, bool null_ps)
{
	unsigned char pout[OUT_CAP], rout[OUT_CAP];
	port::mbstate_t pstate;
	ref_mbstate_t rstate;
	size_t pr, rr;
	int perrno, rerrno;

	st.cases++;
	zero_state(&pstate);
	zero_state(&rstate);
	fill_guard(pout, sizeof(pout));
	fill_guard(rout, sizeof(rout));

	errno = 0;
	if (use_l) {
		pr = port::c32rtomb_l((char *)(pout + 4), c,
		    null_ps ? nullptr : &pstate, port::global_locale());
		perrno = errno;
		errno = 0;
		rr = ref_c32rtomb_l((char *)(rout + 4), c,
		    null_ps ? nullptr : &rstate, &ref_global_locale);
		rerrno = errno;
	} else {
		pr = port::c32rtomb((char *)(pout + 4), c,
		    null_ps ? nullptr : &pstate);
		perrno = errno;
		errno = 0;
		rr = ref_c32rtomb((char *)(rout + 4), c,
		    null_ps ? nullptr : &rstate);
		rerrno = errno;
	}

	if (pr != rr || perrno != rerrno)
		return (report_fail(st, "ret"), false);
	if (memcmp(pout, rout, sizeof(pout)) != 0)
		return (report_fail(st, "buffer"), false);
	return (true);
}

static bool
compare_mbrtoc_l(Stats &st, const unsigned char *in, size_t n, bool is32,
    bool use_l, bool null_ps, bool null_pc)
{
	unsigned char pin[64], rin[64];
	unsigned char pwcbuf[16], rwcbuf[16];
	port::mbstate_t pstate;
	ref_mbstate_t rstate;
	char16_t p16, r16;
	char32_t p32, r32;
	size_t pr, rr;
	int perrno, rerrno;

	st.cases++;
	zero_state(&pstate);
	zero_state(&rstate);
	fill_guard(pin, sizeof(pin));
	fill_guard(rin, sizeof(rin));
	fill_guard(pwcbuf, sizeof(pwcbuf));
	fill_guard(rwcbuf, sizeof(rwcbuf));
	memcpy(pin + 8, in, n);
	memcpy(rin + 8, in, n);

	errno = 0;
	if (is32) {
		if (use_l) {
			pr = port::mbrtoc32_l(null_pc ? nullptr : (char32_t *)(pwcbuf + 4),
			    (const char *)(pin + 8), n,
			    null_ps ? nullptr : &pstate, port::global_locale());
			perrno = errno;
			errno = 0;
			rr = ref_mbrtoc32_l(null_pc ? nullptr : (char32_t *)(rwcbuf + 4),
			    (const char *)(rin + 8), n,
			    null_ps ? nullptr : &rstate, &ref_global_locale);
			rerrno = errno;
		} else {
			pr = port::mbrtoc32(null_pc ? nullptr : (char32_t *)(pwcbuf + 4),
			    (const char *)(pin + 8), n,
			    null_ps ? nullptr : &pstate);
			perrno = errno;
			errno = 0;
			rr = ref_mbrtoc32(null_pc ? nullptr : (char32_t *)(rwcbuf + 4),
			    (const char *)(rin + 8), n,
			    null_ps ? nullptr : &rstate);
			rerrno = errno;
		}
		memcpy(&p32, pwcbuf + 4, sizeof(p32));
		memcpy(&r32, rwcbuf + 4, sizeof(r32));
	} else {
		if (use_l) {
			pr = port::mbrtoc16_l(null_pc ? nullptr : (char16_t *)(pwcbuf + 4),
			    (const char *)(pin + 8), n,
			    null_ps ? nullptr : &pstate, port::global_locale());
			perrno = errno;
			errno = 0;
			rr = ref_mbrtoc16_l(null_pc ? nullptr : (char16_t *)(rwcbuf + 4),
			    (const char *)(rin + 8), n,
			    null_ps ? nullptr : &rstate, &ref_global_locale);
			rerrno = errno;
		} else {
			pr = port::mbrtoc16(null_pc ? nullptr : (char16_t *)(pwcbuf + 4),
			    (const char *)(pin + 8), n,
			    null_ps ? nullptr : &pstate);
			perrno = errno;
			errno = 0;
			rr = ref_mbrtoc16(null_pc ? nullptr : (char16_t *)(rwcbuf + 4),
			    (const char *)(rin + 8), n,
			    null_ps ? nullptr : &rstate);
			rerrno = errno;
		}
		memcpy(&p16, pwcbuf + 4, sizeof(p16));
		memcpy(&r16, rwcbuf + 4, sizeof(r16));
	}

	if (pr != rr || perrno != rerrno)
		return (report_fail(st, "ret"), false);
	if (memcmp(pin, rin, sizeof(pin)) != 0)
		return (report_fail(st, "inbuf"), false);
	if (memcmp(pwcbuf, rwcbuf, sizeof(pwcbuf)) != 0)
		return (report_fail(st, "wcharbuf"), false);
	if (!is32 && p16 != r16)
		return (report_fail(st, "wchar"), false);
	if (is32 && p32 != r32)
		return (report_fail(st, "wchar"), false);
	return (true);
}

static bool
compare_reset_rtomb(Stats &st, bool is32, bool use_l)
{
	port::mbstate_t pstate;
	ref_mbstate_t rstate;
	size_t pr, rr;

	st.cases++;
	zero_state(&pstate);
	zero_state(&rstate);
	errno = 0;
	if (is32) {
		if (use_l) {
			pr = port::c32rtomb_l(nullptr, 0, &pstate, port::global_locale());
			rr = ref_c32rtomb_l(nullptr, 0, &rstate, &ref_global_locale);
		} else {
			pr = port::c32rtomb(nullptr, 0, &pstate);
			rr = ref_c32rtomb(nullptr, 0, &rstate);
		}
	} else {
		if (use_l) {
			pr = port::c16rtomb_l(nullptr, 0, &pstate, port::global_locale());
			rr = ref_c16rtomb_l(nullptr, 0, &rstate, &ref_global_locale);
		} else {
			pr = port::c16rtomb(nullptr, 0, &pstate);
			rr = ref_c16rtomb(nullptr, 0, &rstate);
		}
	}
	if (pr != rr)
		return (report_fail(st, "reset"), false);
	return (true);
}

static bool
compare_reset_mbrtoc(Stats &st, bool is32, bool use_l)
{
	port::mbstate_t pstate;
	ref_mbstate_t rstate;
	size_t pr, rr;

	st.cases++;
	zero_state(&pstate);
	zero_state(&rstate);
	errno = 0;
	if (is32) {
		if (use_l)
			pr = port::mbrtoc32_l(nullptr, nullptr, 0, &pstate,
			    port::global_locale());
		else
			pr = port::mbrtoc32(nullptr, nullptr, 0, &pstate);
		rr = use_l ?
		    ref_mbrtoc32_l(nullptr, nullptr, 0, &rstate, &ref_global_locale) :
		    ref_mbrtoc32(nullptr, nullptr, 0, &rstate);
	} else {
		if (use_l)
			pr = port::mbrtoc16_l(nullptr, nullptr, 0, &pstate,
			    port::global_locale());
		else
			pr = port::mbrtoc16(nullptr, nullptr, 0, &pstate);
		rr = use_l ?
		    ref_mbrtoc16_l(nullptr, nullptr, 0, &rstate, &ref_global_locale) :
		    ref_mbrtoc16(nullptr, nullptr, 0, &rstate);
	}
	if (pr != rr)
		return (report_fail(st, "reset"), false);
	return (true);
}

static bool
compare_open_fail(Stats &st, bool is32, bool rtomb)
{
	port::mbstate_t pstate;
	ref_mbstate_t rstate;
	size_t pr, rr;
	int perrno, rerrno;

	st.cases++;
	ref_iconv_open_fail = 1;
	zero_state(&pstate);
	zero_state(&rstate);
	errno = 0;
	if (rtomb) {
		if (is32) {
			pr = port::c32rtomb_l((char *)1, 0, &pstate, port::global_locale());
			perrno = errno;
			errno = 0;
			rr = ref_c32rtomb_l((char *)1, 0, &rstate, &ref_global_locale);
		} else {
			pr = port::c16rtomb_l((char *)1, 0, &pstate, port::global_locale());
			perrno = errno;
			errno = 0;
			rr = ref_c16rtomb_l((char *)1, 0, &rstate, &ref_global_locale);
		}
	} else {
		if (is32) {
			pr = port::mbrtoc32_l(nullptr, "", 0, &pstate, port::global_locale());
			perrno = errno;
			errno = 0;
			rr = ref_mbrtoc32_l(nullptr, "", 0, &rstate, &ref_global_locale);
		} else {
			pr = port::mbrtoc16_l(nullptr, "", 0, &pstate, port::global_locale());
			perrno = errno;
			errno = 0;
			rr = ref_mbrtoc16_l(nullptr, "", 0, &rstate, &ref_global_locale);
		}
	}
	ref_iconv_open_fail = 0;
	rerrno = errno;
	if (pr != rr || perrno != rerrno)
		return (report_fail(st, "openfail"), false);
	return (true);
}

static void
hand_c16rtomb(Stats &st, bool use_l)
{
	char16_t hi, lo;

	compare_reset_rtomb(st, false, use_l);
	compare_rtomb_l(st, 0, use_l, false);
	compare_rtomb_l(st, u'A', use_l, false);
	compare_rtomb_l(st, u'\0', use_l, false);
	compare_rtomb_l(st, 0x7FF, use_l, false);
	compare_rtomb_l(st, 0x80, use_l, false);
	compare_rtomb_l(st, 0xFF, use_l, false);
	compare_rtomb_l(st, 0xD800, use_l, false);
	cp_to_surrogate(0x10000, &hi, &lo);
	compare_rtomb_l(st, hi, use_l, false);
	compare_rtomb_l(st, lo, use_l, false);
	compare_rtomb_l(st, 0xDC00, use_l, false);
	compare_rtomb_l(st, 0xDFFF, use_l, false);
	compare_rtomb_l(st, u'A', use_l, true);
	compare_open_fail(st, false, true);
}

static void
hand_c32rtomb(Stats &st, bool use_l)
{
	compare_reset_rtomb(st, true, use_l);
	compare_c32rtomb_l(st, 0, use_l, false);
	compare_c32rtomb_l(st, U'A', use_l, false);
	compare_c32rtomb_l(st, U'\0', use_l, false);
	compare_c32rtomb_l(st, 0x7FF, use_l, false);
	compare_c32rtomb_l(st, 0x10000, use_l, false);
	compare_c32rtomb_l(st, 0x10FFFF, use_l, false);
	compare_c32rtomb_l(st, 0xD800, use_l, false);
	compare_c32rtomb_l(st, U'A', use_l, true);
	compare_open_fail(st, true, true);
}

static void
hand_mbrtoc16(Stats &st, bool use_l)
{
	unsigned char seq[8];
	size_t n;

	compare_reset_mbrtoc(st, false, use_l);
	seq[0] = '\0';
	compare_mbrtoc_l(st, seq, 1, false, use_l, false, false);
	seq[0] = 'A';
	compare_mbrtoc_l(st, seq, 1, false, use_l, false, false);
	n = utf8_encode(0x80, seq, sizeof(seq));
	compare_mbrtoc_l(st, seq, n, false, use_l, false, false);
	n = utf8_encode(0x7FF, seq, sizeof(seq));
	compare_mbrtoc_l(st, seq, n, false, use_l, false, false);
	n = utf8_encode(0x10000, seq, sizeof(seq));
	compare_mbrtoc_l(st, seq, n, false, use_l, false, false);
	compare_mbrtoc_l(st, seq, 1, false, use_l, false, false);
	seq[0] = 0xFF;
	compare_mbrtoc_l(st, seq, 1, false, use_l, false, false);
	seq[0] = 0xC0;
	seq[1] = 0x80;
	compare_mbrtoc_l(st, seq, 2, false, use_l, false, false);
	compare_mbrtoc_l(st, seq, 1, false, use_l, false, true);
	compare_mbrtoc_l(st, seq, 1, false, use_l, true, false);
	compare_open_fail(st, false, false);
}

static void
hand_mbrtoc32(Stats &st, bool use_l)
{
	unsigned char seq[8];
	size_t n;

	compare_reset_mbrtoc(st, true, use_l);
	seq[0] = '\0';
	compare_mbrtoc_l(st, seq, 1, true, use_l, false, false);
	seq[0] = 'Z';
	compare_mbrtoc_l(st, seq, 1, true, use_l, false, false);
	n = utf8_encode(0x10000, seq, sizeof(seq));
	compare_mbrtoc_l(st, seq, n, true, use_l, false, false);
	seq[0] = 0xF4;
	seq[1] = 0x90;
	seq[2] = 0x80;
	seq[3] = 0x80;
	compare_mbrtoc_l(st, seq, 4, true, use_l, false, false);
	seq[0] = 0x80;
	compare_mbrtoc_l(st, seq, 1, true, use_l, false, false);
	compare_mbrtoc_l(st, seq, 0, true, use_l, false, false);
	compare_open_fail(st, true, false);
}

static void
random_c16rtomb(Stats &st, bool use_l)
{
	for (unsigned long long i = 0; i < RANDOM_ITERS; i++) {
		char16_t c = (char16_t)(xorshift32() & 0xFFFFu);

		if (!compare_rtomb_l(st, c, use_l, (xorshift32() & 7u) == 0))
			return;
		if ((xorshift32() & 0x3FFu) == 0)
			compare_reset_rtomb(st, false, use_l);
	}
}

static void
random_c32rtomb(Stats &st, bool use_l)
{
	for (unsigned long long i = 0; i < RANDOM_ITERS; i++) {
		char32_t c = xorshift32();

		if ((xorshift32() & 3u) == 0)
			c &= 0xFFFFu;
		if (!compare_c32rtomb_l(st, c, use_l, (xorshift32() & 7u) == 0))
			return;
		if ((xorshift32() & 0x3FFu) == 0)
			compare_reset_rtomb(st, true, use_l);
	}
}

static void
random_mbrtoc(Stats &st, bool is32, bool use_l)
{
	unsigned char seq[MB_LEN_MAX];

	for (unsigned long long i = 0; i < RANDOM_ITERS; i++) {
		size_t n = (size_t)(xorshift32() % (MB_LEN_MAX + 1));

		for (size_t j = 0; j < n; j++)
			seq[j] = (unsigned char)(xorshift32() & 0xFFu);
		if (!compare_mbrtoc_l(st, seq, n, is32, use_l,
		    (xorshift32() & 7u) == 0, (xorshift32() & 7u) == 0))
			return;
		if ((xorshift32() & 0x3FFu) == 0)
			compare_reset_mbrtoc(st, is32, use_l);
	}
}

static void
print_table(const Stats *stats, size_t n)
{
	std::printf("\n%-16s %12s %12s\n", "function", "cases", "failures");
	std::printf("%-16s %12s %12s\n", "--------", "-----", "--------");
	for (size_t i = 0; i < n; i++)
		std::printf("%-16s %12llu %12llu\n",
		    stats[i].name, stats[i].cases, stats[i].fails);
}

int
main()
{
	init_locales();

	hand_c16rtomb(st_c16rtomb_l, true);
	hand_c16rtomb(st_c16rtomb, false);
	hand_c32rtomb(st_c32rtomb_l, true);
	hand_c32rtomb(st_c32rtomb, false);
	hand_mbrtoc16(st_mbrtoc16_l, true);
	hand_mbrtoc16(st_mbrtoc16, false);
	hand_mbrtoc32(st_mbrtoc32_l, true);
	hand_mbrtoc32(st_mbrtoc32, false);

	random_c16rtomb(st_c16rtomb_l, true);
	random_c16rtomb(st_c16rtomb, false);
	random_c32rtomb(st_c32rtomb_l, true);
	random_c32rtomb(st_c32rtomb, false);
	random_mbrtoc(st_mbrtoc16_l, false, true);
	random_mbrtoc(st_mbrtoc16, false, false);
	random_mbrtoc(st_mbrtoc32_l, true, true);
	random_mbrtoc(st_mbrtoc32, true, false);

	print_table(&st_c16rtomb_l, 1);
	print_table(&st_c16rtomb, 1);
	print_table(&st_c32rtomb_l, 1);
	print_table(&st_c32rtomb, 1);
	print_table(&st_mbrtoc16_l, 1);
	print_table(&st_mbrtoc16, 1);
	print_table(&st_mbrtoc32_l, 1);
	print_table(&st_mbrtoc32, 1);

	if (st_c16rtomb_l.fails != 0 || st_c16rtomb.fails != 0 ||
	    st_c32rtomb_l.fails != 0 || st_c32rtomb.fails != 0 ||
	    st_mbrtoc16_l.fails != 0 || st_mbrtoc16.fails != 0 ||
	    st_mbrtoc32_l.fails != 0 || st_mbrtoc32.fails != 0)
		return (1);
	return (0);
}
