/*
 * Differential test for batch b0069 (atoi, atol, atof, atoll and their
 * locale_t variants): every case is run through the C++23 port and through the
 * unmodified C oracle, and the results are compared bit for bit.
 */

#include <cerrno>
#include <climits>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <locale.h>

import pbsd.lib.libc.stdlib.b0069;

namespace P = pbsd::lib_libc_stdlib::b0069;

extern "C" {
int ref_atoi(const char *str);
int ref_atoi_l(const char *str, locale_t locale);
long ref_atol(const char *str);
long ref_atol_l(const char *str, locale_t locale);
double ref_atof(const char *ascii);
double ref_atof_l(const char *ascii, locale_t locale);
long long ref_atoll(const char *str);
long long ref_atoll_l(const char *str, locale_t locale);
}

enum {
	F_ATOI, F_ATOI_L,
	F_ATOL, F_ATOL_L,
	F_ATOF, F_ATOF_L,
	F_ATOLL, F_ATOLL_L,
	F_COUNT
};

static const char *const fn_name[F_COUNT] = {
	"atoi", "atoi_l",
	"atol", "atol_l",
	"atof", "atof_l",
	"atoll", "atoll_l"
};

static unsigned long long n_cases[F_COUNT];
static unsigned long long n_fails[F_COUNT];

/*
 * Guarded buffers: the subject string is placed at LEAD_GUARD bytes into a
 * buffer that is otherwise filled with the guard byte 0x7f.  Two independent
 * buffers are used so the port and the oracle can never observe each other,
 * and the whole buffer -- lead guard, string, NUL, and every byte past the
 * nominal window -- is compared afterwards.
 */
#define B0069_MAX_INPUT	512u
#define LEAD_GUARD	16u
#define TRAIL_GUARD	16u
#define BUF_SIZE	(LEAD_GUARD + B0069_MAX_INPUT + 1u + TRAIL_GUARD)
#define GUARD_BYTE	0x7f

struct Ret {
	unsigned long long bits;
	int err;
};

static Ret
call_port(int fn, const char *s, locale_t loc)
{
	Ret r;
	unsigned long long bits = 0;
	double d;

	errno = 0;
	switch (fn) {
	case F_ATOI:
		bits = (unsigned long long)(unsigned int)P::atoi(s);
		break;
	case F_ATOI_L:
		bits = (unsigned long long)(unsigned int)P::atoi_l(s, loc);
		break;
	case F_ATOL:
		bits = (unsigned long long)(unsigned long)P::atol(s);
		break;
	case F_ATOL_L:
		bits = (unsigned long long)(unsigned long)P::atol_l(s, loc);
		break;
	case F_ATOF:
		d = P::atof(s);
		std::memcpy(&bits, &d, sizeof(d));
		break;
	case F_ATOF_L:
		d = P::atof_l(s, loc);
		std::memcpy(&bits, &d, sizeof(d));
		break;
	case F_ATOLL:
		bits = (unsigned long long)P::atoll(s);
		break;
	case F_ATOLL_L:
		bits = (unsigned long long)P::atoll_l(s, loc);
		break;
	}
	r.err = errno;
	r.bits = bits;
	return r;
}

static Ret
call_ref(int fn, const char *s, locale_t loc)
{
	Ret r;
	unsigned long long bits = 0;
	double d;

	errno = 0;
	switch (fn) {
	case F_ATOI:
		bits = (unsigned long long)(unsigned int)ref_atoi(s);
		break;
	case F_ATOI_L:
		bits = (unsigned long long)(unsigned int)ref_atoi_l(s, loc);
		break;
	case F_ATOL:
		bits = (unsigned long long)(unsigned long)ref_atol(s);
		break;
	case F_ATOL_L:
		bits = (unsigned long long)(unsigned long)ref_atol_l(s, loc);
		break;
	case F_ATOF:
		d = ref_atof(s);
		std::memcpy(&bits, &d, sizeof(d));
		break;
	case F_ATOF_L:
		d = ref_atof_l(s, loc);
		std::memcpy(&bits, &d, sizeof(d));
		break;
	case F_ATOLL:
		bits = (unsigned long long)ref_atoll(s);
		break;
	case F_ATOLL_L:
		bits = (unsigned long long)ref_atoll_l(s, loc);
		break;
	}
	r.err = errno;
	r.bits = bits;
	return r;
}

static unsigned long long reported;

static void
show_bytes(const char *d, size_t n)
{
	size_t i;

	std::fputc('"', stdout);
	for (i = 0; i < n; i++) {
		unsigned char c = (unsigned char)d[i];

		if (c == '\\' || c == '"')
			std::printf("\\%c", c);
		else if (c >= 0x20 && c < 0x7f)
			std::fputc((int)c, stdout);
		else
			std::printf("\\x%02x", (unsigned)c);
	}
	std::fputc('"', stdout);
}

static void
report(int fn, const char *d, size_t n, const char *what, const char *lname,
    unsigned long long pv, unsigned long long rv)
{
	if (reported >= 40u) {
		if (reported == 40u)
			std::printf("  ... further mismatches suppressed\n");
		reported++;
		return;
	}
	reported++;
	std::printf("  MISMATCH %s [%s] locale=%s input=", fn_name[fn], what,
	    lname);
	show_bytes(d, n);
	std::printf(" len=%zu port=0x%016llx ref=0x%016llx\n", n, pv, rv);
}

/*
 * Run one input through one function on both implementations.  Returns true on
 * a match.
 */
static bool
check_one(int fn, const char *in, size_t len, locale_t loc, const char *lname)
{
	unsigned char pristine[BUF_SIZE];
	unsigned char bufp[BUF_SIZE];
	unsigned char bufr[BUF_SIZE];
	Ret rp, rr;
	bool ok = true;

	if (len > B0069_MAX_INPUT) {
		std::printf("  internal: input too long (%zu)\n", len);
		std::exit(2);
	}

	std::memset(bufp, GUARD_BYTE, sizeof(bufp));
	std::memcpy(bufp + LEAD_GUARD, in, len);
	bufp[LEAD_GUARD + len] = '\0';
	std::memcpy(bufr, bufp, sizeof(bufp));
	std::memcpy(pristine, bufp, sizeof(bufp));

	rp = call_port(fn, (const char *)bufp + LEAD_GUARD, loc);
	rr = call_ref(fn, (const char *)bufr + LEAD_GUARD, loc);

	n_cases[fn]++;

	if (rp.bits != rr.bits) {
		report(fn, in, len, "return", lname, rp.bits, rr.bits);
		ok = false;
	}
	if (rp.err != rr.err) {
		report(fn, in, len, "errno", lname,
		    (unsigned long long)(unsigned int)rp.err,
		    (unsigned long long)(unsigned int)rr.err);
		ok = false;
	}
	if (std::memcmp(bufp, bufr, sizeof(bufp)) != 0) {
		report(fn, in, len, "buffers-differ", lname, 0, 0);
		ok = false;
	}
	if (std::memcmp(bufp, pristine, sizeof(bufp)) != 0) {
		report(fn, in, len, "port-wrote-buffer", lname, 0, 0);
		ok = false;
	}
	if (std::memcmp(bufr, pristine, sizeof(bufr)) != 0) {
		report(fn, in, len, "ref-wrote-buffer", lname, 0, 0);
		ok = false;
	}
	if (!ok)
		n_fails[fn]++;
	return ok;
}

static locale_t loc_c;
static locale_t loc_posix;
static unsigned long long case_seq;

static void
check_all(const char *in, size_t len)
{
	locale_t loc;
	const char *lname;
	int fn;

	/* Alternate locales so both newlocale() objects are exercised. */
	if ((case_seq++ & 1u) != 0u) {
		loc = loc_c;
		lname = "C";
	} else {
		loc = loc_posix;
		lname = "POSIX";
	}
	for (fn = 0; fn < F_COUNT; fn++)
		(void)check_one(fn, in, len, loc, lname);
}

static void
check_str(const char *s)
{
	check_all(s, std::strlen(s));
}

/* ------------------------------------------------------------------ */
/* Hand-written edge cases                                            */
/* ------------------------------------------------------------------ */

struct Lit {
	const char *d;
	size_t n;
};

#define L(s) { s, sizeof(s) - 1 }

static const Lit edge_lits[] = {
	/* empty and single characters */
	L(""), L(" "), L("\t"), L("\n"), L("\v"), L("\f"), L("\r"),
	L("0"), L("1"), L("2"), L("5"), L("8"), L("9"),
	L("-"), L("+"), L("."), L(","), L("e"), L("E"), L("x"), L("X"),
	L("a"), L("f"), L("g"), L("i"), L("n"), L("N"), L("("), L(")"),
	L("_"), L("/"), L(":"), L("\x7f"), L("\x80"), L("\xff"), L("\x01"),

	/* plain decimals and sign handling */
	L("0"), L("-0"), L("+0"), L("00"), L("000000000000000000000"),
	L("007"), L("09"), L("-09"), L("1"), L("-1"), L("+1"),
	L("42"), L("-42"), L("+42"), L("  42"), L("\t\n\v\f\r 42"),
	L("- 42"), L("+ 42"), L("--42"), L("++42"), L("+-42"), L("-+42"),
	L("4-2"), L("4+2"), L("4 2"), L("42 43"), L("42abc"), L("abc42"),
	L(" -  42"), L("-\t42"),

	/* base-10 vs base-0/base-16 discrimination (a mutated base would
	   change these) */
	L("0x10"), L("0X10"), L("-0x10"), L("0x"), L("0X"), L("0xg"),
	L("010"), L("-010"), L("0b101"), L("0B1"), L("08"), L("0777"),
	L("1a"), L("ff"), L("FF"), L("0xff"), L("0xFF"), L("0x7fffffff"),
	L("0x80000000"), L("0xffffffff"), L("0x7fffffffffffffff"),
	L("0x8000000000000000"), L("0xffffffffffffffff"),
	L("0x10000000000000000"),

	/* int boundaries */
	L("2147483646"), L("2147483647"), L("2147483648"), L("2147483649"),
	L("-2147483647"), L("-2147483648"), L("-2147483649"),
	L("-2147483650"),
	/* values that only differ once truncated to int */
	L("4294967295"), L("4294967296"), L("4294967297"),
	L("-4294967295"), L("-4294967296"), L("-4294967297"),
	L("8589934592"), L("4294967298"), L("6442450944"),

	/* long / long long boundaries */
	L("9223372036854775806"), L("9223372036854775807"),
	L("9223372036854775808"), L("9223372036854775809"),
	L("-9223372036854775807"), L("-9223372036854775808"),
	L("-9223372036854775809"), L("-9223372036854775810"),
	L("18446744073709551614"), L("18446744073709551615"),
	L("18446744073709551616"), L("18446744073709551617"),
	L("-18446744073709551615"), L("-18446744073709551616"),

	/* far out of range */
	L("99999999999999999999999999999999"),
	L("-99999999999999999999999999999999"),
	L("100000000000000000000000000000000000000000"),
	L("0000000000000000000000000000000000000000009"),
	L("00000000000000000000002147483648"),

	/* floating point forms */
	L(".5"), L("5."), L("."), L("-."), L(".e1"), L("0."), L("-0.0"),
	L("1.5"), L("-1.5"), L("2.5"), L("1.9999999999999999"),
	L("0.1"), L("0.2"), L("0.30000000000000004"),
	L("1e3"), L("1E3"), L("1e+3"), L("1e-3"), L("1e"), L("1e+"),
	L("1e-"), L("1ee3"), L("1e3.5"), L("1.5e"), L("e3"), L("+e3"),
	L("1e308"), L("1e309"), L("1e-308"), L("1e-323"), L("1e-324"),
	L("1e-400"), L("1e999"), L("-1e999"), L("1e99999999999999999999"),
	L("1e-99999999999999999999"),
	L("1.7976931348623157e308"), L("1.7976931348623159e308"),
	L("2.2250738585072014e-308"), L("2.2250738585072011e-308"),
	L("4.9406564584124654e-324"), L("2.4703282292062327e-324"),
	L("2.4703282292062328e-324"),
	L("inf"), L("INF"), L("Inf"), L("infinity"), L("INFINITY"),
	L("infinit"), L("-inf"), L("+inf"), L("-infinity"),
	L("nan"), L("NAN"), L("NaN"), L("-nan"), L("+nan"),
	L("nan("), L("nan()"), L("nan(0x1)"), L("nan(abc"), L("nan(abc)"),
	L("0x1p3"), L("0X1P-3"), L("0x1p"), L("0x.8p1"), L("0x1.8p+1"),
	L("0x10"), L("0x0p0"), L("0xp1"),

	/* high-bit bytes adjacent to digits */
	L("\x80" "42"), L("42\x80"), L("\xff" "1"), L("1\xff"),
	L("\xa0" "1"), L("1\xa0"), L("\xc2\xa0" "1"), L("\x85" "1"),
	L("\x80\x80\x80"), L("\xff\xff\xff\xff"),
	L("-\x80" "1"), L("\x80" "-1"), L("\x7f" "42"), L("42\x7f"),

	/* NUL-heavy (embedded NULs must terminate parsing) */
	{ "\0", 1 },
	{ "\0" "42", 3 },
	{ "4" "\0" "2", 3 },
	{ "42" "\0", 3 },
	{ "42" "\0" "99", 5 },
	{ "\0\0\0\0", 4 },
	{ " " "\0" "42", 4 },
	{ "-" "\0" "42", 4 },
	{ "1.5" "\0" "e9", 6 },
	{ "1e" "\0" "3", 4 },
	{ "0x" "\0" "10", 5 },
	{ "in" "\0" "f", 4 },
	{ "na" "\0" "n", 4 },
	{ "\x80" "\0" "1", 3 },
	{ "2147483647" "\0" "8", 12 },
};

static void
run_edge_cases(void)
{
	size_t i;
	unsigned v;
	char buf[B0069_MAX_INPUT + 1];

	for (i = 0; i < sizeof(edge_lits) / sizeof(edge_lits[0]); i++)
		check_all(edge_lits[i].d, edge_lits[i].n);

	/* Every single byte value on its own. */
	for (v = 0; v < 256u; v++) {
		buf[0] = (char)(unsigned char)v;
		check_all(buf, 1);
	}
	/* Every byte value before, inside and after a digit run. */
	for (v = 0; v < 256u; v++) {
		buf[0] = (char)(unsigned char)v;
		buf[1] = '1';
		buf[2] = '2';
		check_all(buf, 3);

		buf[0] = '1';
		buf[1] = (char)(unsigned char)v;
		buf[2] = '2';
		check_all(buf, 3);

		buf[0] = '1';
		buf[1] = '2';
		buf[2] = (char)(unsigned char)v;
		check_all(buf, 3);

		buf[0] = '-';
		buf[1] = (char)(unsigned char)v;
		buf[2] = '7';
		check_all(buf, 3);

		buf[0] = '1';
		buf[1] = '.';
		buf[2] = (char)(unsigned char)v;
		buf[3] = '5';
		check_all(buf, 4);

		buf[0] = '1';
		buf[1] = 'e';
		buf[2] = (char)(unsigned char)v;
		buf[3] = '2';
		check_all(buf, 4);
	}

	/* Boundary lengths: runs of '9' and of '0' from 0 to 40 bytes, and
	   1 followed by n zeros (crosses every power-of-ten boundary). */
	for (v = 0; v <= 40u; v++) {
		size_t k;

		for (k = 0; k < v; k++)
			buf[k] = '9';
		check_all(buf, v);

		for (k = 0; k < v; k++)
			buf[k] = '0';
		check_all(buf, v);

		buf[0] = '1';
		for (k = 1; k <= v; k++)
			buf[k] = '0';
		check_all(buf, v + 1);

		buf[0] = '-';
		buf[1] = '1';
		for (k = 2; k <= v + 1; k++)
			buf[k] = '0';
		check_all(buf, v + 2);

		for (k = 0; k < v; k++)
			buf[k] = ' ';
		buf[v] = '7';
		check_all(buf, v + 1);
	}
	/* Very long inputs, right up against the buffer. */
	{
		size_t k;

		for (k = 0; k < B0069_MAX_INPUT; k++)
			buf[k] = (char)('0' + (int)(k % 10u));
		check_all(buf, B0069_MAX_INPUT);

		buf[0] = '-';
		check_all(buf, B0069_MAX_INPUT);

		for (k = 0; k < B0069_MAX_INPUT; k++)
			buf[k] = '0';
		buf[B0069_MAX_INPUT - 1] = '5';
		check_all(buf, B0069_MAX_INPUT);
	}
}

/* ------------------------------------------------------------------ */
/* Randomised sweep                                                   */
/* ------------------------------------------------------------------ */

static unsigned long long rng_state = 0x0069b0069ULL;

static unsigned long long
rnd(void)
{
	unsigned long long z;

	rng_state += 0x9e3779b97f4a7c15ULL;
	z = rng_state;
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return z ^ (z >> 31);
}

static unsigned
rnd_below(unsigned n)
{
	return (unsigned)(rnd() % (unsigned long long)n);
}

/* Alphabet biased towards bytes that matter to strtol/strtod, but still
   including the full 0x00-0xff range. */
static const char soup_alpha[] =
	"0123456789" "0123456789" "0123456789"
	"-+.eE" "-+.eE"
	"xXpP" "abcdefABCDEF" "infINFtyNaAn()" " \t\n\v\f\r" " \t"
	"_,;:/'\"" "0123456789";

static size_t
gen_soup(char *buf)
{
	size_t n = (size_t)rnd_below(25u);
	size_t i;

	for (i = 0; i < n; i++) {
		unsigned pick = rnd_below(16u);

		if (pick == 0u)
			buf[i] = (char)(unsigned char)rnd_below(256u);
		else if (pick == 1u) {
			static const unsigned char hi[] = {
				0x00, 0x7f, 0x80, 0x81, 0x85, 0xa0, 0xc2,
				0xef, 0xfe, 0xff
			};
			buf[i] = (char)hi[rnd_below((unsigned)(sizeof(hi)))];
		} else
			buf[i] = soup_alpha[rnd_below(
			    (unsigned)(sizeof(soup_alpha) - 1u))];
	}
	return n;
}

static size_t
fmt_u128(char *out, unsigned __int128 v)
{
	char tmp[64];
	size_t n = 0, i;

	if (v == 0) {
		out[0] = '0';
		return 1;
	}
	while (v != 0) {
		tmp[n++] = (char)('0' + (int)(unsigned)(v % 10u));
		v /= 10u;
	}
	for (i = 0; i < n; i++)
		out[i] = tmp[n - 1u - i];
	return n;
}

static const unsigned __int128 bounds[] = {
	0u,
	1u,
	9u,
	10u,
	127u,
	128u,
	255u,
	32767u,
	32768u,
	65535u,
	2147483647u,
	2147483648u,
	4294967295u,
	4294967296u,
	(unsigned __int128)9223372036854775807ULL,
	(unsigned __int128)9223372036854775807ULL + 1u,
	(unsigned __int128)18446744073709551615ULL,
	(unsigned __int128)18446744073709551615ULL + 1u,
	((unsigned __int128)1 << 96),
};

static size_t
gen_boundary(char *buf)
{
	unsigned __int128 base = bounds[rnd_below(
	    (unsigned)(sizeof(bounds) / sizeof(bounds[0])))];
	int delta = (int)rnd_below(9u) - 4;
	size_t n = 0;
	unsigned k, zeros, ws;

	if (delta < 0) {
		unsigned __int128 mag = (unsigned __int128)(-delta);

		if (base >= mag)
			base -= mag;
		else
			base = 0u;
	} else
		base += (unsigned __int128)delta;

	ws = rnd_below(4u);
	for (k = 0; k < ws; k++) {
		static const char sp[] = " \t\n\v\f\r";

		buf[n++] = sp[rnd_below((unsigned)(sizeof(sp) - 1u))];
	}
	switch (rnd_below(4u)) {
	case 0:
		buf[n++] = '-';
		break;
	case 1:
		buf[n++] = '+';
		break;
	default:
		break;
	}
	zeros = rnd_below(4u);
	for (k = 0; k < zeros; k++)
		buf[n++] = '0';
	n += fmt_u128(buf + n, base);
	switch (rnd_below(8u)) {
	case 0:
		buf[n++] = '.';
		buf[n++] = '5';
		break;
	case 1:
		buf[n++] = 'e';
		buf[n++] = '1';
		break;
	case 2:
		buf[n++] = 'x';
		break;
	case 3:
		buf[n++] = ' ';
		buf[n++] = '9';
		break;
	default:
		break;
	}
	return n;
}

static size_t
gen_float(char *buf)
{
	size_t n = 0;
	unsigned k, d;

	for (k = rnd_below(3u); k > 0u; k--) {
		static const char sp[] = " \t\n";

		buf[n++] = sp[rnd_below((unsigned)(sizeof(sp) - 1u))];
	}
	if (rnd_below(3u) == 0u)
		buf[n++] = (rnd_below(2u) != 0u) ? '-' : '+';

	switch (rnd_below(8u)) {
	case 0: {
		static const char *const w[] = {
			"inf", "INF", "infinity", "INFINITY", "Infinity",
			"nan", "NAN", "nan(1)", "nan(0x7)", "nan(z"
		};
		const char *p = w[rnd_below(10u)];

		while (*p != '\0')
			buf[n++] = *p++;
		break;
	}
	case 1: {
		/* hexadecimal float: strtod accepts it, strtol stops at 'x' */
		buf[n++] = '0';
		buf[n++] = (rnd_below(2u) != 0u) ? 'x' : 'X';
		for (d = 1u + rnd_below(6u); d > 0u; d--)
			buf[n++] = "0123456789abcdefABCDEF"[rnd_below(22u)];
		if (rnd_below(2u) != 0u) {
			buf[n++] = '.';
			for (d = rnd_below(5u); d > 0u; d--)
				buf[n++] =
				    "0123456789abcdef"[rnd_below(16u)];
		}
		if (rnd_below(4u) != 0u) {
			buf[n++] = (rnd_below(2u) != 0u) ? 'p' : 'P';
			if (rnd_below(2u) != 0u)
				buf[n++] = (rnd_below(2u) != 0u) ? '-' : '+';
			n += fmt_u128(buf + n,
			    (unsigned __int128)rnd_below(1200u));
		}
		break;
	}
	default: {
		unsigned intd = rnd_below(22u);
		unsigned frac = rnd_below(22u);

		for (d = 0; d < intd; d++)
			buf[n++] = (char)('0' + (int)rnd_below(10u));
		if (rnd_below(4u) != 0u) {
			buf[n++] = '.';
			for (d = 0; d < frac; d++)
				buf[n++] = (char)('0' + (int)rnd_below(10u));
		}
		if (rnd_below(2u) != 0u) {
			buf[n++] = (rnd_below(2u) != 0u) ? 'e' : 'E';
			if (rnd_below(3u) != 0u)
				buf[n++] = (rnd_below(2u) != 0u) ? '-' : '+';
			switch (rnd_below(4u)) {
			case 0:
				n += fmt_u128(buf + n,
				    (unsigned __int128)(300u +
				    rnd_below(60u)));
				break;
			case 1:
				n += fmt_u128(buf + n,
				    (unsigned __int128)(rnd_below(400u)));
				break;
			case 2:
				n += fmt_u128(buf + n,
				    (unsigned __int128)(rnd_below(100000u)));
				break;
			default:
				break;
			}
		}
		break;
	}
	}
	if (rnd_below(8u) == 0u) {
		buf[n++] = (char)(unsigned char)(0x80u + rnd_below(0x80u));
		buf[n++] = (char)('0' + (int)rnd_below(10u));
	}
	return n;
}

static size_t
gen_digits(char *buf)
{
	size_t n = 0;
	unsigned len = 1u + rnd_below(45u);
	unsigned k;

	if (rnd_below(3u) == 0u)
		buf[n++] = (rnd_below(2u) != 0u) ? '-' : '+';
	for (k = 0; k < len; k++)
		buf[n++] = (char)('0' + (int)rnd_below(10u));
	return n;
}

static size_t
gen_nul_mix(char *buf)
{
	size_t n = 0;
	unsigned len = rnd_below(14u);
	unsigned k;

	for (k = 0; k < len; k++) {
		if (rnd_below(5u) == 0u)
			buf[n++] = '\0';
		else
			buf[n++] = soup_alpha[rnd_below(
			    (unsigned)(sizeof(soup_alpha) - 1u))];
	}
	return n;
}

#define SWEEP_ITERS 260000u

static void
run_random_sweep(void)
{
	char buf[B0069_MAX_INPUT + 1];
	unsigned i;

	for (i = 0; i < SWEEP_ITERS; i++) {
		size_t n;

		switch (i % 5u) {
		case 0:
			n = gen_soup(buf);
			break;
		case 1:
			n = gen_boundary(buf);
			break;
		case 2:
			n = gen_float(buf);
			break;
		case 3:
			n = gen_digits(buf);
			break;
		default:
			n = gen_nul_mix(buf);
			break;
		}
		check_all(buf, n);
	}
}

int
main(void)
{
	int fn;
	unsigned long long total_cases = 0, total_fails = 0;

	loc_c = newlocale(LC_ALL_MASK, "C", (locale_t)0);
	loc_posix = newlocale(LC_ALL_MASK, "POSIX", (locale_t)0);
	if (loc_c == (locale_t)0 || loc_posix == (locale_t)0) {
		std::printf("newlocale() failed\n");
		return 1;
	}

	run_edge_cases();
	run_random_sweep();

	std::printf("\nbatch b0069 differential results\n");
	std::printf("%-10s %12s %10s %s\n", "function", "cases", "failures",
	    "status");
	for (fn = 0; fn < F_COUNT; fn++) {
		std::printf("%-10s %12llu %10llu %s\n", fn_name[fn],
		    n_cases[fn], n_fails[fn],
		    n_fails[fn] == 0u ? "ok" : "FAIL");
		total_cases += n_cases[fn];
		total_fails += n_fails[fn];
	}
	std::printf("%-10s %12llu %10llu %s\n", "TOTAL", total_cases,
	    total_fails, total_fails == 0u ? "ok" : "FAIL");
	std::printf("\nno buffer-writing, pointer-returning or stateful"
	    " iterator functions in this batch;\nevery case still runs on two"
	    " independent 0x7f-guarded buffers whose full contents\nare"
	    " compared afterwards.\n");

	freelocale(loc_c);
	freelocale(loc_posix);

	return total_fails == 0u ? 0 : 1;
}
