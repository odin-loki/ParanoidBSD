/*
 * Differential test harness for PBSD batch b0124s1.
 *
 *   port   : pbsd::sbin_ipf_libipf::b0124s1::printifname()
 *   oracle : ref_printifname()   (unmodified HardenedBSD C)
 *
 * printifname() has no return value; its entire observable behaviour is the
 * byte stream it writes to stdout.  So stdout is redirected into an in-memory
 * stream for each call and the two byte streams are compared in full.  The
 * two input strings are additionally handed to each implementation in its own
 * guard-filled buffer (0x7f) and the ENTIRE buffers -- including every byte
 * past the terminating NUL -- are compared afterwards, against each other and
 * against a pristine copy.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <string>
#include <vector>

import pbsd.sbin.ipf.libipf.b0124s1;

extern "C" void ref_printifname(char *format, char *name, void *ifp);

namespace port = pbsd::sbin_ipf_libipf::b0124s1;

/* ------------------------------------------------------------------ */
/* bookkeeping                                                         */
/* ------------------------------------------------------------------ */

struct Stats {
	const char	*name;
	unsigned long	cases;
	unsigned long	fails;
	unsigned long	reported;
};

static Stats st_printifname = { "printifname", 0, 0, 0 };

static const unsigned long MAX_REPORT = 12;

/* ------------------------------------------------------------------ */
/* stdout capture                                                      */
/* ------------------------------------------------------------------ */

template <class F>
static std::string
capture(F fn)
{
	char *mem = NULL;
	size_t len = 0;
	FILE *ms = open_memstream(&mem, &len);

	if (ms == NULL) {
		perror("open_memstream");
		exit(2);
	}

	FILE *saved = stdout;
	stdout = ms;
	fn();
	fflush(ms);
	stdout = saved;
	fclose(ms);

	std::string out(mem, len);
	free(mem);
	return out;
}

/* ------------------------------------------------------------------ */
/* pretty printing of arbitrary byte strings                           */
/* ------------------------------------------------------------------ */

static std::string
esc(const unsigned char *p, size_t n)
{
	std::string s;
	char tmp[8];

	for (size_t i = 0; i < n; i++) {
		unsigned char c = p[i];
		if (c >= 0x20 && c < 0x7f && c != '\\') {
			s += (char)c;
		} else {
			snprintf(tmp, sizeof(tmp), "\\x%02x", c);
			s += tmp;
		}
	}
	return s;
}

static std::string
esc(const std::string &v)
{
	return esc((const unsigned char *)v.data(), v.size());
}

/* ------------------------------------------------------------------ */
/* the one case runner                                                 */
/* ------------------------------------------------------------------ */

#define	BUFSZ	192
#define	GUARD	0x7f
#define	MAXIN	128		/* longest string we ever plant in a buffer */

static void
plant(unsigned char *buf, const unsigned char *src, size_t n)
{
	memset(buf, GUARD, BUFSZ);
	if (n > 0)
		memcpy(buf, src, n);
	buf[n] = '\0';
}

static void
case_printifname(const unsigned char *fmt, size_t fmtlen,
		 const unsigned char *nam, size_t namlen,
		 int have_ifp, const char *tag)
{
	unsigned char fa[BUFSZ], fb[BUFSZ], f0[BUFSZ];
	unsigned char na[BUFSZ], nb[BUFSZ], n0[BUFSZ];

	if (fmtlen >= MAXIN || namlen >= MAXIN) {
		fprintf(stderr, "internal: input too long\n");
		exit(2);
	}

	plant(fa, fmt, fmtlen);
	plant(fb, fmt, fmtlen);
	plant(f0, fmt, fmtlen);
	plant(na, nam, namlen);
	plant(nb, nam, namlen);
	plant(n0, nam, namlen);

	/*
	 * Two distinct sentinel objects so that a port which compared the
	 * pointers themselves rather than against NULL would still be
	 * exercised, and so the two runs never share state.
	 */
	long sentinel_a = 0x1234;
	long sentinel_b = 0x5678;
	void *ifpa = have_ifp ? (void *)&sentinel_a : (void *)NULL;
	void *ifpb = have_ifp ? (void *)&sentinel_b : (void *)NULL;

	std::string outa = capture([&] {
		port::printifname((char *)fa, (char *)na, ifpa);
	});
	std::string outb = capture([&] {
		ref_printifname((char *)fb, (char *)nb, ifpb);
	});

	int bad_out = (outa != outb);
	int bad_fmt = (memcmp(fa, fb, BUFSZ) != 0) || (memcmp(fa, f0, BUFSZ) != 0);
	int bad_nam = (memcmp(na, nb, BUFSZ) != 0) || (memcmp(na, n0, BUFSZ) != 0);

	st_printifname.cases++;
	if (bad_out || bad_fmt || bad_nam) {
		st_printifname.fails++;
		if (st_printifname.reported < MAX_REPORT) {
			st_printifname.reported++;
			fprintf(stderr,
			    "FAIL printifname [%s] format=\"%s\" name=\"%s\" ifp=%s\n",
			    tag, esc(fmt, fmtlen).c_str(),
			    esc(nam, namlen).c_str(),
			    have_ifp ? "non-NULL" : "NULL");
			if (bad_out)
				fprintf(stderr,
				    "     stdout port=\"%s\" (%zu) ref=\"%s\" (%zu)\n",
				    esc(outa).c_str(), outa.size(),
				    esc(outb).c_str(), outb.size());
			if (bad_fmt)
				fprintf(stderr,
				    "     format buffer clobbered\n");
			if (bad_nam)
				fprintf(stderr,
				    "     name buffer clobbered\n");
		}
	}
}

static void
case_str(const char *fmt, const char *nam, int have_ifp, const char *tag)
{
	case_printifname((const unsigned char *)fmt, strlen(fmt),
			 (const unsigned char *)nam, strlen(nam),
			 have_ifp, tag);
}

/* run with both NULL and non-NULL ifp */
static void
case_both(const char *fmt, const char *nam, const char *tag)
{
	case_str(fmt, nam, 0, tag);
	case_str(fmt, nam, 1, tag);
}

/* ------------------------------------------------------------------ */
/* hand written edge cases                                             */
/* ------------------------------------------------------------------ */

static void
edge_cases(void)
{
	/*
	 * The three predicates in
	 *     (ifp == NULL) && strcmp(name,"-") && strcmp(name,"*")
	 * are covered with every combination of truth values so that
	 * flipping == to !=, or && to ||, changes the printed bytes.
	 */
	case_str("", "-", 0, "T/F/x  ifp NULL, name is -");
	case_str("", "*", 0, "T/T/F  ifp NULL, name is *");
	case_str("", "x", 0, "T/T/T  ifp NULL, name is other");
	case_str("", "-", 1, "F/F/x  ifp set,  name is -");
	case_str("", "*", 1, "F/T/F  ifp set,  name is *");
	case_str("", "x", 1, "F/T/T  ifp set,  name is other");

	/* the empty name: differs from both "-" and "*" */
	case_both("", "", "empty format, empty name");
	case_both("dev:", "", "empty name");
	case_both("", "de0", "empty format");

	/* names one byte away from the two magic strings */
	static const char *nearby[] = {
		"-", "*", "--", "**", "-*", "*-", "-x", "*x", "x-", "x*",
		" -", " *", "- ", "* ", "-\t", "*\t", ".", ",", "+", "/",
		"\x2d", "\x2a", "\x2c", "\x2b", "\x29", "\x2e",
		"-0", "*0", "0-", "0*", "any", "all", "none",
		"lo0", "de0", "em0", "vlan0", "ppp0",
		NULL
	};
	for (int i = 0; nearby[i] != NULL; i++) {
		case_both("if=", nearby[i], "nearby name");
		case_both("", nearby[i], "nearby name, no format");
	}

	/* every single byte value as a one character name */
	for (int c = 1; c < 256; c++) {
		unsigned char b[1];
		b[0] = (unsigned char)c;
		case_printifname((const unsigned char *)"F", 1, b, 1, 0,
				 "single byte name, ifp NULL");
		case_printifname((const unsigned char *)"F", 1, b, 1, 1,
				 "single byte name, ifp set");
	}

	/* every single byte value as a one character format */
	for (int c = 1; c < 256; c++) {
		unsigned char b[1];
		b[0] = (unsigned char)c;
		case_printifname(b, 1, (const unsigned char *)"-", 1, 0,
				 "single byte format, name -");
		case_printifname(b, 1, (const unsigned char *)"*", 1, 1,
				 "single byte format, name *");
		case_printifname(b, 1, (const unsigned char *)"q", 1, 0,
				 "single byte format, name q");
	}

	/* two byte names built off the magic first characters */
	for (int c = 1; c < 256; c++) {
		unsigned char b[2];
		b[0] = '-';
		b[1] = (unsigned char)c;
		case_printifname((const unsigned char *)"", 0, b, 2, 0,
				 "-<byte>");
		b[0] = '*';
		case_printifname((const unsigned char *)"", 0, b, 2, 0,
				 "*<byte>");
		b[0] = (unsigned char)c;
		b[1] = '-';
		case_printifname((const unsigned char *)"", 0, b, 2, 0,
				 "<byte>-");
		b[1] = '*';
		case_printifname((const unsigned char *)"", 0, b, 2, 0,
				 "<byte>*");
	}

	/* high bit bytes 0x80..0xff on their own and in runs */
	for (int c = 0x80; c < 0x100; c++) {
		unsigned char b[4];
		b[0] = (unsigned char)c;
		b[1] = (unsigned char)c;
		b[2] = (unsigned char)c;
		b[3] = (unsigned char)c;
		case_printifname(b, 4, b, 4, 0, "high bit run, ifp NULL");
		case_printifname(b, 4, b, 4, 1, "high bit run, ifp set");
		unsigned char m[2];
		m[0] = '-';
		m[1] = (unsigned char)c;
		case_printifname(b, 4, m, 2, 0, "high bit after -");
		m[0] = '*';
		case_printifname(b, 4, m, 2, 0, "high bit after *");
	}

	/* NUL heavy buffers: only the leading string is visible */
	{
		static const unsigned char nulheavy[] = {
			'a', 0x00, 'b', 0x00, 0x00, 'c', 0xff, 0x00, '-', 0x00
		};
		case_printifname(nulheavy, sizeof(nulheavy),
				 nulheavy, sizeof(nulheavy), 0, "NUL heavy");
		case_printifname(nulheavy, sizeof(nulheavy),
				 nulheavy, sizeof(nulheavy), 1, "NUL heavy");

		static const unsigned char dashnul[] = { '-', 0x00, 'x', 'y' };
		case_printifname(dashnul, sizeof(dashnul),
				 dashnul, sizeof(dashnul), 0, "- then NUL");

		static const unsigned char starnul[] = { '*', 0x00, 0xff, 'z' };
		case_printifname(starnul, sizeof(starnul),
				 starnul, sizeof(starnul), 0, "* then NUL");

		static const unsigned char leadnul[] = { 0x00, '-', '-', '-' };
		case_printifname(leadnul, sizeof(leadnul),
				 leadnul, sizeof(leadnul), 0, "leading NUL");
	}

	/* percent signs: format and name are %s arguments, never formats */
	static const char *pct[] = {
		"%", "%%", "%s", "%d", "%p", "%99999d", "%s%s", "%-", "%*",
		NULL
	};
	for (int i = 0; pct[i] != NULL; i++) {
		case_both(pct[i], "de0", "percent in format");
		case_both("if=", pct[i], "percent in name");
		case_both(pct[i], pct[i], "percent in both");
	}

	/* boundary lengths, both plain and high bit filled */
	for (size_t n = 0; n <= 72; n++) {
		unsigned char a[MAXIN], h[MAXIN];
		for (size_t i = 0; i < n; i++) {
			a[i] = (unsigned char)('A' + (i % 26));
			h[i] = (unsigned char)(0x80 + (i % 0x80));
		}
		case_printifname(a, n, a, n, 0, "length sweep ascii");
		case_printifname(h, n, h, n, 1, "length sweep high bit");
		case_printifname(a, n, (const unsigned char *)"-", 1, 0,
				 "length sweep format, name -");
		case_printifname(h, n, (const unsigned char *)"*", 1, 0,
				 "length sweep format, name *");
		case_printifname((const unsigned char *)"", 0, a, n, 0,
				 "length sweep name only");
	}

	/* longest strings we allow */
	{
		unsigned char big[MAXIN];
		for (size_t i = 0; i < MAXIN - 1; i++)
			big[i] = (unsigned char)(1 + (i % 255));
		case_printifname(big, MAXIN - 1, big, MAXIN - 1, 0, "max length");
		case_printifname(big, MAXIN - 1, big, MAXIN - 1, 1, "max length");
		case_printifname(big, MAXIN - 1, (const unsigned char *)"-", 1,
				 0, "max format, name -");
		case_printifname((const unsigned char *)"-", 1, big, MAXIN - 1,
				 0, "max name");
	}
}

/* ------------------------------------------------------------------ */
/* fixed seed randomised sweep                                         */
/* ------------------------------------------------------------------ */

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

static unsigned
rng_below(unsigned n)
{
	return (unsigned)(rng_next() % n);
}

static const unsigned char alpha_magic[] = { '-', '*' };
static const unsigned char alpha_small[] = { '-', '*', 'x', '%', 0x80, 0xff };
static const unsigned char alpha_high[]  = { 0x80, 0x81, 0xfe, 0xff, 0x7f, '-' };

static size_t
gen_string(unsigned char *out, unsigned mode, size_t maxlen)
{
	size_t n;

	switch (mode) {
	case 0:				/* exactly "-" */
		out[0] = '-';
		return 1;
	case 1:				/* exactly "*" */
		out[0] = '*';
		return 1;
	case 2:				/* empty */
		return 0;
	case 3:				/* short, magic bytes only */
		n = rng_below(4) + 1;
		for (size_t i = 0; i < n; i++)
			out[i] = alpha_magic[rng_below(sizeof(alpha_magic))];
		return n;
	case 4:				/* short, mixed small alphabet */
		n = rng_below(6);
		for (size_t i = 0; i < n; i++)
			out[i] = alpha_small[rng_below(sizeof(alpha_small))];
		return n;
	case 5:				/* high bit heavy */
		n = rng_below(8);
		for (size_t i = 0; i < n; i++)
			out[i] = alpha_high[rng_below(sizeof(alpha_high))];
		return n;
	default:			/* arbitrary bytes, arbitrary length */
		n = (size_t)rng_below((unsigned)maxlen);
		for (size_t i = 0; i < n; i++)
			out[i] = (unsigned char)(1 + rng_below(255));
		return n;
	}
}

static void
random_sweep(unsigned long iters)
{
	unsigned char fmt[MAXIN], nam[MAXIN];

	rng_seed(0x0B01245100000001ULL);	/* fixed seed */

	for (unsigned long i = 0; i < iters; i++) {
		unsigned fmode = rng_below(8);
		unsigned nmode = rng_below(8);
		size_t flen = gen_string(fmt, fmode, 40);
		size_t nlen = gen_string(nam, nmode, 40);
		int have_ifp = (int)(rng_next() & 1);

		case_printifname(fmt, flen, nam, nlen, have_ifp, "random");
	}
}

/* ------------------------------------------------------------------ */

int
main(void)
{
	edge_cases();
	random_sweep(250000UL);

	unsigned long total_cases = st_printifname.cases;
	unsigned long total_fails = st_printifname.fails;

	printf("\n");
	printf("+----------------------+------------+------------+\n");
	printf("| %-20s | %10s | %10s |\n", "function", "cases", "failures");
	printf("+----------------------+------------+------------+\n");
	printf("| %-20s | %10lu | %10lu |\n", st_printifname.name,
	    st_printifname.cases, st_printifname.fails);
	printf("+----------------------+------------+------------+\n");
	printf("| %-20s | %10lu | %10lu |\n", "TOTAL", total_cases, total_fails);
	printf("+----------------------+------------+------------+\n");
	printf("\n%s\n", total_fails == 0 ? "PASS: port matches oracle"
					  : "FAIL: port diverges from oracle");

	return total_fails == 0 ? 0 : 1;
}
