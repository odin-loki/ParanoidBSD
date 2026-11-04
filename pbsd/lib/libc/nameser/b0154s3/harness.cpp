// b0154s3 differential harness: C++23 port vs. the untouched C oracle.
//
// Every function of the batch is driven with hand-written edge cases and a
// fixed-seed randomised sweep of at least 200000 iterations.  Buffer-writing
// functions get two independently guarded buffers whose ENTIRE contents
// (including the 16 guard bytes on either side of the nominal window) are
// compared after the call.

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE 1
#endif

#include <sys/types.h>

#include <arpa/nameser.h>

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

import pbsd.lib.libc.nameser.b0154s3;

namespace P = pbsd::lib_libc_nameser::b0154s3;

extern "C" {
int	ref_ns_samedomain(const char *a, const char *b);
int	ref_ns_subdomain(const char *a, const char *b);
int	ref_ns_makecanon(const char *src, char *dst, size_t dstsize);
int	ref_ns_samename(const char *a, const char *b);
}

/* ---------------------------------------------------------------- report */

struct Stat {
	const char	*name;
	unsigned long	cases;
	unsigned long	fails;
	unsigned	shown;
};

enum { MAXSTAT = 8 };
static Stat	stats[MAXSTAT];
static int	nstat;

static Stat &
mkstat(const char *name)
{
	Stat &s = stats[nstat++];
	s.name = name;
	s.cases = 0;
	s.fails = 0;
	s.shown = 0;
	return s;
}

static void
note(Stat &s, bool ok, const char *fmt, ...)
{
	s.cases++;
	if (ok)
		return;
	s.fails++;
	if (s.shown < 8) {
		s.shown++;
		va_list ap;
		va_start(ap, fmt);
		fprintf(stderr, "  MISMATCH %s: ", s.name);
		vfprintf(stderr, fmt, ap);
		fputc('\n', stderr);
		va_end(ap);
	}
}

/* ------------------------------------------------------------------ rng */

static uint64_t rng_state;

static void
rng_seed(uint64_t s)
{
	rng_state = s ? s : 0x9E3779B97F4A7C15ULL;
}

static inline uint64_t
rnd64(void)
{
	uint64_t x = rng_state;
	x ^= x << 13;
	x ^= x >> 7;
	x ^= x << 17;
	rng_state = x;
	return x;
}

static inline uint32_t
rnd32(void)
{
	return (uint32_t)(rnd64() >> 16);
}

static inline unsigned
rndn(unsigned n)
{
	return (unsigned)(rnd32() % n);
}

/* -------------------------------------------------------------- buffers */

enum { PRE = 16, WIN = 512, POST = 16, RAW = PRE + WIN + POST };

struct Buf {
	unsigned char raw[RAW];

	void fill(void) { memset(raw, 0x7f, sizeof raw); }
	char *cbase(void) { return (char *)(raw + PRE); }
};

static bool
sameraw(const Buf &a, const Buf &b)
{
	return memcmp(a.raw, b.raw, RAW) == 0;
}

static long
firstdiff(const Buf &a, const Buf &b)
{
	for (long i = 0; i < RAW; i++)
		if (a.raw[i] != b.raw[i])
			return i - PRE;
	return -1;
}

/* ----------------------------------------------------- ns_samedomain.c */

static const char domalpha[] = {
	'a', 'b', 'c', 'A', 'B', 'C', '.', '\\', '\\', '.', '-', '0',
	(char)0x80, (char)0xff, (char)0xc3, (char)0xa9,
};

static void
rnddom(char *out, unsigned maxlen)
{
	unsigned n = rndn(maxlen + 1);
	for (unsigned i = 0; i < n; i++)
		out[i] = domalpha[rndn(sizeof domalpha)];
	out[n] = '\0';
}

static char
flipcase(char c)
{
	if (c >= 'a' && c <= 'z')
		return (char)(c - 'a' + 'A');
	if (c >= 'A' && c <= 'Z')
		return (char)(c - 'A' + 'a');
	return c;
}

static void
gen_pair(char *a, char *b, unsigned maxlen)
{
	rnddom(a, maxlen);
	size_t la = strlen(a);
	switch (rndn(6)) {
	case 0:
		rnddom(b, maxlen);
		break;
	case 1: {
		size_t off = la ? (size_t)rndn((unsigned)la + 1) : 0;
		strcpy(b, a + off);
		break;
	}
	case 2: {
		size_t off = la ? (size_t)rndn((unsigned)la + 1) : 0;
		strcpy(b, a + off);
		for (char *p = b; *p; p++)
			if (rndn(2))
				*p = flipcase(*p);
		break;
	}
	case 3:
		strcpy(b, a);
		if (la && rndn(2))
			b[la - 1] = '\0';
		else {
			b[la] = domalpha[rndn(sizeof domalpha)];
			b[la + 1] = '\0';
		}
		break;
	case 4: {
		size_t off = la ? (size_t)rndn((unsigned)la + 1) : 0;
		strcpy(b, a + off);
		size_t lb = strlen(b);
		if (lb && rndn(2)) {
			b[lb] = '.';
			b[lb + 1] = '\0';
		}
		break;
	}
	default:
		strcpy(b, a);
		break;
	}
}

static const char *dom_edges[] = {
	"", ".", "..", "...", "a", "a.", "a..", ".a", "A", "A.",
	"\\", "\\.", "\\\\", "\\\\.", "\\\\\\.", "\\\\\\\\.",
	"a\\.", "a\\\\.", "a\\\\\\.", "b", "b.", "ab", "a.b", "a.b.",
	"top", "top.", "foobar.top", "bar.top", "host.foobar.top",
	"HOST.FooBar.TOP", "host.foobar.top.", "oobar.top", "obar.top",
	"x.top", ".top", "a\\.b", "a\\.b.", "a\\\\.b", "x\\.top",
	"foo\\.top", "foo\\\\.top", "\\.top", "\\\\.top",
	"\x80", "\x80.", "\xff\xfe", "a\x80", "\x80" "a", "A\x80",
	"aa", "aaa", "a.a", "a..a", "..a", "a...", "\\a", "\\a.",
};

static void
one_samedomain(Stat &s, const char *a, const char *b)
{
	errno = 0;
	int ra = P::ns_samedomain(a, b);
	int ea = errno;
	errno = 0;
	int rb = ref_ns_samedomain(a, b);
	int eb = errno;
	note(s, ra == rb && ea == eb, "(\"%s\",\"%s\"): r %d/%d errno %d/%d",
	    a, b, ra, rb, ea, eb);
}

static void
one_subdomain(Stat &s, const char *a, const char *b)
{
	errno = 0;
	int ra = P::ns_subdomain(a, b);
	int ea = errno;
	errno = 0;
	int rb = ref_ns_subdomain(a, b);
	int eb = errno;
	note(s, ra == rb && ea == eb, "(\"%s\",\"%s\"): r %d/%d errno %d/%d",
	    a, b, ra, rb, ea, eb);
}

static void
one_samename(Stat &s, const char *a, const char *b)
{
	errno = 0;
	int ra = P::ns_samename(a, b);
	int ea = errno;
	errno = 0;
	int rb = ref_ns_samename(a, b);
	int eb = errno;
	note(s, ra == rb && ea == eb, "(len %zu,len %zu): r %d/%d errno %d/%d",
	    strlen(a), strlen(b), ra, rb, ea, eb);
}

static void
one_makecanon(Stat &s, const char *src, size_t dstsize)
{
	Buf A, B;

	A.fill();
	B.fill();
	errno = 0;
	int ra = P::ns_makecanon(src, A.cbase(), dstsize);
	int ea = errno;
	errno = 0;
	int rb = ref_ns_makecanon(src, B.cbase(), dstsize);
	int eb = errno;

	note(s, ra == rb && ea == eb && sameraw(A, B),
	    "(\"%s\",%zu): r %d/%d errno %d/%d buf@%ld", src, dstsize, ra, rb,
	    ea, eb, firstdiff(A, B));
}

static void
t_ns_samedomain(void)
{
	Stat &s = mkstat("ns_samedomain");
	unsigned ne = sizeof dom_edges / sizeof dom_edges[0];

	for (unsigned i = 0; i < ne; i++)
		for (unsigned j = 0; j < ne; j++)
			one_samedomain(s, dom_edges[i], dom_edges[j]);

	char a[64], b[64];
	for (unsigned long it = 0; it < 200000; it++) {
		gen_pair(a, b, 1 + rndn(12));
		one_samedomain(s, a, b);
	}
}

static void
t_ns_subdomain(void)
{
	Stat &s = mkstat("ns_subdomain");
	unsigned ne = sizeof dom_edges / sizeof dom_edges[0];

	for (unsigned i = 0; i < ne; i++)
		for (unsigned j = 0; j < ne; j++)
			one_subdomain(s, dom_edges[i], dom_edges[j]);

	char a[2048], b[2048];
	for (unsigned long it = 0; it < 200000; it++) {
		if ((it % 5000) == 0) {
			unsigned n = 1020 + rndn(9);
			for (unsigned i = 0; i < n; i++)
				a[i] = domalpha[rndn(sizeof domalpha)];
			a[n] = '\0';
			strcpy(b, a);
			one_subdomain(s, a, b);
			continue;
		}
		gen_pair(a, b, 1 + rndn(12));
		one_subdomain(s, a, b);
	}
}

static void
t_ns_samename(void)
{
	Stat &s = mkstat("ns_samename");
	unsigned ne = sizeof dom_edges / sizeof dom_edges[0];

	for (unsigned i = 0; i < ne; i++)
		for (unsigned j = 0; j < ne; j++)
			one_samename(s, dom_edges[i], dom_edges[j]);

	char a[2048], b[2048];
	for (unsigned n = 1018; n <= 1030; n++) {
		for (unsigned i = 0; i < n; i++)
			a[i] = 'a';
		a[n] = '\0';
		for (unsigned m = 1018; m <= 1030; m++) {
			for (unsigned i = 0; i < m; i++)
				b[i] = 'a';
			b[m] = '\0';
			one_samename(s, a, b);
		}
	}
	for (unsigned long it = 0; it < 200000; it++) {
		if ((it % 1000) == 0) {
			unsigned n = 1018 + rndn(14);
			for (unsigned i = 0; i < n; i++)
				a[i] = domalpha[rndn(sizeof domalpha)];
			a[n] = '\0';
			unsigned m = 1018 + rndn(14);
			for (unsigned i = 0; i < m; i++)
				b[i] = domalpha[rndn(sizeof domalpha)];
			b[m] = '\0';
			one_samename(s, a, b);
			continue;
		}
		gen_pair(a, b, 1 + rndn(12));
		one_samename(s, a, b);
	}
}

static void
t_ns_makecanon(void)
{
	Stat &s = mkstat("ns_makecanon");
	char buf[16];

	static const char tri[3] = { 'a', '.', '\\' };
	for (unsigned len = 0; len <= 7; len++) {
		unsigned long total = 1;
		for (unsigned i = 0; i < len; i++)
			total *= 3;
		for (unsigned long code = 0; code < total; code++) {
			unsigned long c = code;
			for (unsigned i = 0; i < len; i++) {
				buf[i] = tri[c % 3];
				c /= 3;
			}
			buf[len] = '\0';
			for (size_t d = 0; d <= len + 3; d++)
				one_makecanon(s, buf, d);
			one_makecanon(s, buf, WIN);
		}
	}
	for (unsigned i = 0; i < sizeof dom_edges / sizeof dom_edges[0]; i++) {
		size_t n = strlen(dom_edges[i]);
		for (size_t d = 0; d <= n + 3; d++)
			one_makecanon(s, dom_edges[i], d);
		one_makecanon(s, dom_edges[i], WIN);
	}
	char big[64];
	for (unsigned long it = 0; it < 200000; it++) {
		rnddom(big, 1 + rndn(20));
		size_t n = strlen(big);
		size_t d;
		switch (rndn(4)) {
		case 0: d = n + 1; break;
		case 1: d = n + 2; break;
		case 2: d = (size_t)rndn((unsigned)n + 5); break;
		default: d = (size_t)rndn(WIN + 1); break;
		}
		one_makecanon(s, big, d);
	}
}

/* ------------------------------------------------------------------ main */

int
main(void)
{
	rng_seed(0x0154B154C0FFEE12ULL);

	printf("=== b0154s3 nameser differential test ===\n");

	t_ns_samedomain();
	t_ns_subdomain();
	t_ns_samename();
	t_ns_makecanon();

	unsigned long tc = 0, tf = 0;
	printf("\n%-24s %12s %12s\n", "function", "cases", "failures");
	printf("--------------------------------------------------\n");
	for (int i = 0; i < nstat; i++) {
		printf("%-24s %12lu %12lu\n", stats[i].name, stats[i].cases,
		    stats[i].fails);
		tc += stats[i].cases;
		tf += stats[i].fails;
	}
	printf("--------------------------------------------------\n");
	printf("%-24s %12lu %12lu\n", "TOTAL", tc, tf);
	printf("\n%s\n", tf == 0 ? "PASS" : "FAIL");
	return tf == 0 ? 0 : 1;
}
