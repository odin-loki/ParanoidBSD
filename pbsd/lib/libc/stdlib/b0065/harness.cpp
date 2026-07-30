/*
 * harness.cpp -- differential test for PBSD batch b0065.
 *
 * Compares the C++23 module port against the unmodified C oracle for:
 *	strtoq, strtouq, div, ldiv
 *
 * Every case exercises both implementations and compares return values,
 * errno, endptr offsets (never raw addresses) and the entire input buffer
 * including guard bytes past the nominal write window.
 */

#include <sys/types.h>

#include <cerrno>
#include <climits>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <string>
#include <vector>

import pbsd.lib.libc.stdlib.b0065;

namespace P = pbsd::lib_libc_stdlib::b0065;

extern "C" {
quad_t ref_strtoq(const char *nptr, char **endptr, int base);
u_quad_t ref_strtouq(const char *nptr, char **endptr, int base);
div_t ref_div(int numer, int denom);
ldiv_t ref_ldiv(long numer, long denom);
}

/* ------------------------------------------------------------------ stats */

struct Stat {
	const char *name;
	unsigned long long cases;
	unsigned long long fails;
	unsigned long long printed;
};

static Stat st_strtoq = { "strtoq", 0, 0, 0 };
static Stat st_strtouq = { "strtouq", 0, 0, 0 };
static Stat st_div = { "div", 0, 0, 0 };
static Stat st_ldiv = { "ldiv", 0, 0, 0 };

static const unsigned long long MAX_PRINT = 8;

static void
fail(Stat &s, const char *detail)
{
	s.fails++;
	if (s.printed < MAX_PRINT) {
		s.printed++;
		std::fprintf(stderr, "FAIL %s: %s\n", s.name, detail);
	}
}

static std::string
escape(const char *p, size_t n)
{
	std::string out;
	char tmp[8];
	for (size_t i = 0; i < n; i++) {
		unsigned char c = (unsigned char)p[i];
		if (c >= 0x20 && c < 0x7f && c != '\\' && c != '"') {
			out.push_back((char)c);
		} else {
			std::snprintf(tmp, sizeof(tmp), "\\x%02x", c);
			out += tmp;
		}
	}
	return out;
}

/* --------------------------------------------------------------- rng ----- */

static uint64_t rng_state;

static void
rng_seed(uint64_t s)
{
	rng_state = s;
}

static uint64_t
rng_next(void)
{
	uint64_t z = (rng_state += 0x9E3779B97F4A7C15ULL);
	z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
	z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
	return z ^ (z >> 31);
}

static uint32_t
rng_below(uint32_t n)
{
	return (uint32_t)(rng_next() % (uint64_t)n);
}

/* ------------------------------------------------------- strto* checking - */

static const size_t BUFSZ = 128;
static const unsigned char GUARD = 0x7f;

/*
 * Run one strtoq case.  The input is copied into two independently
 * guard-filled buffers so that any stray write by either implementation is
 * visible, and endptr is compared as an offset from the respective base.
 */
static void
check_strtoq(const std::string &in, int base, bool use_endptr)
{
	char a[BUFSZ], b[BUFSZ];
	char detail[512];

	st_strtoq.cases++;

	std::memset(a, GUARD, BUFSZ);
	std::memset(b, GUARD, BUFSZ);

	size_t n = in.size();
	if (n > BUFSZ - 2)
		n = BUFSZ - 2;
	std::memcpy(a, in.data(), n);
	std::memcpy(b, in.data(), n);
	a[n] = '\0';
	b[n] = '\0';

	/*
	 * Seed endptr with an offset no legitimate write can produce, so an
	 * implementation that leaves it alone is still compared like for like.
	 */
	char *ea = (char *)((uintptr_t)a + BUFSZ + 13);
	char *eb = (char *)((uintptr_t)b + BUFSZ + 13);

	errno = 0;
	quad_t ra = P::strtoq(a, use_endptr ? &ea : (char **)0, base);
	int erra = errno;

	errno = 0;
	quad_t rb = ref_strtoq(b, use_endptr ? &eb : (char **)0, base);
	int errb = errno;

	if (ra != rb) {
		std::snprintf(detail, sizeof(detail),
		    "input=\"%s\" base=%d ret port=%lld ref=%lld",
		    escape(in.data(), in.size()).c_str(), base,
		    (long long)ra, (long long)rb);
		fail(st_strtoq, detail);
		return;
	}
	if (erra != errb) {
		std::snprintf(detail, sizeof(detail),
		    "input=\"%s\" base=%d errno port=%d ref=%d",
		    escape(in.data(), in.size()).c_str(), base, erra, errb);
		fail(st_strtoq, detail);
		return;
	}
	if (use_endptr) {
		long oa = (long)((uintptr_t)ea - (uintptr_t)a);
		long ob = (long)((uintptr_t)eb - (uintptr_t)b);
		if (oa != ob) {
			std::snprintf(detail, sizeof(detail),
			    "input=\"%s\" base=%d endptr off port=%ld ref=%ld",
			    escape(in.data(), in.size()).c_str(), base, oa, ob);
			fail(st_strtoq, detail);
			return;
		}
	}
	if (std::memcmp(a, b, BUFSZ) != 0) {
		std::snprintf(detail, sizeof(detail),
		    "input=\"%s\" base=%d buffer diverged: port=\"%s\" ref=\"%s\"",
		    escape(in.data(), in.size()).c_str(), base,
		    escape(a, BUFSZ).c_str(), escape(b, BUFSZ).c_str());
		fail(st_strtoq, detail);
		return;
	}
}

static void
check_strtouq(const std::string &in, int base, bool use_endptr)
{
	char a[BUFSZ], b[BUFSZ];
	char detail[512];

	st_strtouq.cases++;

	std::memset(a, GUARD, BUFSZ);
	std::memset(b, GUARD, BUFSZ);

	size_t n = in.size();
	if (n > BUFSZ - 2)
		n = BUFSZ - 2;
	std::memcpy(a, in.data(), n);
	std::memcpy(b, in.data(), n);
	a[n] = '\0';
	b[n] = '\0';

	/*
	 * Seed endptr with an offset no legitimate write can produce, so an
	 * implementation that leaves it alone is still compared like for like.
	 */
	char *ea = (char *)((uintptr_t)a + BUFSZ + 13);
	char *eb = (char *)((uintptr_t)b + BUFSZ + 13);

	errno = 0;
	u_quad_t ra = P::strtouq(a, use_endptr ? &ea : (char **)0, base);
	int erra = errno;

	errno = 0;
	u_quad_t rb = ref_strtouq(b, use_endptr ? &eb : (char **)0, base);
	int errb = errno;

	if (ra != rb) {
		std::snprintf(detail, sizeof(detail),
		    "input=\"%s\" base=%d ret port=%llu ref=%llu",
		    escape(in.data(), in.size()).c_str(), base,
		    (unsigned long long)ra, (unsigned long long)rb);
		fail(st_strtouq, detail);
		return;
	}
	if (erra != errb) {
		std::snprintf(detail, sizeof(detail),
		    "input=\"%s\" base=%d errno port=%d ref=%d",
		    escape(in.data(), in.size()).c_str(), base, erra, errb);
		fail(st_strtouq, detail);
		return;
	}
	if (use_endptr) {
		long oa = (long)((uintptr_t)ea - (uintptr_t)a);
		long ob = (long)((uintptr_t)eb - (uintptr_t)b);
		if (oa != ob) {
			std::snprintf(detail, sizeof(detail),
			    "input=\"%s\" base=%d endptr off port=%ld ref=%ld",
			    escape(in.data(), in.size()).c_str(), base, oa, ob);
			fail(st_strtouq, detail);
			return;
		}
	}
	if (std::memcmp(a, b, BUFSZ) != 0) {
		std::snprintf(detail, sizeof(detail),
		    "input=\"%s\" base=%d buffer diverged: port=\"%s\" ref=\"%s\"",
		    escape(in.data(), in.size()).c_str(), base,
		    escape(a, BUFSZ).c_str(), escape(b, BUFSZ).c_str());
		fail(st_strtouq, detail);
		return;
	}
}

static void
check_both_strto(const std::string &in, int base)
{
	check_strtoq(in, base, true);
	check_strtoq(in, base, false);
	check_strtouq(in, base, true);
	check_strtouq(in, base, false);
}

/* ------------------------------------------------------- div/ldiv checking */

static void
check_div(int numer, int denom)
{
	char detail[256];

	if (denom == 0)
		return;			/* undefined behaviour, not testable */
	if (numer == INT_MIN && denom == -1)
		return;			/* undefined behaviour, not testable */

	st_div.cases++;

	div_t ra = P::div(numer, denom);
	div_t rb = ref_div(numer, denom);

	if (ra.quot != rb.quot || ra.rem != rb.rem) {
		std::snprintf(detail, sizeof(detail),
		    "div(%d, %d) port={quot=%d, rem=%d} ref={quot=%d, rem=%d}",
		    numer, denom, ra.quot, ra.rem, rb.quot, rb.rem);
		fail(st_div, detail);
	}
}

static void
check_ldiv(long numer, long denom)
{
	char detail[256];

	if (denom == 0)
		return;			/* undefined behaviour, not testable */
	if (numer == LONG_MIN && denom == -1)
		return;			/* undefined behaviour, not testable */

	st_ldiv.cases++;

	ldiv_t ra = P::ldiv(numer, denom);
	ldiv_t rb = ref_ldiv(numer, denom);

	if (ra.quot != rb.quot || ra.rem != rb.rem) {
		std::snprintf(detail, sizeof(detail),
		    "ldiv(%ld, %ld) port={quot=%ld, rem=%ld} ref={quot=%ld, rem=%ld}",
		    numer, denom, ra.quot, ra.rem, rb.quot, rb.rem);
		fail(st_ldiv, detail);
	}
}

/* ------------------------------------------------------ input generation - */

static std::string
fmt_base(unsigned long long v, int base, bool upper)
{
	const char *lo = "0123456789abcdefghijklmnopqrstuvwxyz";
	const char *up = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	const char *tab = upper ? up : lo;
	std::string s;

	if (base < 2 || base > 36)
		base = 10;
	if (v == 0)
		return std::string("0");
	while (v != 0) {
		s.push_back(tab[v % (unsigned long long)base]);
		v /= (unsigned long long)base;
	}
	for (size_t i = 0, j = s.size() - 1; i < j; i++, j--) {
		char t = s[i];
		s[i] = s[j];
		s[j] = t;
	}
	return s;
}

static const char *edge_strings[] = {
	"",
	" ",
	"\t",
	"\n",
	"\v",
	"\f",
	"\r",
	" \t\n\v\f\r ",
	"+",
	"-",
	"++1",
	"--1",
	"+-1",
	"-+1",
	"0",
	"1",
	"9",
	"a",
	"z",
	"A",
	"Z",
	"g",
	"G",
	"/",
	":",
	"@",
	"[",
	"`",
	"{",
	"-0",
	"+0",
	"00",
	"01",
	"07",
	"08",
	"09",
	"0b",
	"0B",
	"0o",
	"0x",
	"0X",
	"0x0",
	"0x1",
	"0X1",
	"0xg",
	"0xG",
	"0xf",
	"0xF",
	"-0x",
	"-0x1",
	"+0x7f",
	"  0x10",
	"   -12",
	"\t+42",
	"12abc",
	"12 34",
	"  12  ",
	"abc",
	" abc",
	"1.5",
	"1e5",
	"-1",
	"-9",
	"7fffffffffffffff",
	"8000000000000000",
	"ffffffffffffffff",
	"10000000000000000",
	"0x7fffffffffffffff",
	"0x8000000000000000",
	"0xffffffffffffffff",
	"0x10000000000000000",
	"-0x8000000000000000",
	"-0x8000000000000001",
	"-0xffffffffffffffff",
	"9223372036854775806",
	"9223372036854775807",
	"9223372036854775808",
	"9223372036854775809",
	"-9223372036854775807",
	"-9223372036854775808",
	"-9223372036854775809",
	"18446744073709551614",
	"18446744073709551615",
	"18446744073709551616",
	"-18446744073709551615",
	"-18446744073709551616",
	"99999999999999999999999999",
	"-99999999999999999999999999",
	"000000000000000000000000001",
	"0000000000000000000000000000",
	"2147483647",
	"2147483648",
	"-2147483648",
	"-2147483649",
	"4294967295",
	"4294967296",
	"zzzzzzzzzzzzz",
	"ZZZZZZZZZZZZZ",
	"zik0zj",			/* ULONG_MAX in base 36 fragment */
	"1y2p0ij32e8e7",		/* LLONG_MAX in base 36 */
	"3w5e11264sgsf",		/* ULLONG_MAX in base 36 */
	"-1y2p0ij32e8e8",
	"\x80",
	"\xff",
	"\x7f",
	"\x81\x82",
	"\xa0",
	"\xc2\xa0",
	"1\x80" "2",
	"\x80" "1",
	" \xff 1",
	"-\x80",
	"0x\x80",
	"\xb2",
	"\xef\xbb\xbf" "12",
	"1234567890123456789012345678901234567890",
	"-1234567890123456789012345678901234567890",
	"0x0000000000000000000000000000000000000001",
	"          1",
	"1          ",
};

static const int edge_bases[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 15, 16, 17, 20, 32, 33,
	35, 36, 37, 38, 100, -1, -2, -10, -36, INT_MAX, INT_MIN,
};

/* ------------------------------------------------------- edge case drivers */

static void
edge_cases_strto(void)
{
	size_t ns = sizeof(edge_strings) / sizeof(edge_strings[0]);
	size_t nb = sizeof(edge_bases) / sizeof(edge_bases[0]);

	for (size_t i = 0; i < ns; i++) {
		std::string s(edge_strings[i]);
		for (size_t j = 0; j < nb; j++)
			check_both_strto(s, edge_bases[j]);
	}

	/* Strings containing embedded NUL bytes. */
	{
		static const char e0[] = "1\0" "2";
		static const char e1[] = "\0" "123";
		static const char e2[] = "-\0" "1";
		static const char e3[] = "0x\0" "10";
		static const char e4[] = " \0 1";
		const char *embed[] = { e0, e1, e2, e3, e4 };
		const size_t elen[] = { sizeof(e0) - 1, sizeof(e1) - 1,
		    sizeof(e2) - 1, sizeof(e3) - 1, sizeof(e4) - 1 };

		for (size_t i = 0; i < 5; i++) {
			std::string s(embed[i], elen[i]);
			for (size_t j = 0; j < nb; j++)
				check_both_strto(s, edge_bases[j]);
		}
	}

	/* Every single byte value 0x00-0xff on its own, and as a suffix. */
	for (int c = 0; c <= 0xff; c++) {
		std::string one(1, (char)(unsigned char)c);
		std::string two = std::string("1") + one;
		std::string three = one + std::string("1");
		check_both_strto(one, 0);
		check_both_strto(one, 10);
		check_both_strto(one, 16);
		check_both_strto(one, 36);
		check_both_strto(two, 0);
		check_both_strto(two, 10);
		check_both_strto(two, 16);
		check_both_strto(two, 36);
		check_both_strto(three, 0);
		check_both_strto(three, 10);
		check_both_strto(three, 16);
		check_both_strto(three, 36);
	}

	/* Boundary lengths: 0..40 digits of '9', both signs. */
	for (size_t len = 0; len <= 40; len++) {
		std::string s(len, '9');
		check_both_strto(s, 10);
		check_both_strto(std::string("-") + s, 10);
		check_both_strto(std::string("+") + s, 10);
		std::string f(len, 'f');
		check_both_strto(f, 16);
		check_both_strto(std::string("0x") + f, 0);
		check_both_strto(std::string("-0x") + f, 16);
		std::string z(len, 'z');
		check_both_strto(z, 36);
		std::string zero(len, '0');
		check_both_strto(zero, 0);
		check_both_strto(zero + "1", 0);
	}

	/* Exact overflow boundaries per base, both signed and unsigned. */
	for (int base = 2; base <= 36; base++) {
		unsigned long long marks[] = {
			0ULL,
			1ULL,
			(unsigned long long)LLONG_MAX - 1,
			(unsigned long long)LLONG_MAX,
			(unsigned long long)LLONG_MAX + 1,
			(unsigned long long)LLONG_MAX + 2,
			ULLONG_MAX - 1,
			ULLONG_MAX,
		};
		for (size_t k = 0; k < sizeof(marks) / sizeof(marks[0]); k++) {
			std::string s = fmt_base(marks[k], base, false);
			std::string u = fmt_base(marks[k], base, true);
			check_both_strto(s, base);
			check_both_strto(u, base);
			check_both_strto(std::string("-") + s, base);
			check_both_strto(std::string("+") + s, base);
			check_both_strto(s + "0", base);
			check_both_strto(std::string("-") + s + "0", base);
		}
	}
}

static void
edge_cases_div(void)
{
	static const int vals[] = {
		0, 1, -1, 2, -2, 3, -3, 4, -4, 5, -5, 6, -6, 7, -7, 8, -8,
		9, -9, 10, -10, 11, -11, 16, -16, 17, -17, 99, -99, 100,
		-100, 1000, -1000, 65535, -65535, 65536, -65536,
		1000000, -1000000,
		INT_MAX, INT_MAX - 1, INT_MIN, INT_MIN + 1,
		INT_MAX / 2, INT_MIN / 2,
	};
	size_t n = sizeof(vals) / sizeof(vals[0]);

	for (size_t i = 0; i < n; i++)
		for (size_t j = 0; j < n; j++)
			check_div(vals[i], vals[j]);
}

static void
edge_cases_ldiv(void)
{
	static const long vals[] = {
		0L, 1L, -1L, 2L, -2L, 3L, -3L, 4L, -4L, 5L, -5L, 6L, -6L,
		7L, -7L, 8L, -8L, 9L, -9L, 10L, -10L, 11L, -11L, 16L, -16L,
		17L, -17L, 99L, -99L, 100L, -100L, 1000L, -1000L,
		65535L, -65535L, 65536L, -65536L,
		2147483647L, -2147483647L, -2147483648L, 2147483648L,
		4294967295L, -4294967295L, 4294967296L, -4294967296L,
		LONG_MAX, LONG_MAX - 1, LONG_MIN, LONG_MIN + 1,
		LONG_MAX / 2, LONG_MIN / 2,
	};
	size_t n = sizeof(vals) / sizeof(vals[0]);

	for (size_t i = 0; i < n; i++)
		for (size_t j = 0; j < n; j++)
			check_ldiv(vals[i], vals[j]);
}

/* ---------------------------------------------------------- random sweeps */

static std::string
random_string(void)
{
	static const char alpha[] =
	    "0123456789abcdefghijklmnopqrstuvwxyz"
	    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	    "  \t\n\v\f\r++--xX00zZ.,;:/@[`{|}~!#$%^&*()_=";
	static const size_t nalpha = sizeof(alpha) - 1;
	std::string s;
	uint32_t mode = rng_below(5);

	switch (mode) {
	case 0: {
		/* Arbitrary bytes over the full 0x00-0xff range. */
		uint32_t len = rng_below(24);
		for (uint32_t i = 0; i < len; i++)
			s.push_back((char)(unsigned char)rng_below(256));
		break;
	}
	case 1: {
		/* Numeric-ish soup drawn from a digit/sign/space alphabet. */
		uint32_t len = rng_below(24);
		for (uint32_t i = 0; i < len; i++)
			s.push_back(alpha[rng_below((uint32_t)nalpha)]);
		break;
	}
	case 2: {
		/* Well-formed number in a random base, random decorations. */
		unsigned long long v = rng_next();
		uint32_t shift = rng_below(65);
		if (shift < 64)
			v >>= shift;
		int base = 2 + (int)rng_below(35);
		bool upper = rng_below(2) != 0;
		uint32_t ws = rng_below(4);
		for (uint32_t i = 0; i < ws; i++)
			s.push_back(" \t\n\r"[rng_below(4)]);
		uint32_t sign = rng_below(3);
		if (sign == 1)
			s.push_back('-');
		else if (sign == 2)
			s.push_back('+');
		if (base == 16 && rng_below(2))
			s += (upper ? "0X" : "0x");
		if (base == 8 && rng_below(2))
			s.push_back('0');
		s += fmt_base(v, base, upper);
		uint32_t trail = rng_below(4);
		for (uint32_t i = 0; i < trail; i++)
			s.push_back(alpha[rng_below((uint32_t)nalpha)]);
		break;
	}
	case 3: {
		/* Values hugging the signed/unsigned 64-bit boundaries. */
		static const unsigned long long anchors[] = {
			0ULL, 1ULL,
			(unsigned long long)INT_MAX,
			(unsigned long long)INT_MAX + 1,
			0xFFFFFFFFULL, 0x100000000ULL,
			(unsigned long long)LLONG_MAX - 2,
			(unsigned long long)LLONG_MAX - 1,
			(unsigned long long)LLONG_MAX,
			(unsigned long long)LLONG_MAX + 1,
			(unsigned long long)LLONG_MAX + 2,
			ULLONG_MAX - 2, ULLONG_MAX - 1, ULLONG_MAX,
		};
		size_t na = sizeof(anchors) / sizeof(anchors[0]);
		unsigned long long v = anchors[rng_below((uint32_t)na)];
		uint32_t d = rng_below(5);
		if (d == 1)
			v += 1;
		else if (d == 2)
			v -= 1;
		static const int bases[] = { 2, 8, 10, 16, 36 };
		int base = bases[rng_below(5)];
		bool upper = rng_below(2) != 0;
		uint32_t sign = rng_below(3);
		if (sign == 1)
			s.push_back('-');
		else if (sign == 2)
			s.push_back('+');
		if (base == 16 && rng_below(2))
			s += (upper ? "0X" : "0x");
		s += fmt_base(v, base, upper);
		break;
	}
	default: {
		/* Very short strings, heavy on high-bit and control bytes. */
		uint32_t len = rng_below(4);
		for (uint32_t i = 0; i < len; i++) {
			if (rng_below(2))
				s.push_back((char)(unsigned char)
				    (0x80 + rng_below(128)));
			else
				s.push_back(alpha[rng_below((uint32_t)nalpha)]);
		}
		break;
	}
	}
	return s;
}

static int
random_base(void)
{
	switch (rng_below(8)) {
	case 0:
		return 0;
	case 1:
		return 10;
	case 2:
		return 16;
	case 3:
		return 8;
	case 4:
		return 2;
	case 5:
		return 36;
	case 6:
		return (int)rng_below(40) - 2;	/* -2 .. 37 */
	default:
		return (int)rng_below(37);	/* 0 .. 36 */
	}
}

static const unsigned long long STRTO_ITERS = 200000;
static const unsigned long long DIV_ITERS = 220000;

static void
random_sweep_strto(void)
{
	for (unsigned long long i = 0; i < STRTO_ITERS; i++) {
		std::string s = random_string();
		int base = random_base();
		bool ep = (rng_below(4) != 0);
		check_strtoq(s, base, ep);
		check_strtouq(s, base, ep);
	}
}

static int
random_int(void)
{
	uint64_t r = rng_next();
	switch (r & 3) {
	case 0:
		return (int)(int32_t)(r >> 8);
	case 1:
		return (int)(int32_t)((r >> 8) % 1000) - 500;
	case 2:
		return (int)(int32_t)((r >> 8) % 65536) - 32768;
	default:
		return (int)(int32_t)(r >> 32);
	}
}

static long
random_long(void)
{
	uint64_t r = rng_next();
	switch (r & 3) {
	case 0:
		return (long)(int64_t)rng_next();
	case 1:
		return (long)(int64_t)((rng_next() >> 8) % 1000) - 500;
	case 2:
		return (long)(int32_t)(rng_next() >> 16);
	default:
		return (long)(int64_t)(rng_next() >> (rng_next() % 63));
	}
}

static void
random_sweep_div(void)
{
	for (unsigned long long i = 0; i < DIV_ITERS; i++) {
		int numer = random_int();
		int denom = random_int();
		if (denom == 0)
			denom = 1 + (int)rng_below(7);
		check_div(numer, denom);
		/* Small divisors give non-zero remainders of both signs. */
		int small = (int)rng_below(19) - 9;
		if (small == 0)
			small = 9;
		check_div(numer, small);
		check_div(small, denom);
	}
}

static void
random_sweep_ldiv(void)
{
	for (unsigned long long i = 0; i < DIV_ITERS; i++) {
		long numer = random_long();
		long denom = random_long();
		if (denom == 0)
			denom = 1L + (long)rng_below(7);
		check_ldiv(numer, denom);
		long small = (long)rng_below(19) - 9L;
		if (small == 0)
			small = 9L;
		check_ldiv(numer, small);
		check_ldiv(small, denom);
	}
}

/* --------------------------------------------------------------- reporting */

static void
print_row(const Stat &s)
{
	std::printf("  %-10s %14llu %14llu  %s\n", s.name, s.cases, s.fails,
	    s.fails == 0 ? "ok" : "FAILED");
}

int
main(void)
{
	rng_seed(0x0123456789ABCDEFULL);

	edge_cases_strto();
	edge_cases_div();
	edge_cases_ldiv();

	random_sweep_strto();
	random_sweep_div();
	random_sweep_ldiv();

	std::printf("\n  %-10s %14s %14s  %s\n", "function", "cases",
	    "failures", "status");
	std::printf("  ---------------------------------------------------\n");
	print_row(st_strtoq);
	print_row(st_strtouq);
	print_row(st_div);
	print_row(st_ldiv);

	unsigned long long total_cases = st_strtoq.cases + st_strtouq.cases +
	    st_div.cases + st_ldiv.cases;
	unsigned long long total_fails = st_strtoq.fails + st_strtouq.fails +
	    st_div.fails + st_ldiv.fails;

	std::printf("  ---------------------------------------------------\n");
	std::printf("  %-10s %14llu %14llu  %s\n", "TOTAL", total_cases,
	    total_fails, total_fails == 0 ? "ok" : "FAILED");

	if (total_fails != 0)
		return 1;
	return 0;
}
