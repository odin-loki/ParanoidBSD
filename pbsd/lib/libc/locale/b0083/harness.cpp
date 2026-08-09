/*
 * Differential harness for batch b0083.
 */

import pbsd.lib.libc.locale.b0083;

#include <climits>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace port = pbsd::lib_libc_locale::b0083;

#ifndef EOF
#define EOF	(-1)
#endif

extern "C" {
typedef struct ref_xlocale *ref_locale_t;

extern ref_locale_t ref_current_locale;

const char *ref___fix_locale_grouping_str(const char *);
int ref____tolower_l(int c, ref_locale_t l);
int ref____tolower(int c);
int ref____toupper_l(int c, ref_locale_t l);
int ref____toupper(int c);
}

struct ref_xlocale_ctype {
	port::_RuneLocale	*runes;
};

struct ref_xlocale {
	ref_xlocale_ctype	*__ctype;
};

struct Stats {
	const char		*name;
	unsigned long long	cases;
	unsigned long long	fails;
};

static constexpr unsigned char GUARD = 0x7f;
static constexpr size_t GRP_CAP = 128;
static constexpr unsigned long long RANDOM_ITERS = 200000;

static Stats st_fix_grouping = { "__fix_locale_grouping_str", 0, 0 };
static Stats st_tolower_l = { "___tolower_l", 0, 0 };
static Stats st_tolower = { "___tolower", 0, 0 };
static Stats st_toupper_l = { "___toupper_l", 0, 0 };
static Stats st_toupper = { "___toupper", 0, 0 };

static uint32_t rng = 0xB0083001u;

static port::_RuneEntry lower_entries[8];
static port::_RuneEntry upper_entries[8];
static port::_RuneLocale shared_runes;
static port::xlocale_ctype port_ctype;
static port::xlocale port_locale;
static ref_xlocale_ctype ref_ctype;
static ref_xlocale ref_locale;

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
fill_guard(unsigned char *buf, size_t n)
{
	memset(buf, GUARD, n);
}

static void
report_fail(Stats &st, const char *tag)
{
	st.fails++;
	if (st.fails <= 5)
		std::printf("FAIL %s [%s]\n", st.name, tag);
}

static void
init_rune_tables()
{
	static const struct {
		int	min, max, map;
	} lower_spec[] = {
		{ 0x10, 0x1f, 0x60 },
		{ 0x30, 0x3f, 0x70 },
		{ 0x50, 0x5f, 0x80 },
		{ 0x70, 0x7f, 0x90 },
		{ 0x90, 0x9f, 0xa0 },
		{ 0xb0, 0xbf, 0xc0 },
		{ 0xd0, 0xdf, 0xe0 },
		{ 0xf0, 0xff, 0x00 },
	};
	static const struct {
		int	min, max, map;
	} upper_spec[] = {
		{ 0x00, 0x0f, 0x40 },
		{ 0x20, 0x2f, 0x50 },
		{ 0x40, 0x4f, 0x60 },
		{ 0x60, 0x6f, 0x70 },
		{ 0x80, 0x8f, 0x90 },
		{ 0xa0, 0xaf, 0xb0 },
		{ 0xc0, 0xcf, 0xd0 },
		{ 0xe0, 0xef, 0xf0 },
	};

	for (int i = 0; i < 8; i++) {
		lower_entries[i].__min = lower_spec[i].min;
		lower_entries[i].__max = lower_spec[i].max;
		lower_entries[i].__map = lower_spec[i].map;
		lower_entries[i].__types = nullptr;
		upper_entries[i].__min = upper_spec[i].min;
		upper_entries[i].__max = upper_spec[i].max;
		upper_entries[i].__map = upper_spec[i].map;
		upper_entries[i].__types = nullptr;
	}

	shared_runes.__maplower_ext.__nranges = 8;
	shared_runes.__maplower_ext.__ranges = lower_entries;
	shared_runes.__mapupper_ext.__nranges = 8;
	shared_runes.__mapupper_ext.__ranges = upper_entries;

	port_ctype.runes = &shared_runes;
	port_locale.__ctype = &port_ctype;
	port::current_locale = &port_locale;

	ref_ctype.runes = &shared_runes;
	ref_locale.__ctype = &ref_ctype;
	ref_current_locale = (ref_locale_t)&ref_locale;
}

static bool
ret_in_buffer(const char *ret, const char *buf)
{
	return (ret == buf);
}

static bool
compare_fix_grouping(Stats &st, const char *input)
{
	char pbuf[GRP_CAP], rbuf[GRP_CAP];
	const char *pr, *rr;
	bool p_in, r_in;

	st.cases++;
	fill_guard((unsigned char *)pbuf, sizeof(pbuf));
	fill_guard((unsigned char *)rbuf, sizeof(rbuf));

	if (input != nullptr) {
		size_t len = strlen(input);

		if (len >= GRP_CAP)
			len = GRP_CAP - 1;
		memcpy(pbuf, input, len);
		pbuf[len] = '\0';
		memcpy(rbuf, input, len);
		rbuf[len] = '\0';
	}

	pr = port::__fix_locale_grouping_str(input != nullptr ? pbuf : nullptr);
	rr = ref___fix_locale_grouping_str(input != nullptr ? rbuf : nullptr);

	if (memcmp(pbuf, rbuf, sizeof(pbuf)) != 0) {
		report_fail(st, "buf");
		return (false);
	}

	p_in = ret_in_buffer(pr, pbuf);
	r_in = ret_in_buffer(rr, rbuf);
	if (p_in != r_in) {
		report_fail(st, "ret-kind");
		return (false);
	}
	if (!p_in && !r_in) {
		if ((pr != nullptr && *pr != '\0') ||
		    (rr != nullptr && *rr != '\0')) {
			report_fail(st, "nogroup");
			return (false);
		}
	}
	return (true);
}

static bool
compare_tolower_l(Stats &st, int c, bool use_l, bool null_locale)
{
	int pr, rr;

	st.cases++;
	if (use_l) {
		pr = port::___tolower_l(c, null_locale ? nullptr :
		    port::current_locale);
		rr = ref____tolower_l(c, null_locale ? nullptr :
		    ref_current_locale);
	} else {
		pr = port::___tolower(c);
		rr = ref____tolower(c);
	}

	if (pr != rr) {
		report_fail(st, "ret");
		return (false);
	}
	return (true);
}

static bool
compare_toupper_l(Stats &st, int c, bool use_l, bool null_locale)
{
	int pr, rr;

	st.cases++;
	if (use_l) {
		pr = port::___toupper_l(c, null_locale ? nullptr :
		    port::current_locale);
		rr = ref____toupper_l(c, null_locale ? nullptr :
		    ref_current_locale);
	} else {
		pr = port::___toupper(c);
		rr = ref____toupper(c);
	}

	if (pr != rr) {
		report_fail(st, "ret");
		return (false);
	}
	return (true);
}

static void
test_fix_grouping_hand()
{
	static const char *cases[] = {
		nullptr,
		"",
		"3",
		"3;3",
		"3;2;-1",
		";3",
		"3;",
		"-1",
		"99",
		"10",
		"0",
		"0;3",
		"3;0",
		"00",
		"a",
		"3a",
		"3;-",
		"3;-2",
		"127",
		"12;34",
		"\x80",
		"\xff",
		"3;\xff",
		"\xff;3",
		"9;9;9;9;9",
		"1;-1;2",
		"55;-1",
		"0;0;0",
		"1;2;3;4;5;6;7;8;9",
		";",
		";;",
		";;;3",
		"3;;;",
		"-",
		"--1",
		"1-1",
		"3;1;-1;0",
		"\x7f",
		"3;\x7f",
	};

	for (const char *s : cases)
		compare_fix_grouping(st_fix_grouping, s);
}

static void
test_tolower_hand()
{
	static const int cases[] = {
		-2, -1, 0, 1, EOF,
		0x0f, 0x10, 0x1f, 0x20,
		0x2f, 0x30, 0x3f, 0x40,
		0x4f, 0x50, 0x5f, 0x60,
		0x6f, 0x70, 0x7f, 0x80,
		0x8f, 0x90, 0x9f, 0xa0,
		0xaf, 0xb0, 0xbf, 0xc0,
		0xcf, 0xd0, 0xdf, 0xe0,
		0xef, 0xf0, 0xff,
		0x100, 0x7fffffff, -0x7fffffff,
	};

	for (int c : cases) {
		compare_tolower_l(st_tolower_l, c, true, false);
		compare_tolower_l(st_tolower_l, c, true, true);
		compare_tolower_l(st_tolower, c, false, false);
	}
}

static void
test_toupper_hand()
{
	static const int cases[] = {
		-2, -1, 0, 1, EOF,
		0x0f, 0x10, 0x1f, 0x20,
		0x2f, 0x30, 0x3f, 0x40,
		0x4f, 0x50, 0x5f, 0x60,
		0x6f, 0x70, 0x7f, 0x80,
		0x8f, 0x90, 0x9f, 0xa0,
		0xaf, 0xb0, 0xbf, 0xc0,
		0xcf, 0xd0, 0xdf, 0xe0,
		0xef, 0xf0, 0xff,
		0x100, 0x7fffffff, -0x7fffffff,
	};

	for (int c : cases) {
		compare_toupper_l(st_toupper_l, c, true, false);
		compare_toupper_l(st_toupper_l, c, true, true);
		compare_toupper_l(st_toupper, c, false, false);
	}
}

static void
random_grouping_string(char *out, size_t cap)
{
	size_t len;
	unsigned char ch;

	if (cap < 2) {
		out[0] = '\0';
		return;
	}

	len = 1 + (xorshift32() % (cap - 1));
	for (size_t i = 0; i < len; i++) {
		switch (xorshift32() % 8) {
		case 0:
			ch = (unsigned char)('0' + (xorshift32() % 10));
			break;
		case 1:
			ch = ';';
			break;
		case 2:
			ch = '-';
			break;
		case 3:
			ch = '1';
			break;
		case 4:
			ch = (unsigned char)(0x80 + (xorshift32() % 0x80));
			break;
		case 5:
			ch = '\0';
			len = i;
			goto done;
		case 6:
			ch = (unsigned char)((xorshift32() % 26) + 'a');
			break;
		default:
			ch = (unsigned char)(xorshift32() & 0xff);
			break;
		}
		out[i] = (char)ch;
	}
done:
	out[len] = '\0';
}

static void
test_fix_grouping_random()
{
	char input[GRP_CAP];

	for (unsigned long long i = 0; i < RANDOM_ITERS; i++) {
		random_grouping_string(input, sizeof(input));
		compare_fix_grouping(st_fix_grouping, input);
		if ((xorshift32() & 0x3ff) == 0)
			compare_fix_grouping(st_fix_grouping, nullptr);
	}
}

static void
test_tolower_random()
{
	for (unsigned long long i = 0; i < RANDOM_ITERS; i++) {
		int c = (int)xorshift32();
		if ((xorshift32() & 7) == 0)
			c = (int)(int32_t)xorshift32();
		if ((xorshift32() & 15) == 0)
			c = EOF;
		if ((xorshift32() & 31) == 0)
			c = -1 - (int)(xorshift32() & 0xff);

		compare_tolower_l(st_tolower_l, c, true, (xorshift32() & 3) == 0);
		compare_tolower_l(st_tolower, c, false, false);
	}
}

static void
test_toupper_random()
{
	for (unsigned long long i = 0; i < RANDOM_ITERS; i++) {
		int c = (int)xorshift32();
		if ((xorshift32() & 7) == 0)
			c = (int)(int32_t)xorshift32();
		if ((xorshift32() & 15) == 0)
			c = EOF;
		if ((xorshift32() & 31) == 0)
			c = -1 - (int)(xorshift32() & 0xff);

		compare_toupper_l(st_toupper_l, c, true, (xorshift32() & 3) == 0);
		compare_toupper_l(st_toupper, c, false, false);
	}
}

static void
print_stats(const Stats &st)
{
	std::printf("%-30s %12llu %12llu\n", st.name, st.cases, st.fails);
}

int
main()
{
	unsigned long long total_fails = 0;

	init_rune_tables();

	test_fix_grouping_hand();
	test_tolower_hand();
	test_toupper_hand();

	test_fix_grouping_random();
	test_tolower_random();
	test_toupper_random();

	std::printf("\n%-30s %12s %12s\n", "function", "cases", "failures");
	print_stats(st_fix_grouping);
	print_stats(st_tolower_l);
	print_stats(st_tolower);
	print_stats(st_toupper_l);
	print_stats(st_toupper);

	total_fails = st_fix_grouping.fails + st_tolower_l.fails +
	    st_tolower.fails + st_toupper_l.fails + st_toupper.fails;

	return (total_fails == 0 ? 0 : 1);
}
