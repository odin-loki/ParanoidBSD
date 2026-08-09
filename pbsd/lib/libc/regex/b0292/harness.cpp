/*
 * harness.cpp -- differential test for PBSD batch b0292 (regerror, regexec).
 */

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cstdarg>
#include <clocale>

import pbsd.lib.libc.regex.b0292;

namespace P = pbsd::lib_libc_regex::b0292;

extern "C" {
size_t ref_regerror(int errcode, const regex_t *preg, char *errbuf,
    size_t errbuf_size);
int ref_regexec(const regex_t *preg, const char *string, size_t nmatch,
    regmatch_t pmatch[], int eflags);
int regcomp(regex_t *preg, const char *pattern, int cflags);
void regfree(regex_t *preg);
}

/* ------------------------------------------------------------------ */
/* bookkeeping                                                        */
/* ------------------------------------------------------------------ */

struct Stat {
	const char *name;
	unsigned long cases;
	unsigned long fails;
};

static Stat st_regerror = { "regerror", 0, 0 };
static Stat st_regexec = { "regexec", 0, 0 };

static unsigned long reported;

static void
fail(Stat &s, const char *fmt, ...) __attribute__((format(printf, 2, 3)));

static void
fail(Stat &s, const char *fmt, ...)
{
	va_list ap;

	s.fails++;
	if (reported < 25) {
		reported++;
		std::fprintf(stderr, "FAIL [%s] ", s.name);
		va_start(ap, fmt);
		std::vfprintf(stderr, fmt, ap);
		va_end(ap);
		std::fputc('\n', stderr);
	}
}

/* ------------------------------------------------------------------ */
/* PRNG (fixed seed)                                                  */
/* ------------------------------------------------------------------ */

static std::uint64_t rng_state;

static void
rng_seed(std::uint64_t s)
{
	rng_state = s ? s : 0x9e3779b97f4a7c15ull;
}

static std::uint64_t
rng_next(void)
{
	std::uint64_t x = rng_state;

	x ^= x << 13;
	x ^= x >> 7;
	x ^= x << 17;
	rng_state = x;
	return (x * 0x2545f4914f6cdd1dull);
}

static std::uint32_t
rng_below(std::uint32_t bound)
{
	return (bound == 0 ? 0 : (std::uint32_t)(rng_next() % bound));
}

/* ------------------------------------------------------------------ */
/* buffer helpers                                                     */
/* ------------------------------------------------------------------ */

static constexpr unsigned char GUARD_BYTE = 0x7f;
static constexpr size_t GUARD = 16;

static void
fill_guard(void *p, size_t n)
{
	std::memset(p, GUARD_BYTE, n);
}

static int
buf_eq(const void *a, const void *b, size_t n)
{
	return (std::memcmp(a, b, n) == 0);
}

/* ------------------------------------------------------------------ */
/* regerror                                                           */
/* ------------------------------------------------------------------ */

static const char *const err_names[] = {
	"REG_NOMATCH", "REG_BADPAT", "REG_ECOLLATE", "REG_ECTYPE",
	"REG_EESCAPE", "REG_ESUBREG", "REG_EBRACK", "REG_EPAREN",
	"REG_EBRACE", "REG_BADBR", "REG_ERANGE", "REG_ESPACE",
	"REG_BADRPT", "REG_EMPTY", "REG_ASSERT", "REG_INVARG",
	"REG_ILLSEQ",
};

static const int err_codes[] = {
	REG_NOMATCH, REG_BADPAT, REG_ECOLLATE, REG_ECTYPE,
	REG_EESCAPE, REG_ESUBREG, REG_EBRACK, REG_EPAREN,
	REG_EBRACE, REG_BADBR, REG_ERANGE, REG_ESPACE,
	REG_BADRPT, REG_EMPTY, REG_ASSERT, REG_INVARG,
	REG_ILLSEQ,
};

static void
check_regerror(int errcode, const regex_t *preg, size_t bufsz)
{
	static constexpr size_t cap = 256;
	char pbuf[GUARD + cap + GUARD];
	char rbuf[GUARD + cap + GUARD];
	size_t plen, rlen;
	size_t use = bufsz > cap ? cap : bufsz;

	st_regerror.cases++;
	fill_guard(pbuf, sizeof(pbuf));
	fill_guard(rbuf, sizeof(rbuf));

	plen = P::regerror(errcode, preg, pbuf + GUARD, use);
	rlen = ref_regerror(errcode, preg, rbuf + GUARD, use);

	if (plen != rlen)
		fail(st_regerror, "errcode=%d bufsz=%zu ret port=%zu ref=%zu",
		    errcode, bufsz, plen, rlen);
	if (!buf_eq(pbuf, rbuf, sizeof(pbuf)))
		fail(st_regerror, "errcode=%d bufsz=%zu buffer mismatch",
		    errcode, bufsz);
}

static void
test_regerror_hand(void)
{
	regex_t preg;
	unsigned i;

	for (i = 0; i < sizeof(err_codes) / sizeof(err_codes[0]); i++) {
		check_regerror(err_codes[i], nullptr, 0);
		check_regerror(err_codes[i], nullptr, 1);
		check_regerror(err_codes[i], nullptr, 2);
		check_regerror(err_codes[i], nullptr, 64);
		check_regerror(err_codes[i], nullptr, 256);

		check_regerror(err_codes[i] | REG_ITOA, nullptr, 0);
		check_regerror(err_codes[i] | REG_ITOA, nullptr, 1);
		check_regerror(err_codes[i] | REG_ITOA, nullptr, 32);
		check_regerror(err_codes[i] | REG_ITOA, nullptr, 256);
	}

	check_regerror(99, nullptr, 0);
	check_regerror(99, nullptr, 1);
	check_regerror(99, nullptr, 64);
	check_regerror(99 | REG_ITOA, nullptr, 64);
	check_regerror(0x42 | REG_ITOA, nullptr, 64);

	std::memset(&preg, 0, sizeof(preg));
	for (i = 0; i < sizeof(err_names) / sizeof(err_names[0]); i++) {
		preg.re_endp = err_names[i];
		check_regerror(REG_ATOI, &preg, 0);
		check_regerror(REG_ATOI, &preg, 1);
		check_regerror(REG_ATOI, &preg, 8);
		check_regerror(REG_ATOI, &preg, 64);
	}
	preg.re_endp = "REG_NOPE";
	check_regerror(REG_ATOI, &preg, 64);
	preg.re_endp = "";
	check_regerror(REG_ATOI, &preg, 64);

	/* boundary around nominal strlen */
	check_regerror(REG_BADPAT, nullptr, 27);
	check_regerror(REG_BADPAT, nullptr, 28);
	check_regerror(REG_BADPAT, nullptr, 29);
}

static void
test_regerror_random(void)
{
	regex_t preg;
	unsigned long n;

	for (n = 0; n < 20000; n++) {
		int code;
		size_t bufsz;
		unsigned pick;

		code = (int)(rng_next() & 0x1ff);
		if ((rng_next() & 3) == 0)
			code |= REG_ITOA;
		bufsz = (size_t)(rng_next() % 300);
		check_regerror(code, nullptr, bufsz);

		pick = rng_below((std::uint32_t)(sizeof(err_names) /
		    sizeof(err_names[0])));
		preg.re_endp = err_names[pick];
		check_regerror(REG_ATOI, &preg, bufsz);

		if ((rng_next() & 1) != 0) {
			preg.re_endp = "REG_UNKNOWN_X";
			check_regerror(REG_ATOI, &preg, bufsz);
		}
	}
}

/* ------------------------------------------------------------------ */
/* regexec helpers                                                    */
/* ------------------------------------------------------------------ */

static constexpr size_t PMATCH_MAX = 16;
static constexpr size_t STR_CAP = 512;

struct ExecBuf {
	regmatch_t g_lo;
	regmatch_t m[PMATCH_MAX];
	regmatch_t g_hi;
};

static void
init_exec_buf(ExecBuf &eb)
{
	unsigned i;

	eb.g_lo.rm_so = eb.g_lo.rm_eo = 0x7f7f7f7f;
	eb.g_hi.rm_so = eb.g_hi.rm_eo = 0x7f7f7f7f;
	for (i = 0; i < PMATCH_MAX; i++) {
		eb.m[i].rm_so = 0x7f7f7f7f;
		eb.m[i].rm_eo = 0x7f7f7f7f;
	}
}

static int
exec_buf_eq(const ExecBuf &a, const ExecBuf &b)
{
	return (std::memcmp(&a, &b, sizeof(a)) == 0);
}

static void
check_regexec(const regex_t *preg, const char *str, size_t nmatch,
    int eflags, const regmatch_t *startend)
{
	char sbuf[GUARD + STR_CAP + GUARD];
	char pbuf[GUARD + STR_CAP + GUARD];
	ExecBuf peb, reb;
	size_t slen;
	int prc, rrc;

	st_regexec.cases++;

	slen = std::strlen(str);
	if (slen > STR_CAP - 1)
		slen = STR_CAP - 1;

	fill_guard(sbuf, sizeof(sbuf));
	fill_guard(pbuf, sizeof(pbuf));
	std::memcpy(sbuf + GUARD, str, slen);
	sbuf[GUARD + slen] = '\0';
	std::memcpy(pbuf + GUARD, str, slen);
	pbuf[GUARD + slen] = '\0';

	init_exec_buf(peb);
	init_exec_buf(reb);

	if (startend != nullptr) {
		peb.m[0] = reb.m[0] = *startend;
	}

	prc = P::regexec(preg, pbuf + GUARD, nmatch, peb.m, eflags);
	rrc = ref_regexec(preg, sbuf + GUARD, nmatch, reb.m, eflags);

	if (prc != rrc)
		fail(st_regexec,
		    "str=\"%.*s\" nmatch=%zu eflags=0x%x ret port=%d ref=%d",
		    (int)slen, str, nmatch, eflags, prc, rrc);
	if (!exec_buf_eq(peb, reb))
		fail(st_regexec,
		    "str=\"%.*s\" nmatch=%zu eflags=0x%x pmatch mismatch",
		    (int)slen, str, nmatch, eflags);
	if (!buf_eq(sbuf, pbuf, sizeof(sbuf)))
		fail(st_regexec,
		    "str=\"%.*s\" nmatch=%zu eflags=0x%x string buf mismatch",
		    (int)slen, str, nmatch, eflags);
}

static void
run_exec_case(const char *pat, int cflags, const char *str, size_t nmatch,
    int eflags, const regmatch_t *se)
{
	regex_t re;
	int err;

	std::memset(&re, 0, sizeof(re));
	err = regcomp(&re, pat, cflags);
	if (err != 0)
		return;
	check_regexec(&re, str, nmatch, eflags, se);
	regfree(&re);
}

static void
run_exec_case_expect(const char *pat, int cflags, const char *str,
    size_t nmatch, int eflags, int expect)
{
	regex_t re;
	int err, prc, rrc;
	ExecBuf peb, reb;

	std::memset(&re, 0, sizeof(re));
	err = regcomp(&re, pat, cflags);
	if (err != 0) {
		if (expect == err)
			return;
		fail(st_regexec, "regcomp pat=\"%s\" unexpected err=%d",
		    pat, err);
		return;
	}

	st_regexec.cases++;
	init_exec_buf(peb);
	init_exec_buf(reb);
	prc = P::regexec(&re, str, nmatch, peb.m, eflags);
	rrc = ref_regexec(&re, str, nmatch, reb.m, eflags);
	if (prc != rrc || prc != expect)
		fail(st_regexec,
		    "pat=\"%s\" str=\"%s\" expect=%d port=%d ref=%d",
		    pat, str, expect, prc, rrc);
	if (!exec_buf_eq(peb, reb))
		fail(st_regexec, "pat=\"%s\" str=\"%s\" pmatch mismatch",
		    pat, str);
	regfree(&re);
}

static void
test_regexec_badpat(void)
{
	regex_t re;
	regmatch_t m[1];

	std::memset(&re, 0, sizeof(re));
	re.re_magic = 0;
	check_regexec(&re, "x", 1, 0, nullptr);

	std::memset(&re, 0, sizeof(re));
	if (regcomp(&re, "a", REG_EXTENDED) == 0) {
		re.re_magic = 0;
		check_regexec(&re, "a", 1, 0, nullptr);
		regfree(&re);
	}
	(void)m;
}

static void
test_regexec_hand(void)
{
	regmatch_t se;
	static const char hi[] = "\x80\x81\xff\xfe\xfd";
	static const char mix[] = "a\x80b\xffc";

	/* empty and single char */
	run_exec_case("", REG_EXTENDED | REG_NOSPEC, "", 1, 0, nullptr);
	run_exec_case("a", REG_EXTENDED | REG_NOSPEC, "a", 1, 0, nullptr);
	run_exec_case("a", REG_EXTENDED | REG_NOSPEC, "b", 1, 0, nullptr);
	run_exec_case("a", REG_EXTENDED | REG_NOSPEC, "", 1, 0, nullptr);

	/* literals and anchors */
	run_exec_case("hello", REG_EXTENDED | REG_NOSPEC, "say hello", 1, 0,
	    nullptr);
	run_exec_case("^hello$", REG_EXTENDED, "hello", 1, 0, nullptr);
	run_exec_case("^hello$", REG_EXTENDED, "hello\n", 1, 0, nullptr);
	run_exec_case("^", REG_EXTENDED, "abc", 1, 0, nullptr);
	run_exec_case("$", REG_EXTENDED, "abc", 1, 0, nullptr);

	/* newline mode */
	run_exec_case("^a$", REG_EXTENDED | REG_NEWLINE, "a\nb", 1, 0,
	    nullptr);
	run_exec_case("^b$", REG_EXTENDED | REG_NEWLINE, "a\nb", 1, 0,
	    nullptr);
	run_exec_case("a.b", REG_EXTENDED | REG_NEWLINE, "a\nb", 1, 0,
	    nullptr);

	/* icase */
	run_exec_case("[Aa]", REG_EXTENDED, "a", 1, 0, nullptr);
	run_exec_case("[Aa]", REG_EXTENDED, "A", 1, 0, nullptr);
	run_exec_case("(?i)a", REG_EXTENDED, "A", 1, 0, nullptr);

	/* quantifiers */
	run_exec_case("a*", REG_EXTENDED, "", 1, 0, nullptr);
	run_exec_case("a*", REG_EXTENDED, "aaa", 1, 0, nullptr);
	run_exec_case("a+", REG_EXTENDED, "aaa", 1, 0, nullptr);
	run_exec_case("a?", REG_EXTENDED, "b", 1, 0, nullptr);
	run_exec_case("a{2,4}", REG_EXTENDED, "aaaa", 1, 0, nullptr);
	run_exec_case("a{2,4}", REG_EXTENDED, "a", 1, 0, nullptr);

	/* alternation and grouping */
	run_exec_case("(a|b|c)", REG_EXTENDED, "b", 2, 0, nullptr);
	run_exec_case("(foo)(bar)", REG_EXTENDED, "foobar", 3, 0, nullptr);
	run_exec_case("((a)(b))", REG_EXTENDED, "ab", 4, 0, nullptr);

	/* backreferences */
	run_exec_case("(a)\\1", REG_EXTENDED, "aa", 2, 0, nullptr);
	run_exec_case("(a)\\1", REG_EXTENDED, "ab", 2, 0, nullptr);
	run_exec_case("(a*)\\1", REG_EXTENDED, "aaaa", 2, REG_BACKR, nullptr);
	run_exec_case("(.)(.)\\1\\2", REG_EXTENDED, "abab", 3, REG_BACKR,
	    nullptr);

	/* bracket expressions */
	run_exec_case("[abc]", REG_EXTENDED, "b", 1, 0, nullptr);
	run_exec_case("[^abc]", REG_EXTENDED, "d", 1, 0, nullptr);
	run_exec_case("[a-z]", REG_EXTENDED, "m", 1, 0, nullptr);
	run_exec_case("[0-9]+", REG_EXTENDED, "123", 1, 0, nullptr);

	/* word boundaries */
	run_exec_case("\\<word\\>", REG_EXTENDED, "a word here", 1, 0, nullptr);
	run_exec_case("\\<word\\>", REG_EXTENDED, "wordy", 1, 0, nullptr);
	run_exec_case("\\B", REG_EXTENDED, "ab", 1, 0, nullptr);

	/* BRE */
	run_exec_case("a\\|b", REG_BASIC, "a", 1, 0, nullptr);
	run_exec_case("a\\|b", REG_BASIC, "b", 1, 0, nullptr);
	run_exec_case("\\(a\\)\\1", REG_BASIC, "aa", 2, REG_BACKR, nullptr);
	run_exec_case("^[0-9][0-9]*$", REG_BASIC, "42", 1, 0, nullptr);

	/* REG_NOSUB */
	run_exec_case("(a)(b)", REG_EXTENDED | REG_NOSUB, "ab", 3, 0, nullptr);

	/* REG_NOTBOL / REG_NOTEOL */
	run_exec_case("^a", REG_EXTENDED, "ba", 1, REG_NOTBOL, nullptr);
	run_exec_case("a$", REG_EXTENDED, "ab", 1, REG_NOTEOL, nullptr);

	/* REG_STARTEND */
	se.rm_so = 2;
	se.rm_eo = 5;
	run_exec_case("b", REG_EXTENDED | REG_NOSPEC, "xxbyyzz", 1,
	    REG_STARTEND, &se);

	/* REG_LARGE forces large-state matcher in C locale */
	run_exec_case("a", REG_EXTENDED | REG_NOSPEC, "a", 1, REG_LARGE,
	    nullptr);

	/* long pattern for Boyer-Moore prescreen */
	run_exec_case("abcdefghijklmnop", REG_EXTENDED | REG_NOSPEC,
	    "zzzabcdefghijklmnopzzz", 1, 0, nullptr);
	run_exec_case("abcdefghijklmnop", REG_EXTENDED | REG_NOSPEC,
	    "zzzabcdefghijklmnoqzzz", 1, 0, nullptr);

	/* high-bit bytes */
	run_exec_case("\x80", REG_EXTENDED | REG_NOSPEC, hi, 1, 0, nullptr);
	run_exec_case("\xff", REG_EXTENDED | REG_NOSPEC, hi, 1, 0, nullptr);
	run_exec_case(".\x80.", REG_EXTENDED | REG_NOSPEC, mix, 1, 0, nullptr);

	/* dot and escapes */
	run_exec_case(".", REG_EXTENDED, "\n", 1, 0, nullptr);
	run_exec_case(".", REG_EXTENDED | REG_NEWLINE, "\n", 1, 0, nullptr);
	run_exec_case("\\.", REG_EXTENDED | REG_NOSPEC, ".", 1, 0, nullptr);
	run_exec_case("\\(", REG_EXTENDED | REG_NOSPEC, "(", 1, 0, nullptr);

	/* nested quantifiers / plus */
	run_exec_case("(a+)+", REG_EXTENDED, "aaa", 2, REG_BACKR, nullptr);
	run_exec_case("(x|y)*z", REG_EXTENDED, "xyyxz", 1, 0, nullptr);

	/* REG_INVARG via REG_STARTEND */
	se.rm_so = 5;
	se.rm_eo = 2;
	run_exec_case_expect("a", REG_EXTENDED | REG_NOSPEC, "abc", 1,
	    REG_STARTEND, REG_INVARG);

	/* complex pattern with many states (large matcher in C locale) */
	run_exec_case(
	    "((((((((((a))))))))))", REG_EXTENDED, "a", 1, REG_LARGE, nullptr);

	/* character classes */
	run_exec_case("[[:digit:]]+", REG_EXTENDED, "123abc", 1, 0, nullptr);
	run_exec_case("[[:alpha:]]", REG_EXTENDED, "9", 1, 0, nullptr);

	/* empty groups and tricky cases */
	run_exec_case("()", REG_EXTENDED, "", 2, 0, nullptr);
	run_exec_case("a|", REG_EXTENDED, "", 1, 0, nullptr);
	run_exec_case("|a", REG_EXTENDED, "a", 1, 0, nullptr);

	/* run a subset under C locale for smatcher path */
	if (setlocale(LC_ALL, "C") != nullptr) {
		run_exec_case("abc", REG_EXTENDED | REG_NOSPEC, "xabcy", 1, 0,
		    nullptr);
		run_exec_case("(a)(b)(c)", REG_EXTENDED, "abc", 4, 0, nullptr);
		{
			char many_a[201];

			std::memset(many_a, 'a', 200);
			many_a[200] = '\0';
			run_exec_case("a{1,255}", REG_EXTENDED, many_a, 1, 0,
			    nullptr);
		}
		run_exec_case(
		    "((((((((((a))))))))))", REG_EXTENDED, "a", 1, REG_LARGE,
		    nullptr);
	}
	setlocale(LC_ALL, "");
}

static void
test_regexec_random(void)
{
	static const char *const atoms[] = {
		"a", "b", "c", ".", "^", "$", "*", "+", "?", "|", "(", ")",
		"[", "]", "\\", "1", "0", "-", "\n", "\x80", "\xff",
	};
	static const int cflag_opts[] = {
		REG_EXTENDED, REG_EXTENDED | REG_ICASE,
		REG_EXTENDED | REG_NEWLINE,
		REG_EXTENDED | REG_NOSUB,
		REG_BASIC,
	};
	char pat[64];
	char str[128];
	unsigned long n;
	unsigned i, j;

	for (n = 0; n < 180000; n++) {
		regex_t re;
		int cflags, eflags;
		size_t nmatch;
		unsigned plen, slen;
		int err;

		if ((n % 5000) == 0 && (n % 10000) == 0)
			setlocale(LC_ALL, "C");
		else if ((n % 5000) == 0)
			setlocale(LC_ALL, "");

		plen = 1 + rng_below(40);
		for (i = 0; i < plen; i++) {
			const char *a = atoms[rng_below(
			    (std::uint32_t)(sizeof(atoms) / sizeof(atoms[0])))];
			pat[i] = *a;
		}
		pat[plen] = '\0';

		slen = rng_below(80);
		for (j = 0; j < slen; j++) {
			unsigned v = rng_below(256);

			str[j] = (char)(unsigned char)v;
		}
		str[slen] = '\0';

		cflags = cflag_opts[rng_below(
		    (std::uint32_t)(sizeof(cflag_opts) /
		    sizeof(cflag_opts[0])))];
		eflags = 0;
		if (rng_below(8) == 0)
			eflags |= REG_NOTBOL;
		if (rng_below(8) == 0)
			eflags |= REG_NOTEOL;
		if (rng_below(16) == 0)
			eflags |= REG_LARGE;
		if (rng_below(16) == 0)
			eflags |= REG_BACKR;
		nmatch = 1 + rng_below(PMATCH_MAX);

		std::memset(&re, 0, sizeof(re));
		err = regcomp(&re, pat, cflags);
		if (err != 0)
			continue;
		check_regexec(&re, str, nmatch, eflags, nullptr);
		regfree(&re);
	}
}

/* ------------------------------------------------------------------ */
/* main                                                               */
/* ------------------------------------------------------------------ */

int
main(void)
{
	unsigned long total_cases, total_fails;

	rng_seed(0xb0292u);
	test_regerror_hand();
	test_regerror_random();
	test_regexec_badpat();
	test_regexec_hand();
	test_regexec_random();

	total_cases = st_regerror.cases + st_regexec.cases;
	total_fails = st_regerror.fails + st_regexec.fails;

	std::printf("function     cases     failures\n");
	std::printf("--------     -----     --------\n");
	std::printf("%-12s %9lu %10lu\n", st_regerror.name, st_regerror.cases,
	    st_regerror.fails);
	std::printf("%-12s %9lu %10lu\n", st_regexec.name, st_regexec.cases,
	    st_regexec.fails);
	std::printf("%-12s %9lu %10lu\n", "TOTAL", total_cases, total_fails);

	return (total_fails == 0 ? 0 : 1);
}
