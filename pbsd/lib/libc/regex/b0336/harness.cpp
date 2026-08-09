/*
 * harness.cpp -- differential test for PBSD batch b0336 (regcomp).
 */

#include <clocale>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <sys/types.h>

typedef off_t regoff_t;

struct re_guts;

typedef struct {
	int re_magic;
	size_t re_nsub;
	const char *re_endp;
	struct re_guts *re_g;
} regex_t;

#define REG_BASIC     0000
#define REG_EXTENDED  0001
#define REG_ICASE     0002
#define REG_NOSUB     0004
#define REG_NEWLINE   0010
#define REG_NOSPEC    0020
#define REG_PEND      0040
#define REG_DUMP      0200
#define REG_POSIX     0400

#define REG_NOMATCH   1
#define REG_BADPAT    2
#define REG_ECOLLATE  3
#define REG_ECTYPE    4
#define REG_EESCAPE   5
#define REG_ESUBREG   6
#define REG_EBRACK    7
#define REG_EPAREN    8
#define REG_EBRACE    9
#define REG_BADBR     10
#define REG_ERANGE    11
#define REG_ESPACE    12
#define REG_BADRPT    13
#define REG_EMPTY     14
#define REG_ASSERT    15
#define REG_INVARG    16
#define REG_ILLSEQ    17

#define MAGIC1        ((('r'^0200)<<8) | 'e')
#define MAGIC2        ((('R'^0200)<<8)|'E')
#define NC_MAX        (CHAR_MAX - CHAR_MIN + 1)

typedef unsigned long sop;
typedef unsigned long sopno;

typedef struct {
	wint_t min;
	wint_t max;
} crange;

typedef struct {
	unsigned char bmp[NC_MAX / 8];
	wctype_t *types;
	unsigned int ntypes;
	wint_t *wides;
	unsigned int nwides;
	crange *ranges;
	unsigned int nranges;
	int invert;
	int icase;
} cset;

struct re_guts {
	int magic;
	sop *strip;
	unsigned int ncsets;
	cset *sets;
	int cflags;
	sopno nstates;
	sopno firststate;
	sopno laststate;
	int iflags;
	int nbol;
	int neol;
	char *must;
	int moffset;
	int *charjump;
	int *matchjump;
	int mlen;
	size_t nsub;
	int backrefs;
	sopno nplus;
};

import pbsd.lib.libc.regex.b0336;

namespace P = pbsd::lib_libc_regex::b0336;

extern "C" {
int ref_regcomp(regex_t *preg, const char *pattern, int cflags);
void regfree(regex_t *preg);
}

/* ------------------------------------------------------------------ */

struct Stat {
	const char *name;
	unsigned long cases;
	unsigned long fails;
};

static Stat st_regcomp = { "regcomp", 0, 0 };
static unsigned long reported;

static void
fail(Stat &s, const char *fmt, ...) __attribute__((format(printf, 2, 3)));

static void
fail(Stat &s, const char *fmt, ...)
{
	va_list ap;

	s.fails++;
	if (reported < 30) {
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

static int
rng_bit(void)
{
	return (int)(rng_next() & 1u);
}

/* ------------------------------------------------------------------ */
/* regex_t guard buffers                                              */
/* ------------------------------------------------------------------ */

static constexpr unsigned char GUARD_BYTE = 0x7f;
static constexpr size_t RE_GUARD = 32;

struct GuardedRegex {
	unsigned char pre[RE_GUARD];
	regex_t re;
	unsigned char post[RE_GUARD];
};

static void
guard_init(GuardedRegex *gr)
{
	std::memset(gr->pre, GUARD_BYTE, RE_GUARD);
	std::memset(&gr->re, 0, sizeof(gr->re));
	std::memset(gr->post, GUARD_BYTE, RE_GUARD);
}

static int
guard_ok(const GuardedRegex *gr)
{
	for (size_t i = 0; i < RE_GUARD; i++) {
		if (gr->pre[i] != GUARD_BYTE || gr->post[i] != GUARD_BYTE)
			return (0);
	}
	return (1);
}

/* ------------------------------------------------------------------ */
/* re_guts deep comparison                                            */
/* ------------------------------------------------------------------ */

static int
cset_eq(const cset *a, const cset *b)
{
	if (a->ntypes != b->ntypes || a->nwides != b->nwides ||
	    a->nranges != b->nranges || a->invert != b->invert ||
	    a->icase != b->icase)
		return (0);
	if (std::memcmp(a->bmp, b->bmp, sizeof(a->bmp)) != 0)
		return (0);
	if (a->ntypes > 0 &&
	    std::memcmp(a->types, b->types, a->ntypes * sizeof(wctype_t)) != 0)
		return (0);
	if (a->nwides > 0 &&
	    std::memcmp(a->wides, b->wides, a->nwides * sizeof(wint_t)) != 0)
		return (0);
	if (a->nranges > 0 &&
	    std::memcmp(a->ranges, b->ranges, a->nranges * sizeof(crange)) != 0)
		return (0);
	return (1);
}

static int
guts_eq(const re_guts *pg, const re_guts *rg)
{
	int ch, mi;

	if (pg == NULL && rg == NULL)
		return (1);
	if (pg == NULL || rg == NULL)
		return (0);

	if (pg->magic != rg->magic || pg->ncsets != rg->ncsets ||
	    pg->cflags != rg->cflags || pg->nstates != rg->nstates ||
	    pg->firststate != rg->firststate || pg->laststate != rg->laststate ||
	    pg->iflags != rg->iflags || pg->nbol != rg->nbol ||
	    pg->neol != rg->neol || pg->moffset != rg->moffset ||
	    pg->mlen != rg->mlen || pg->nsub != rg->nsub ||
	    pg->backrefs != rg->backrefs || pg->nplus != rg->nplus)
		return (0);

	if (pg->nstates > 0 &&
	    std::memcmp(pg->strip, rg->strip, pg->nstates * sizeof(sop)) != 0)
		return (0);

	for (unsigned int i = 0; i < pg->ncsets; i++) {
		if (!cset_eq(&pg->sets[i], &rg->sets[i]))
			return (0);
	}

	if (pg->mlen > 0) {
		if (pg->must == NULL || rg->must == NULL)
			return (0);
		if (std::memcmp(pg->must, rg->must, (size_t)pg->mlen) != 0)
			return (0);
	} else if (pg->must != rg->must && !(pg->must == NULL && rg->must == NULL)) {
		if (pg->must != NULL || rg->must != NULL) {
			if (pg->must == NULL || rg->must == NULL)
				return (0);
			if (std::strcmp(pg->must, rg->must) != 0)
				return (0);
		}
	}

	if (pg->charjump != NULL || rg->charjump != NULL) {
		if (pg->charjump == NULL || rg->charjump == NULL)
			return (0);
		for (ch = CHAR_MIN; ch <= CHAR_MAX; ch++) {
			if (pg->charjump[ch] != rg->charjump[ch])
				return (0);
		}
	}

	if (pg->matchjump != NULL || rg->matchjump != NULL) {
		if (pg->matchjump == NULL || rg->matchjump == NULL)
			return (0);
		for (mi = 0; mi < pg->mlen; mi++) {
			if (pg->matchjump[mi] != rg->matchjump[mi])
				return (0);
		}
	}

	return (1);
}

static void
check_one(const char *pat, int cflags, const char *endp, const char *tag)
{
	GuardedRegex pg, rg;
	int pr, rr;
	const char *use_pat = pat;
	char pend_buf[256];
	size_t patlen;

	st_regcomp.cases++;
	guard_init(&pg);
	guard_init(&rg);

	if (endp != NULL) {
		patlen = (size_t)(endp - pat);
		if (patlen >= sizeof(pend_buf))
			patlen = sizeof(pend_buf) - 1;
		std::memcpy(pend_buf, pat, patlen);
		pend_buf[patlen] = '\0';
		use_pat = pend_buf;
		pg.re.re_endp = pend_buf + patlen;
		rg.re.re_endp = pend_buf + patlen;
	}

	pr = P::regcomp(&pg.re, use_pat, cflags);
	rr = ref_regcomp(&rg.re, use_pat, cflags);

	if (!guard_ok(&pg) || !guard_ok(&rg)) {
		fail(st_regcomp, "%s guard corrupted", tag);
		goto cleanup;
	}

	if (pr != rr) {
		fail(st_regcomp, "%s flags=0%o ret port=%d ref=%d pat=\"%s\"",
		    tag, cflags, pr, rr, use_pat);
		goto cleanup;
	}

	if (pr == 0) {
		if (pg.re.re_magic != rg.re.re_magic) {
			fail(st_regcomp, "%s re_magic port=%d ref=%d",
			    tag, pg.re.re_magic, rg.re.re_magic);
		}
		if (pg.re.re_nsub != rg.re.re_nsub) {
			fail(st_regcomp, "%s re_nsub port=%zu ref=%zu",
			    tag, pg.re.re_nsub, rg.re.re_nsub);
		}
		if (!guts_eq((const re_guts *)pg.re.re_g,
		    (const re_guts *)rg.re.re_g)) {
			fail(st_regcomp, "%s guts mismatch flags=0%o pat=\"%s\"",
			    tag, cflags, use_pat);
		}
	} else {
		if (pg.re.re_g != NULL || rg.re.re_g != NULL) {
			fail(st_regcomp, "%s error but re_g non-NULL port=%p ref=%p",
			    tag, (void *)pg.re.re_g, (void *)rg.re.re_g);
		}
	}

cleanup:
	if (pg.re.re_g != NULL)
		regfree(&pg.re);
	if (rg.re.re_g != NULL)
		regfree(&rg.re);
}

/* ------------------------------------------------------------------ */
/* hand-crafted patterns                                              */
/* ------------------------------------------------------------------ */

static void
handcrafted(void)
{
	static const struct {
		const char *pat;
		int cflags;
	} basic[] = {
		{ "", REG_EXTENDED },
		{ "a", REG_EXTENDED },
		{ ".", REG_EXTENDED },
		{ ".*", REG_EXTENDED },
		{ "^$", REG_EXTENDED },
		{ "^abc$", REG_EXTENDED },
		{ "a|b", REG_EXTENDED },
		{ "a|", REG_EXTENDED },
		{ "|a", REG_EXTENDED },
		{ "(a)", REG_EXTENDED },
		{ "((a))", REG_EXTENDED },
		{ "(a)(b)", REG_EXTENDED },
		{ "a+", REG_EXTENDED },
		{ "a?", REG_EXTENDED },
		{ "a*", REG_EXTENDED },
		{ "a{1}", REG_EXTENDED },
		{ "a{1,3}", REG_EXTENDED },
		{ "a{1,}", REG_EXTENDED },
		{ "a{0,0}", REG_EXTENDED },
		{ "[a]", REG_EXTENDED },
		{ "[^a]", REG_EXTENDED },
		{ "[a-z]", REG_EXTENDED },
		{ "[]]", REG_EXTENDED },
		{ "[-a]", REG_EXTENDED },
		{ "[a-]", REG_EXTENDED },
		{ "[[:alpha:]]", REG_EXTENDED },
		{ "[[:digit:]]", REG_EXTENDED },
		{ "[[:alnum:]_]", REG_EXTENDED },
		{ "[[:space:]]", REG_EXTENDED },
		{ "\\w", REG_EXTENDED },
		{ "\\W", REG_EXTENDED },
		{ "\\s", REG_EXTENDED },
		{ "\\S", REG_EXTENDED },
		{ "\\b", REG_EXTENDED },
		{ "\\B", REG_EXTENDED },
		{ "a\\1", REG_EXTENDED },
		{ "(a)\\1", REG_EXTENDED },
		{ "(a)(b)\\2", REG_EXTENDED },
		{ "\\<", REG_EXTENDED },
		{ "\\>", REG_EXTENDED },
		{ "a\n", REG_EXTENDED | REG_NEWLINE },
		{ ".", REG_EXTENDED | REG_NEWLINE },
		{ "A", REG_EXTENDED | REG_ICASE },
		{ "[a]", REG_EXTENDED | REG_ICASE },
		{ "a{255}", REG_EXTENDED },
		{ "a{256}", REG_EXTENDED },
		{ "a{0,255}", REG_EXTENDED },
		{ "a{256,255}", REG_EXTENDED },
		{ "*", REG_EXTENDED },
		{ "+", REG_EXTENDED },
		{ "?", REG_EXTENDED },
		{ "{", REG_EXTENDED },
		{ "a**", REG_EXTENDED },
		{ "^*", REG_EXTENDED },
		{ "(", REG_EXTENDED },
		{ ")", REG_EXTENDED },
		{ "[", REG_EXTENDED },
		{ "]", REG_EXTENDED },
		{ "\\", REG_EXTENDED },
		{ "a\\", REG_EXTENDED },
		{ "[z-a]", REG_EXTENDED },
		{ "[\\x80-\\xff]", REG_EXTENDED },
		{ "\xc3\xa9", REG_EXTENDED },
		{ "\xff", REG_EXTENDED },
		{ "\x80", REG_EXTENDED },
		{ "a\x00b", REG_EXTENDED },
		{ "hello world", REG_NOSPEC },
		{ "a.b*c", REG_NOSPEC },
		{ "a", REG_BASIC },
		{ "a*", REG_BASIC },
		{ "a\\{1,3\\}", REG_BASIC },
		{ "\\(a\\)", REG_BASIC },
		{ "^a$", REG_BASIC },
		{ ".", REG_BASIC },
		{ "[a]", REG_BASIC },
		{ "\\<", REG_BASIC },
		{ "\\>", REG_BASIC },
		{ "a\\+", REG_BASIC },
		{ "a\\?", REG_BASIC },
		{ "\\w", REG_BASIC },
		{ "\\1", REG_BASIC },
		{ "(a)", REG_BASIC },
		{ "a|b", REG_BASIC },
		{ "a|", REG_BASIC },
		{ "a\\{", REG_BASIC },
		{ "a\\}", REG_BASIC },
		{ "[[:<:]]", REG_BASIC },
		{ "[[:>:]]", REG_BASIC },
		{ "abc", REG_EXTENDED | REG_NOSUB },
		{ "a|b", REG_EXTENDED | REG_POSIX },
		{ "a|b", REG_EXTENDED },
		{ "a|b", REG_EXTENDED | REG_ICASE | REG_NEWLINE },
		{ "a|b", REG_EXTENDED | REG_NOSUB | REG_NEWLINE },
		{ "((((((((((a))))))))))", REG_EXTENDED },
		{ "a{2,4}{1,2}", REG_EXTENDED },
		{ "[a-zA-Z0-9_]+@[a-z]+", REG_EXTENDED },
		{ "foo|bar|baz|qux", REG_EXTENDED },
		{ "(foo)(bar)(baz)", REG_EXTENDED },
		{ "\\x41", REG_EXTENDED },
		{ "a\\nb", REG_EXTENDED },
		{ "[\\n]", REG_EXTENDED },
		{ "[^\\n]", REG_EXTENDED | REG_NEWLINE },
		{ "a{1,1}", REG_EXTENDED },
		{ "a{2,2}", REG_EXTENDED },
		{ "a{0,1}", REG_EXTENDED },
		{ "a{0,2}", REG_EXTENDED },
		{ "a{3,5}", REG_EXTENDED },
		{ "aaaaaaaaaa", REG_EXTENDED },
		{ "a{10,20}", REG_EXTENDED },
		{ "\\`", REG_EXTENDED },
		{ "\\'", REG_EXTENDED },
		{ "test", REG_EXTENDED | REG_DUMP },
		{ "a", REG_EXTENDED | REG_INVARG },
		{ "a", REG_EXTENDED | REG_NOSPEC },
	};

	for (size_t i = 0; i < sizeof(basic) / sizeof(basic[0]); i++) {
		if ((basic[i].cflags & REG_EXTENDED) && (basic[i].cflags & REG_NOSPEC))
			continue;
		check_one(basic[i].pat, basic[i].cflags, NULL, "hand");
	}

	/* REG_PEND cases */
	{
		const char data[] = "abc\x00def\xff";
		check_one(data, REG_EXTENDED | REG_PEND, data + 3, "pend1");
		check_one(data, REG_EXTENDED | REG_PEND, data + 1, "pend2");
		check_one(data, REG_NOSPEC | REG_PEND, data + 7, "pend3");
		check_one(data, REG_BASIC | REG_PEND, data + 3, "pend4");
	}

	/* invalid REG_PEND */
	{
		const char data[] = "abc";
		GuardedRegex pg, rg;
		guard_init(&pg);
		guard_init(&rg);
		pg.re.re_endp = data - 1;
		rg.re.re_endp = data - 1;
		st_regcomp.cases++;
		int pr = P::regcomp(&pg.re, data, REG_EXTENDED | REG_PEND);
		int rr = ref_regcomp(&rg.re, data, REG_EXTENDED | REG_PEND);
		if (pr != rr || pr != REG_INVARG)
			fail(st_regcomp, "pend-invalid ret port=%d ref=%d", pr, rr);
	}

	/* high-bit byte patterns */
	for (int b = 0x80; b <= 0xff; b++) {
		char p[4] = { (char)b, '\0', '\0', '\0' };
		check_one(p, REG_EXTENDED, NULL, "hibyte");
		char p2[6];
		std::snprintf(p2, sizeof(p2), "[%c]", b);
		check_one(p2, REG_EXTENDED, NULL, "hibyte-br");
	}

	/* boundary lengths around Boyer-Moore threshold (mlen > 3) */
	{
		char longpat[64];
		for (int n = 1; n <= 20; n++) {
			for (int i = 0; i < n; i++)
				longpat[i] = (char)('a' + (i % 26));
			longpat[n] = '\0';
			check_one(longpat, REG_EXTENDED, NULL, "long");
			check_one(longpat, REG_BASIC, NULL, "long-bre");
		}
	}

	/* bracket edge cases */
	check_one("[:<:]]", REG_EXTENDED, NULL, "bow-kludge");
	check_one("[:>:]]", REG_EXTENDED, NULL, "eow-kludge");
	check_one("[[:<:]]", REG_BASIC, NULL, "bow-bre");
	check_one("[[:>:]]", REG_BASIC, NULL, "eow-bre");
}

/* ------------------------------------------------------------------ */
/* random pattern generator                                           */
/* ------------------------------------------------------------------ */

static const char *const FRAGS[] = {
	"a", "b", "c", ".", "*", "+", "?", "^", "$", "|",
	"(", ")", "[", "]", "\\", "{", "}", "1", "2", "9",
	"[a]", "[^a]", "[a-z]", "[]]", "[-a]", "[a-]",
	"[[:alpha:]]", "[[:digit:]]", "[[:space:]]",
	"\\w", "\\W", "\\s", "\\S", "\\b", "\\B",
	"\\<", "\\>", "\\(", "\\)", "\\{", "\\}", "\\1",
	"(a)", "(a)(b)", "(a)\\1", "a|b", "a|", "|a",
	"a*", "a+", "a?", "a{1}", "a{1,3}", "a{0,0}", "a{255}",
	"\xc3\xa9", "\xff", "\x80", "\n",
};

static void
random_sweep(void)
{
	char buf[128];
	unsigned char raw[96];

	for (unsigned long i = 0; i < 10000; i++) {
		int kind = (int)rng_below(8);
		int cflags;
		unsigned int fl;

		fl = rng_below(16);
		cflags = REG_EXTENDED;
		if (fl & 1)
			cflags = REG_BASIC;
		if (fl & 2)
			cflags |= REG_ICASE;
		if (fl & 4)
			cflags |= REG_NEWLINE;
		if (fl & 8)
			cflags |= REG_NOSUB;
		if ((fl & 3) == 3)
			cflags |= REG_POSIX;
		if (rng_bit() && !(cflags & REG_EXTENDED))
			; /* BRE */
		else if (rng_bit())
			cflags = REG_NOSPEC;

		if ((cflags & REG_EXTENDED) && (cflags & REG_NOSPEC))
			cflags &= ~REG_NOSPEC;

		switch (kind) {
		case 0: {
			unsigned int len = rng_below(32);
			for (unsigned int j = 0; j < len; j++)
				buf[j] = (char)(1 + rng_below(255));
			buf[len] = '\0';
			if (rng_bit() && len > 0)
				check_one(buf, cflags | REG_PEND, buf + (len / 2 + 1),
				    "rand-raw");
			else
				check_one(buf, cflags, NULL, "rand-raw");
			break;
		}
		case 1: {
			unsigned int nfrag = 1 + rng_below(6);
			buf[0] = '\0';
			for (unsigned int j = 0; j < nfrag; j++) {
				const char *f = FRAGS[rng_below(
				    (unsigned)(sizeof(FRAGS) / sizeof(FRAGS[0])))];
				if (std::strlen(buf) + std::strlen(f) < sizeof(buf) - 1)
					std::strcat(buf, f);
			}
			check_one(buf, cflags, NULL, "rand-frag");
			break;
		}
		case 2: {
			unsigned int len = rng_below(64);
			for (unsigned int j = 0; j < len; j++)
				raw[j] = (unsigned char)rng_below(256);
			if (len >= sizeof(buf))
				len = (unsigned)(sizeof(buf) - 1);
			std::memcpy(buf, raw, len);
			buf[len] = '\0';
			check_one(buf, cflags | REG_PEND, buf + len, "rand-nul");
			break;
		}
		case 3:
			check_one("", cflags, NULL, "rand-empty");
			break;
		case 4: {
			char p[8];
			std::snprintf(p, sizeof(p), "a{%u}", 1 + rng_below(300));
			check_one(p, cflags, NULL, "rand-count");
			break;
		}
		case 5: {
			char p[16];
			int lo = (int)rng_below(10);
			int hi = lo + (int)rng_below(10);
			std::snprintf(p, sizeof(p), "x{%d,%d}", lo, hi);
			check_one(p, cflags, NULL, "rand-range");
			break;
		}
		case 6: {
			char p[48];
			int n = 4 + (int)rng_below(20);
			for (int j = 0; j < n && (size_t)j < sizeof(p) - 1; j++)
				p[j] = (char)('a' + (j % 26));
			p[n] = '\0';
			check_one(p, cflags, NULL, "rand-lit");
			break;
		}
		default: {
			unsigned int len = 1 + rng_below(80);
			for (unsigned int j = 0; j < len && j < sizeof(buf) - 1; j++) {
				unsigned int r = rng_below(100);
				if (r < 60)
					buf[j] = (char)(' ' + rng_below(95));
				else if (r < 80)
					buf[j] = (char)(0x80 + rng_below(128));
				else
					buf[j] = (char)rng_below(256);
			}
			buf[len] = '\0';
			check_one(buf, cflags, NULL, "rand-mix");
			break;
		}
		}
	}
}

/* ------------------------------------------------------------------ */

int
main(void)
{
	std::setlocale(LC_ALL, "C");

	rng_seed(0xb0336deadbeefull);

	handcrafted();
	random_sweep();

	std::printf("function            cases     failures\n");
	std::printf("---------------------------------------\n");
	std::printf("%-16s %8lu %10lu\n",
	    st_regcomp.name, st_regcomp.cases, st_regcomp.fails);

	return (st_regcomp.fails == 0 ? 0 : 1);
}
