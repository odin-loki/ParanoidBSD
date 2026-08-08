// b0154s2 differential harness: C++23 port vs. the untouched C oracle.
//
// Every function of the batch is driven with hand-written edge cases and a
// fixed-seed randomised sweep of at least 200000 iterations.  Buffer-writing
// functions get two independently guarded buffers whose ENTIRE contents
// (including the 16 guard bytes on either side of the nominal window) are
// compared after the call.  Pointers are always compared as offsets from the
// respective buffer base.

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE 1
#endif

#include <sys/types.h>

#include <arpa/nameser.h>

#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

import pbsd.lib.libc.nameser.b0154s2;

namespace P = pbsd::lib_libc_nameser::b0154s2;

extern "C" {
int	ref_fmt1(int t, char s, char **buf, size_t *buflen);
int	ref_ns_format_ttl(u_long src, char *dst, size_t dstlen);
int	ref_ns_parse_ttl(const char *src, u_long *dst);
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
	unsigned char *base(void) { return raw + PRE; }
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

/* ------------------------------------------------------------ ns_ttl.c */

static void
one_fmt1(Stat &s, int t, char c, size_t buflen)
{
	Buf A, B;

	A.fill();
	B.fill();
	char *pa = A.cbase();
	char *pb = B.cbase();
	size_t la = buflen, lb = buflen;
	errno = 0;
	int ra = P::fmt1(t, c, &pa, &la);
	int ea = errno;
	errno = 0;
	int rb = ref_fmt1(t, c, &pb, &lb);
	int eb = errno;

	bool ok = ra == rb && la == lb && ea == eb &&
	    (pa - A.cbase()) == (pb - B.cbase()) && sameraw(A, B);
	note(s, ok, "t=%d c=%#02x buflen=%zu: r %d/%d off %ld/%ld left %zu/%zu "
	    "errno %d/%d buf@%ld", t, (unsigned char)c, buflen, ra, rb,
	    (long)(pa - A.cbase()), (long)(pb - B.cbase()), la, lb, ea, eb,
	    firstdiff(A, B));
}

static void
t_fmt1(void)
{
	Stat &s = mkstat("fmt1");
	static const int tv[] = {
		0, 1, -1, 9, 10, -9, -10, 99, 100, 999, 1000, 12345, -12345,
		INT_MAX, INT_MIN, 2147483647, -2147483647,
	};
	static const char cv[] = {
		'W', 'D', 'H', 'M', 'S', 'w', 's', '0', '\0', '\x01', '\x7f',
		'\x80', '\xff',
	};

	for (unsigned i = 0; i < sizeof tv / sizeof tv[0]; i++)
		for (unsigned j = 0; j < sizeof cv / sizeof cv[0]; j++)
			for (size_t L = 0; L <= 16; L++)
				one_fmt1(s, tv[i], cv[j], L);
	for (unsigned i = 0; i < sizeof tv / sizeof tv[0]; i++) {
		char tmp[64];
		size_t len = (size_t)snprintf(tmp, sizeof tmp, "%dS", tv[i]);
		for (size_t d = 0; d <= 3; d++) {
			if (len + 1 < d)
				continue;
			one_fmt1(s, tv[i], 'S', len + 1 - d);
			one_fmt1(s, tv[i], 'S', len + 1 + d);
		}
	}
	for (unsigned long it = 0; it < 200000; it++) {
		int t;
		switch (rndn(4)) {
		case 0: t = (int)rndn(100); break;
		case 1: t = (int)rnd32(); break;
		case 2: t = -(int)rndn(100); break;
		default: t = (int)(rnd32() & 0xffff) - 32768; break;
		}
		char c = (char)rndn(256);
		size_t L = (size_t)rndn(24);
		if ((it & 7) == 0)
			L = (size_t)rndn(WIN + 1);
		one_fmt1(s, t, c, L);
	}
}

static void
one_format_ttl(Stat &s, u_long src, size_t dstlen)
{
	Buf A, B;

	A.fill();
	B.fill();
	errno = 0;
	int ra = P::ns_format_ttl(src, A.cbase(), dstlen);
	int ea = errno;
	errno = 0;
	int rb = ref_ns_format_ttl(src, B.cbase(), dstlen);
	int eb = errno;

	note(s, ra == rb && ea == eb && sameraw(A, B),
	    "src=%lu dstlen=%zu: r %d/%d errno %d/%d buf@%ld", src, dstlen,
	    ra, rb, ea, eb, firstdiff(A, B));
}

static void
t_ns_format_ttl(void)
{
	Stat &s = mkstat("ns_format_ttl");
	static const u_long sv[] = {
		0, 1, 2, 59, 60, 61, 119, 120, 3599, 3600, 3601, 86399, 86400,
		86401, 604799, 604800, 604801, 1209600, 90061, 3661, 3660,
		0x7ffffffful, 0x80000000ul, 0xfffffffful, 0x100000000ul,
		0x7ffffffffffffffful, 0x8000000000000000ul, 0xfffffffffffffffful,
		604800ul * 1000ul, 604800ul * 0x100000000ul,
		604800ul * 0x80000000ul + 1ul,
	};

	for (unsigned i = 0; i < sizeof sv / sizeof sv[0]; i++)
		for (size_t L = 0; L <= 80; L++)
			one_format_ttl(s, sv[i], L);
	for (unsigned i = 0; i < sizeof sv / sizeof sv[0]; i++) {
		one_format_ttl(s, sv[i], 100);
		one_format_ttl(s, sv[i], WIN);
	}
	for (unsigned long it = 0; it < 200000; it++) {
		u_long src;
		switch (rndn(6)) {
		case 0: src = rndn(120); break;
		case 1: src = rndn(100000); break;
		case 2: src = rnd32(); break;
		case 3: src = rnd64(); break;
		case 4: src = 604800ul * (1ul + rndn(100000)); break;
		default: src = 60ul * rndn(1000000); break;
		}
		size_t L = (size_t)rndn(72);
		if ((it & 15) == 0)
			L = (size_t)rndn(WIN + 1);
		one_format_ttl(s, src, L);
	}
}

static void
one_parse_ttl(Stat &s, const char *str)
{
	u_long da = 0xa5a5a5a5a5a5a5a5ul, db = 0xa5a5a5a5a5a5a5a5ul;

	errno = 0;
	int ra = P::ns_parse_ttl(str, &da);
	int ea = errno;
	errno = 0;
	int rb = ref_ns_parse_ttl(str, &db);
	int eb = errno;

	note(s, ra == rb && da == db && ea == eb,
	    "\"%s\": r %d/%d val %lu/%lu errno %d/%d", str, ra, rb, da, db,
	    ea, eb);
}

static void
t_ns_parse_ttl(void)
{
	Stat &s = mkstat("ns_parse_ttl");
	static const char *ev[] = {
		"", "0", "1", "9", "10", "007", "4294967295", "4294967296",
		"18446744073709551615", "18446744073709551616",
		"99999999999999999999999999",
		"W", "D", "H", "M", "S", "w", "d", "h", "m", "s",
		"1W", "1D", "1H", "1M", "1S", "1w", "1d", "1h", "1m", "1s",
		"0W", "0S", "1w2d", "1w2d3h4m5s", "5s4m3h2d1w",
		"1W2", "1S1", "1S1S", "1s1s", "1w1w", "12w34d",
		"1x", "x", "1X", "-1", "+1", "1.", "1 ", " 1", "1\t", "\t",
		"1-", "1_", "1z", "1@", "1[", "1{", "1`", "1|",
		"\x01", "\x1f", "\x7f", "\x80", "\xff", "\xfe",
		"1\x7f", "1\x80", "1\xff", "\x80" "1", "\xff" "1",
		"1S\x80", "1S1S1S1S", "9999999999999999999S",
		"1WW", "1SS", "1MS", "1SM", "1HM", "1DW",
		"2s", "60s", "3600s", "1m1", "1h1", "1d1", "1w1",
		"00000000000000000000001s", "0000000000s",
	};

	for (unsigned i = 0; i < sizeof ev / sizeof ev[0]; i++)
		one_parse_ttl(s, ev[i]);

	static const char alpha[] = {
		'0', '1', '2', '5', '9', '8', '7',
		'W', 'D', 'H', 'M', 'S', 'w', 'd', 'h', 'm', 's',
		'x', 'X', 'z', '.', '-', ' ', '\t', '\n', '+', '_',
		'\x01', '\x1f', '\x7f', (char)0x80, (char)0xa0, (char)0xff,
	};
	char buf[24];
	for (unsigned long it = 0; it < 200000; it++) {
		unsigned n;
		switch (rndn(4)) {
		case 0: n = rndn(4); break;
		case 1: n = rndn(8); break;
		case 2: n = rndn(14); break;
		default: n = rndn(23); break;
		}
		for (unsigned i = 0; i < n; i++) {
			if (rndn(3) != 0)
				buf[i] = (char)('0' + rndn(10));
			else
				buf[i] = alpha[rndn(sizeof alpha)];
		}
		buf[n] = '\0';
		one_parse_ttl(s, buf);
	}
	static const char units[] = { 'W', 'D', 'H', 'M', 'S', 'w', 'd', 'h',
	    'm', 's', 'X', 'q', '.', (char)0x80 };
	for (unsigned long it = 0; it < 200000; it++) {
		unsigned o = 0;
		unsigned groups = 1 + rndn(3);
		for (unsigned g = 0; g < groups && o < 18; g++) {
			unsigned nd = rndn(3);
			for (unsigned i = 0; i < nd && o < 18; i++)
				buf[o++] = (char)('0' + rndn(10));
			if (rndn(4) != 0 && o < 19)
				buf[o++] = units[rndn(sizeof units)];
		}
		buf[o] = '\0';
		one_parse_ttl(s, buf);
	}
}

/* ------------------------------------------------------------------ main */

int
main(void)
{
	rng_seed(0x0154s2C0FFEE123456ULL);

	printf("=== b0154s2 nameser differential test ===\n");

	t_fmt1();
	t_ns_format_ttl();
	t_ns_parse_ttl();

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
