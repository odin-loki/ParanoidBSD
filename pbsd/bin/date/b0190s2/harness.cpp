/*
 * Differential test harness for PBSD batch b0190s2 (bin/date/date.c).
 *
 * Every ported function is exercised against the unmodified reference
 * implementation in oracle.c.  For each case both the return value and the
 * complete output buffer (including the guard bytes past the nominal write
 * window) are compared.
 *
 * Batch contents:
 *   strftime_ns()  -- buffer writer, compared byte for byte.
 * There are no pointer-returning or stateful-iterator functions in this batch,
 * so those comparison modes do not apply here.
 */

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <climits>
#include <clocale>
#include <ctime>

import pbsd.bin.date.b0190s2;

extern "C" size_t ref_strftime_ns(char * __restrict s, size_t maxsize,
    const char * __restrict format, const struct tm * __restrict t,
    long nsec, long res);

namespace port = pbsd::bin_date::b0190s2;

/* ------------------------------------------------------------------ */

static const size_t BUFCAP = 4096;
static const unsigned char GUARD = 0x7f;

static long long g_cases = 0;
static long long g_fails = 0;
static int g_reported = 0;

static void
dump(const char *tag, const char *p, size_t n)
{
	fprintf(stderr, "    %s [%zu] \"", tag, n);
	for (size_t i = 0; i < n; i++) {
		unsigned char c = (unsigned char)p[i];
		if (c >= 0x20 && c < 0x7f && c != '"' && c != '\\')
			fputc(c, stderr);
		else
			fprintf(stderr, "\\x%02x", c);
	}
	fprintf(stderr, "\"\n");
}

/*
 * Run one strftime_ns() case through both implementations.
 *
 * Two independent buffers are pre-filled with the guard byte; after the call
 * the whole of both buffers is compared, not just the first `maxsize` bytes
 * and not just the return value.
 */
static void
check(const char *what, size_t maxsize, const char *format,
    const struct tm *t, long nsec, long res)
{
	static char abuf[BUFCAP];
	static char bbuf[BUFCAP];
	struct tm ta, tb;

	if (maxsize > BUFCAP)
		maxsize = BUFCAP;

	memset(abuf, GUARD, BUFCAP);
	memset(bbuf, GUARD, BUFCAP);
	memcpy(&ta, t, sizeof(ta));
	memcpy(&tb, t, sizeof(tb));

	size_t ra = port::strftime_ns(abuf, maxsize, format, &ta, nsec, res);
	size_t rb = ref_strftime_ns(bbuf, maxsize, format, &tb, nsec, res);

	g_cases++;

	bool ok = true;
	if (ra != rb)
		ok = false;
	if (memcmp(abuf, bbuf, BUFCAP) != 0)
		ok = false;
	if (memcmp(&ta, &tb, sizeof(ta)) != 0)
		ok = false;

	if (ok)
		return;

	g_fails++;
	if (g_reported++ < 25) {
		fprintf(stderr,
		    "FAIL strftime_ns [%s] maxsize=%zu nsec=%ld res=%ld\n",
		    what, maxsize, nsec, res);
		dump("format", format, strlen(format));
		fprintf(stderr, "    ret port=%zu ref=%zu\n", ra, rb);
		size_t first = BUFCAP;
		for (size_t i = 0; i < BUFCAP; i++)
			if (abuf[i] != bbuf[i]) { first = i; break; }
		if (first != BUFCAP) {
			size_t n = BUFCAP - first;
			if (n > 48)
				n = 48;
			fprintf(stderr, "    first buffer diff at %zu\n", first);
			dump("port  ", abuf + first, n);
			dump("ref   ", bbuf + first, n);
		}
	}
}

/* ------------------------------------------------------------------ */

static struct tm g_tms[6];
static int g_ntms;

static void
init_tms(void)
{
	static const time_t seeds[] = {
		0,			/* epoch */
		1,
		951827696,		/* 2000-02-29 */
		1234567890,
		2147483647,		/* 32-bit time_t boundary */
		-1,
	};
	g_ntms = 0;
	for (size_t i = 0; i < sizeof(seeds) / sizeof(seeds[0]); i++) {
		time_t tv = seeds[i];
		struct tm tmp;
		if (gmtime_r(&tv, &tmp) != NULL)
			g_tms[g_ntms++] = tmp;
	}
	if (g_ntms == 0) {
		memset(&g_tms[0], 0, sizeof(g_tms[0]));
		g_tms[0].tm_mday = 1;
		g_tms[0].tm_year = 70;
		g_ntms = 1;
	}
}

/* ------------------------------------------------------------------ */

static uint64_t g_rs = 0x9e3779b97f4a7c15ULL;	/* fixed seed */

static inline uint32_t
rnd(void)
{
	g_rs ^= g_rs >> 12;
	g_rs ^= g_rs << 25;
	g_rs ^= g_rs >> 27;
	return (uint32_t)((g_rs * 0x2545f4914f6cdd1dULL) >> 32);
}

static long
rnd_nsec(void)
{
	switch (rnd() % 10) {
	case 0: return 0;
	case 1: return (long)(rnd() % 10);
	case 2: return (long)(rnd() % 100);
	case 3: return 999999999L;
	case 4: return 1000000000L;
	case 5: return -(long)(rnd() % 1000000000u);
	case 6: return LONG_MAX;
	case 7: return LONG_MIN;
	case 8: return 123456789L;
	default: return (long)(rnd() % 1000000000u);
	}
}

static long
rnd_res(void)
{
	switch (rnd() % 10) {
	case 0: return 0;
	case 1: return 1;
	case 2: {
		unsigned e = rnd() % 13;
		long r = 1;
		for (unsigned i = 0; i < e; i++)
			r *= 10;
		return r;
	}
	case 3: return -(long)(rnd() % 1000u) - 1;
	case 4: return LONG_MAX;
	case 5: return LONG_MIN;
	case 6: return 999999999L;
	case 7: return (long)(rnd() % 1000000000u);
	case 8: return 2;
	default: return 1000L;
	}
}

static size_t
rnd_maxsize(void)
{
	switch (rnd() % 8) {
	case 0: return 0;
	case 1: return 1;
	case 2: return rnd() % 24;
	case 3: return rnd() % 80;
	case 4: return BUFCAP;
	case 5: return 1 + rnd() % 12;
	default: return 128 + rnd() % (BUFCAP - 128);
	}
}

/*
 * Build a random format string.  Runs of digits are capped at three so that
 * the `width = width * 10 + *tok - '0'` accumulator in the function under test
 * can never signed-overflow (which would be undefined behaviour and would make
 * the differential comparison meaningless), while still reaching widths well
 * above the nine-digit clamp.
 */
static void
rnd_format(char *out, size_t cap)
{
	static const char alpha[] = {
		'%', '%', '%', '%',
		'N', 'N', 'N',
		'-', '-',
		'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
		'a', 'X', ' ', '.', ':', 'Y', 'm', 'd', 'q', 'n',
		(char)0x01, (char)0x7f, (char)0x80, (char)0xa5, (char)0xff,
	};
	size_t n = rnd() % (cap - 1);
	size_t digits = 0;
	size_t i;

	for (i = 0; i < n; i++) {
		char c = alpha[rnd() % (sizeof(alpha) / sizeof(alpha[0]))];
		if (c >= '0' && c <= '9') {
			if (digits >= 3)
				c = 'a';
			else
				digits++;
		} else {
			digits = 0;
		}
		out[i] = c;
	}
	out[i] = '\0';
}

/* ------------------------------------------------------------------ */

static const char *const g_formats[] = {
	/* degenerate */
	"", "%", "%%", "%%%", "%%%%",
	"N", "NN", "n", "-", "0", "9",
	/* the plain conversion */
	"%N", "%%N", "%%%N", "%%%%N", "%N%N", "%N%NN", "%NN",
	"a%N", "ab%Nc", "%Na%Nb", "%N%%N", "%%N%N", "abc%", "abc%N",
	"%N%N%N%N",
	/* dash flag */
	"%-N", "%--N", "%---N", "%-%N", "%-N%-N", "%-NX%-N",
	"-N", "%x-N", "%--%N",
	/* explicit widths, both sides of every clamp */
	"%0N", "%1N", "%2N", "%3N", "%4N", "%5N", "%6N", "%7N", "%8N",
	"%9N", "%10N", "%11N", "%12N", "%18N", "%19N", "%20N", "%99N",
	"%100N", "%999N",
	"%00N", "%01N", "%09N", "%000N", "%010N", "%090N",
	/* dash and width in both orders (only one order is a conversion) */
	"%-0N", "%-1N", "%-5N", "%-9N", "%-10N", "%-99N",
	"%0-N", "%1-N", "%5-N", "%9-N", "%12-N", "%5-5N",
	"%--5N", "%5--N", "%55-N",
	/* a percent that gets cancelled before the N */
	"%xN", "%qN", "%YN", "% N", "%\tN", "%.N", "%aN", "%zN",
	"%N ", "% %N",
	/* high-bit and control bytes around the conversion */
	"\x80%N", "\xff\xfe%N\x81", "%\x80N", "%N\x80", "\x01%N\x7f",
	"\xc3\xa9%-N\xc3\xa9", "%\xffN", "\x80\x81\x82\x83",
	/* realistic date(1) formats */
	"%+", "%Y-%m-%d", "%Y-%m-%dT%H:%M:%S,%N", "T%H:%M:%S,%-N",
	"%a, %d %b %Y %T %z", ",%N", ":%S", "%Y-%m-%dT%H:%M:%S,%9N%z",
	/* long literal, exercises prefixlen arithmetic */
	"0123456789012345678901234567890123456789%N",
	"0123456789012345678901234567890123456789%-N",
	"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa%12Nbbbbbbbbbb%3N",
};
static const size_t g_nformats = sizeof(g_formats) / sizeof(g_formats[0]);

static const long g_nsecs[] = {
	0L, 1L, 9L, 10L, 99L, 100L, 987L, 123456789L, 999999999L,
	1000000000L, -1L, -123456789L, LONG_MAX, LONG_MIN,
};
static const size_t g_nnsecs = sizeof(g_nsecs) / sizeof(g_nsecs[0]);

static const long g_ress[] = {
	0L, 1L, 2L, 9L, 10L, 11L, 100L, 1000L, 100000L, 1000000L,
	10000000L, 100000000L, 999999999L, 1000000000L, 10000000000L,
	-1L, LONG_MAX, LONG_MIN,
};
static const size_t g_nress = sizeof(g_ress) / sizeof(g_ress[0]);

/* ------------------------------------------------------------------ */

static void
phase_curated(void)
{
	const struct tm *t = &g_tms[0];

	for (size_t f = 0; f < g_nformats; f++)
		for (size_t n = 0; n < g_nnsecs; n++)
			for (size_t r = 0; r < g_nress; r++)
				check("curated", BUFCAP, g_formats[f], t,
				    g_nsecs[n], g_ress[r]);
}

/*
 * Sweep every possible byte value through the three positions where the
 * scanner's state machine can be steered: right after the '%', right after a
 * width/dash flag, and immediately before the '%'.
 */
static void
phase_bytes(void)
{
	const struct tm *t = &g_tms[1];
	char buf[8];

	for (int b = 1; b < 256; b++) {
		for (int shape = 0; shape < 6; shape++) {
			switch (shape) {
			case 0:
				buf[0] = '%'; buf[1] = (char)b; buf[2] = 'N';
				buf[3] = '\0';
				break;
			case 1:
				buf[0] = (char)b; buf[1] = '%'; buf[2] = 'N';
				buf[3] = '\0';
				break;
			case 2:
				buf[0] = '%'; buf[1] = 'N'; buf[2] = (char)b;
				buf[3] = '\0';
				break;
			case 3:
				buf[0] = '%'; buf[1] = '-'; buf[2] = (char)b;
				buf[3] = 'N'; buf[4] = '\0';
				break;
			case 4:
				buf[0] = '%'; buf[1] = '1'; buf[2] = (char)b;
				buf[3] = 'N'; buf[4] = '\0';
				break;
			default:
				buf[0] = '%'; buf[1] = (char)b; buf[2] = '%';
				buf[3] = 'N'; buf[4] = '\0';
				break;
			}
			check("bytes", BUFCAP, buf, t, 123456789L, 1L);
			check("bytes", BUFCAP, buf, t, 999999999L, 1000000L);
			check("bytes", BUFCAP, buf, t, 0L, 0L);
			check("bytes", BUFCAP, buf, t, -7L, LONG_MAX);
		}
	}
}

/* Boundary lengths: every small maxsize around the length of the output. */
static void
phase_lengths(void)
{
	static const char *const fmts[] = {
		"", "%N", "%%N", "%-N", "%1N", "%5N", "%9N", "%12N",
		"ab%Ncd", "%N%N", "%-N%3N", "%Y-%m-%dT%H:%M:%S,%N",
		"%20N", "\x80%N",
	};
	const size_t nf = sizeof(fmts) / sizeof(fmts[0]);

	for (size_t f = 0; f < nf; f++)
		for (size_t m = 0; m <= 48; m++) {
			check("len", m, fmts[f], &g_tms[2], 123456789L, 1L);
			check("len", m, fmts[f], &g_tms[3], 999999999L,
			    1000000000L);
			check("len", m, fmts[f], &g_tms[0], -1L, 0L);
		}
	for (size_t f = 0; f < nf; f++)
		for (size_t m = 60; m <= 130; m += 7)
			check("len", m, fmts[f], &g_tms[4], 500000000L, 100L);
}

/* All tm values against the interesting formats. */
static void
phase_tms(void)
{
	static const char *const fmts[] = {
		"%N", "%-N", "%5N", "%Y-%m-%dT%H:%M:%S,%N",
		"%a, %d %b %Y %T %z", "%c", "%+", "%Z%N",
	};
	const size_t nf = sizeof(fmts) / sizeof(fmts[0]);

	for (int i = 0; i < g_ntms; i++)
		for (size_t f = 0; f < nf; f++)
			for (size_t n = 0; n < g_nnsecs; n++)
				for (size_t r = 0; r < g_nress; r++)
					check("tm", BUFCAP, fmts[f], &g_tms[i],
					    g_nsecs[n], g_ress[r]);
}

static void
phase_random(long long iters)
{
	char fmt[24];

	for (long long i = 0; i < iters; i++) {
		rnd_format(fmt, sizeof(fmt));
		long nsec = rnd_nsec();
		long res = rnd_res();
		size_t maxsize = rnd_maxsize();
		const struct tm *t = &g_tms[rnd() % (unsigned)g_ntms];
		check("random", maxsize, fmt, t, nsec, res);
	}
}

/* ------------------------------------------------------------------ */

int
main(void)
{
	setenv("TZ", "UTC0", 1);
	tzset();
	setlocale(LC_ALL, "C");

	init_tms();

	phase_curated();
	phase_bytes();
	phase_lengths();
	phase_tms();
	phase_random(250000);

	printf("\n");
	printf("%-24s %12s %12s\n", "function", "cases", "failures");
	printf("%-24s %12s %12s\n", "------------------------",
	    "------------", "------------");
	printf("%-24s %12lld %12lld\n", "strftime_ns", g_cases, g_fails);
	printf("%-24s %12s %12s\n", "------------------------",
	    "------------", "------------");
	printf("%-24s %12lld %12lld\n", "TOTAL", g_cases, g_fails);
	printf("\n%s\n", g_fails == 0 ? "PASS" : "FAIL");

	return g_fails == 0 ? 0 : 1;
}
