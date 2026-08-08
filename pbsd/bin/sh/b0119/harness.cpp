/*
 * harness.cpp -- differential test for PBSD batch b0119.
 */

#include <cerrno>
#include <cinttypes>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

import pbsd.bin.sh.b0119;

namespace port = pbsd::bin_sh::b0119;

extern "C" int ref_is_number(const char *p);
extern "C" std::intmax_t ref_strtoarith_t(const char *nptr, char **endptr);
extern "C" int ref_yylex(void);

extern "C" const char *arith_buf;
extern "C" union yystype {
	std::intmax_t val;
	char *name;
} yylval;

static const int MAX_REPORT = 8;

struct Stat {
	const char *name;
	long cases;
	long fails;
	int reported;
};

static Stat st_is_number = { "is_number", 0, 0, 0 };
static Stat st_strtoarith_t = { "strtoarith_t", 0, 0, 0 };
static Stat st_yylex = { "yylex", 0, 0, 0 };

static std::uint64_t rng_state;

static inline std::uint64_t
rnd(void)
{
	std::uint64_t z = (rng_state += 0x9e3779b97f4a7c15ULL);

	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return z ^ (z >> 31);
}

static inline std::size_t
rnd_mod(std::size_t m)
{
	return (std::size_t)(rnd() % (std::uint64_t)m);
}

static void
in_run(const char *s, const char *tag)
{
	int pa = port::is_number(s);
	int pb = ref_is_number(s);

	st_is_number.cases++;
	if (pa != pb) {
		st_is_number.fails++;
		if (st_is_number.reported < MAX_REPORT) {
			st_is_number.reported++;
			std::printf("  FAIL is_number [%s] ret port=%d ref=%d\n", tag,
			    pa, pb);
		}
	}
}

static const char *const in_strs[] = {
	"", "0", "00", "000", "0000000000", "1", "9", "123", "1234567890",
	"2147483646", "2147483647", "2147483648", "2147483649", "9999999999",
	"10000000000", "0002147483647", "00000000001", "a", "1a", "a1",
	"12a34", "12\20034", "\200", "\xff", "\x7f", "0\200", " 1", "+1",
	"-1", "0x10", "0123", "21474836470", "0999999999", "Z", "_",
	"abc_def", "214748364", "2147483640", "2147483641",
};

static void
in_edge(void)
{
	std::size_t n = sizeof(in_strs) / sizeof(in_strs[0]);

	for (std::size_t i = 0; i < n; i++)
		in_run(in_strs[i], "edge");
}

static void
in_random(long count)
{
	char buf[32];

	for (long i = 0; i < count; i++) {
		std::size_t len = rnd_mod(17);
		for (std::size_t j = 0; j < len; j++) {
			unsigned mode = rnd_mod(16);
			if (mode == 0)
				buf[j] = '0' + (char)rnd_mod(10);
			else if (mode == 1)
				buf[j] = (char)(0x80 + rnd_mod(128));
			else if (mode == 2)
				buf[j] = '\0';
			else
				buf[j] = (char)(rnd_mod(256));
		}
		buf[len] = '\0';
		in_run(buf, "rand");
	}
}

static void
sa_run(const char *s, const char *tag)
{
	char *end_a = nullptr;
	char *end_b = nullptr;

	int errno_before = errno;
	std::intmax_t va = port::strtoarith_t(s, &end_a);
	int errno_a = errno;
	errno = errno_before;
	std::intmax_t vb = ref_strtoarith_t(s, &end_b);
	int errno_b = errno;

	std::ptrdiff_t off_a = end_a != nullptr ? end_a - s : 0;
	std::ptrdiff_t off_b = end_b != nullptr ? end_b - s : 0;

	int bad = 0;
	if (va != vb)
		bad = 1;
	if (off_a != off_b)
		bad = 2;
	if (errno_a != errno_b)
		bad |= 4;

	st_strtoarith_t.cases++;
	if (bad) {
		st_strtoarith_t.fails++;
		if (st_strtoarith_t.reported < MAX_REPORT) {
			st_strtoarith_t.reported++;
			std::printf("  FAIL strtoarith_t [%s] val port=%jd ref=%jd "
			    "off port=%td ref=%td errno port=%d ref=%d bad=%d\n",
			    tag, (intmax_t)va, (intmax_t)vb, off_a, off_b,
			    errno_a, errno_b, bad);
		}
	}
}

static const char *const sa_strs[] = {
	"0", "1", "42", "-1", "-42", "+1", "  123", "\t\n789", "00", "0123",
	"0x10", "0Xff", "077", "-077", "-0", "9223372036854775807",
	"9223372036854775808", "18446744073709551615", "18446744073709551616",
	"9999999999999999999", "-9223372036854775808", "-9223372036854775809",
	"2147483647", "2147483648", "abc", "12abc", "12\2003", "\2001", "",
	"   ", "-", "+", "--1", "++1", "0xffffffffffffffff", "0x", "0x0",
	"-0x10",
};

static void
sa_edge(void)
{
	std::size_t n = sizeof(sa_strs) / sizeof(sa_strs[0]);

	for (std::size_t i = 0; i < n; i++)
		sa_run(sa_strs[i], "edge");
}

static void
sa_random(long count)
{
	char buf[48];

	for (long i = 0; i < count; i++) {
		std::size_t len = rnd_mod(24);
		for (std::size_t j = 0; j < len; j++) {
			unsigned mode = rnd_mod(12);
			if (mode == 0)
				buf[j] = '0' + (char)rnd_mod(10);
			else if (mode == 1)
				buf[j] = (char)(0x80 + rnd_mod(128));
			else if (mode == 2 && j == 0)
				buf[j] = '-';
			else if (mode == 3 && j == 0)
				buf[j] = '+';
			else if (mode == 4 && j <= 1) {
				buf[j] = '0';
				if (j == 1 && len > 2)
					buf[j] = (char)('x' + rnd_mod(2));
			} else
				buf[j] = (char)(rnd_mod(256));
		}
		buf[len] = '\0';
		sa_run(buf, "rand");
	}
}

static const int ARITH_BAD = 4;
static const int ARITH_NUM = 5;
static const int ARITH_VAR = 6;

static void
yy_run_buf(const char *input, const char *tag)
{
	char in_copy[256];
	std::size_t in_len = std::strlen(input);
	if (in_len >= sizeof(in_copy))
		in_len = sizeof(in_copy) - 1;
	std::memcpy(in_copy, input, in_len);
	in_copy[in_len] = '\0';

	port::yylval.name = nullptr;
	yylval.name = nullptr;

	const char *cursor = in_copy;
	std::size_t step = 0;
	for (;;) {
		port::arith_buf = cursor;
		arith_buf = cursor;

		int ta = port::yylex();
		std::ptrdiff_t off_a = port::arith_buf - in_copy;
		std::intmax_t val_a = port::yylval.val;
		char *name_a = port::yylval.name;

		arith_buf = cursor;
		int tb = ref_yylex();
		std::ptrdiff_t off_b = arith_buf - in_copy;
		std::intmax_t val_b = yylval.val;
		char *name_b = yylval.name;

		int bad = 0;
		if (ta != tb)
			bad = 1;
		if (off_a != off_b)
			bad |= 2;
		if (ta == ARITH_NUM && val_a != val_b)
			bad |= 4;
		if (tb == ARITH_NUM && val_a != val_b)
			bad |= 4;
		if (ta == ARITH_VAR || tb == ARITH_VAR) {
			if (ta != tb)
				bad |= 8;
			else if (name_a == nullptr || name_b == nullptr) {
				if (name_a != name_b)
					bad |= 8;
			} else if (std::strcmp(name_a, name_b) != 0)
				bad |= 8;
		}

		st_yylex.cases++;
		if (bad) {
			st_yylex.fails++;
			if (st_yylex.reported < MAX_REPORT) {
				st_yylex.reported++;
				std::printf("  FAIL yylex [%s] step=%zu tok port=%d "
				    "ref=%d off port=%td ref=%td bad=%d input=%s\n",
				    tag, step, ta, tb, off_a, off_b, bad, input);
			}
			if (ta == ARITH_VAR)
				std::free(port::yylval.name);
			if (tb == ARITH_VAR)
				std::free(yylval.name);
			break;
		}

		if (ta == ARITH_BAD && tb == ARITH_BAD && !bad)
			break;

		if (ta == ARITH_VAR) {
			std::free(port::yylval.name);
			port::yylval.name = nullptr;
		}
		if (tb == ARITH_VAR) {
			std::free(yylval.name);
			yylval.name = nullptr;
		}

		if (ta == 0)
			break;
		cursor = port::arith_buf;
		step++;
		if (step > 512) {
			st_yylex.fails++;
			if (st_yylex.reported < MAX_REPORT) {
				st_yylex.reported++;
				std::printf("  FAIL yylex [%s] step limit\n", tag);
			}
			break;
		}
	}
}

static const char *const yy_strs[] = {
	"", " ", "\t\n", "42", "0", "007", "1234567890", "a", "A", "_", "z9_",
	"var_name", "=", "==", ">", ">=", ">>", ">>>", ">>=", "<", "<=",
	"<<", "<<=", "|", "||", "|=", "&", "&&", "&=", "!", "!=", "(", ")",
	"*", "*=", "/", "/=", "%", "%=", "+", "+=", "++", "-", "-=", "--",
	"~", "^", "^=", "?", ":", "@", "#", "\200", "\xff", "a+1", "1+a",
	"x==y", "a>=b", "c<<d", "e>>f", "g|=h", "i&=j", "k*=l", "m/=n",
	"o%=p", "q+=r", "s-=t", "u^=v", "!x", "(1)", "  99  ", "\n\t42\n",
	"a++b", "a--b", "++", "--", "===", ">>>=", "====", "0xffffffff",
	"-42", "0x10", "var1 var2", "___", "Z9", "a\200b", "\200a", "a|b",
	"a||b", "a&b", "a&&b", "a=b", "a==b", "a<b", "a<=b", "a>b", "a>=b",
	"a<<b", "a<<=b", "a>>b", "a>>=b", "a*b", "a*=b", "a/b", "a/=b",
	"a%b", "a%=b", "a+b", "a+=b", "a-b", "a-=b", "a^b", "a^=b",
	"a?b:c", "~a", "!a", "!=a", "(a)", ")a", "a)",
};

static void
yy_edge(void)
{
	std::size_t n = sizeof(yy_strs) / sizeof(yy_strs[0]);

	for (std::size_t i = 0; i < n; i++)
		yy_run_buf(yy_strs[i], "edge");
}

static void
yy_random(long count)
{
	static const char *atoms[] = {
		"0", "1", "42", "999", "a", "var", "x1", "_y",
		"=", "==", ">", ">=", ">>", ">>=", "<", "<=", "<<", "<<=",
		"|", "||", "|=", "&", "&&", "&=", "!", "!=",
		"(", ")", "*", "*=", "/", "/=", "%", "%=",
		"+", "+=", "++", "-", "-=", "--",
		"~", "^", "^=", "?", ":",
		" ", "\t", "\n", "@", "#", "\200", "\xff",
	};
	const std::size_t natoms = sizeof(atoms) / sizeof(atoms[0]);
	char buf[128];

	for (long i = 0; i < count; i++) {
		std::size_t ntok = 1 + rnd_mod(8);
		std::size_t pos = 0;

		for (std::size_t t = 0; t < ntok && pos + 1 < sizeof(buf); t++) {
			const char *atom = atoms[rnd_mod(natoms)];
			std::size_t alen = std::strlen(atom);
			if (pos + alen >= sizeof(buf))
				break;
			std::memcpy(buf + pos, atom, alen);
			pos += alen;
		}
		buf[pos] = '\0';

		if (rnd_mod(16) == 0) {
			std::size_t junk = rnd_mod(8);
			for (std::size_t j = 0; j < junk && pos + 1 < sizeof(buf); j++)
				buf[pos++] = (char)(0x80 + rnd_mod(128));
			buf[pos] = '\0';
		}

		yy_run_buf(buf, "rand");
	}
}

int
main(void)
{
	rng_state = 0xb0119b0119ULL;
	in_edge();
	in_random(70000);

	rng_state = 0xb0119aULL;
	sa_edge();
	sa_random(70000);

	rng_state = 0xb011901ULL;
	yy_edge();
	yy_random(70000);

	std::printf("\n%-14s %12s %12s   %s\n", "function", "cases",
	    "failures", "result");
	std::printf("%-14s %12s %12s   %s\n", "--------------", "------------",
	    "------------", "------");

	const Stat *all[] = { &st_is_number, &st_strtoarith_t, &st_yylex };
	long total_fail = 0;
	long total_case = 0;

	for (std::size_t i = 0; i < sizeof(all) / sizeof(all[0]); i++) {
		std::printf("%-14s %12ld %12ld   %s\n", all[i]->name,
		    all[i]->cases, all[i]->fails,
		    all[i]->fails == 0 ? "PASS" : "FAIL");
		total_fail += all[i]->fails;
		total_case += all[i]->cases;
	}
	std::printf("%-14s %12s %12s   %s\n", "--------------", "------------",
	    "------------", "------");
	std::printf("%-14s %12ld %12ld   %s\n", "TOTAL", total_case,
	    total_fail, total_fail == 0 ? "PASS" : "FAIL");

	return total_fail == 0 ? 0 : 1;
}
