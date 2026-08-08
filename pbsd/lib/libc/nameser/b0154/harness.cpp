// b0154 differential harness: C++23 port vs. the untouched C oracle.
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

#include <netinet/in.h>
#include <arpa/nameser.h>

#include <errno.h>
#include <limits.h>
#include <resolv.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

import pbsd.lib.libc.nameser.b0154;

namespace P = pbsd::lib_libc_nameser::b0154;

extern "C" {
u_int	ref_ns_get16(const u_char *src);
u_long	ref_ns_get32(const u_char *src);
void	ref_ns_put16(u_int src, u_char *dst);
void	ref_ns_put32(u_long src, u_char *dst);
int	ref_fmt1(int t, char s, char **buf, size_t *buflen);
int	ref_ns_format_ttl(u_long src, char *dst, size_t dstlen);
int	ref_ns_parse_ttl(const char *src, u_long *dst);
int	ref_ns_samedomain(const char *a, const char *b);
int	ref_ns_subdomain(const char *a, const char *b);
int	ref_ns_makecanon(const char *src, char *dst, size_t dstsize);
int	ref_ns_samename(const char *a, const char *b);
int	ref_ns_msg_getflag(ns_msg handle, int flag);
int	ref_ns_skiprr(const u_char *ptr, const u_char *eom, ns_sect section,
		      int count);
int	ref_ns_initparse(const u_char *msg, int msglen, ns_msg *handle);
int	ref_ns_parserr(ns_msg *handle, ns_sect section, int rrnum, ns_rr *rr);
void	ref_setsection(ns_msg *msg, ns_sect sect);
}

/* ---------------------------------------------------------------- report */

struct Stat {
	const char	*name;
	unsigned long	cases;
	unsigned long	fails;
	unsigned	shown;
};

enum { MAXSTAT = 32 };
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

/* Offset of p from base; distinct codes for NULL and for out-of-range. */
static long
poff(const void *p, const unsigned char *base)
{
	if (p == NULL)
		return -1000001;
	const unsigned char *q = (const unsigned char *)p;
	long d = (long)(q - base);
	if (d < -(long)PRE - 4096 || d > (long)(WIN + POST) + 4096)
		return -1000002;
	return d;
}

/* ------------------------------------------------------- ns_msg snapshot */

struct Snap {
	long msg, eom, msg_ptr, sect, rrnum, id, flags;
	long counts[4];
	long sections[4];
};

static Snap
snap(const ns_msg &h, const unsigned char *base)
{
	Snap s;
	s.msg = poff(h._msg, base);
	s.eom = poff(h._eom, base);
	s.msg_ptr = poff(h._msg_ptr, base);
	s.sect = (long)(int)h._sect;
	s.rrnum = (long)h._rrnum;
	s.id = (long)h._id;
	s.flags = (long)h._flags;
	for (int i = 0; i < 4; i++) {
		s.counts[i] = (long)h._counts[i];
		s.sections[i] = poff(h._sections[i], base);
	}
	return s;
}

static bool
snapeq(const Snap &a, const Snap &b)
{
	return memcmp(&a, &b, sizeof a) == 0;
}

static int
snapwhich(const Snap &a, const Snap &b)
{
	const long *x = (const long *)&a, *y = (const long *)&b;
	for (unsigned i = 0; i < sizeof a / sizeof(long); i++)
		if (x[i] != y[i])
			return (int)i;
	return -1;
}

static void
prefill_msg(ns_msg &h, const unsigned char *base)
{
	memset(&h, 0, sizeof h);
	h._msg = base + 5;
	h._eom = base + 6;
	h._id = 0x1234;
	h._flags = 0x5678;
	for (int i = 0; i < ns_s_max; i++) {
		h._counts[i] = (uint16_t)(0x4400 + i);
		h._sections[i] = base + 7 + i;
	}
	h._sect = (ns_sect)3;
	h._rrnum = 12345;
	h._msg_ptr = base + 11;
}

static void
prefill_rr(ns_rr &rr, const unsigned char *base)
{
	memset(&rr, 0x7f, sizeof rr);
	rr.rdata = base + 3;
}

static bool
rreq(const ns_rr &a, const unsigned char *ba, const ns_rr &b,
     const unsigned char *bb)
{
	if (memcmp(a.name, b.name, NS_MAXDNAME) != 0)
		return false;
	if (a.type != b.type || a.rr_class != b.rr_class)
		return false;
	if (a.ttl != b.ttl || a.rdlength != b.rdlength)
		return false;
	return poff(a.rdata, ba) == poff(b.rdata, bb);
}

/* --------------------------------------------------------- ns_netint.c */

static void
t_ns_get16(void)
{
	Stat &s = mkstat("ns_get16");
	static const unsigned char edge[][2] = {
		{ 0x00, 0x00 }, { 0x00, 0x01 }, { 0x01, 0x00 }, { 0x01, 0x02 },
		{ 0x7f, 0x80 }, { 0x80, 0x7f }, { 0x80, 0x00 }, { 0x00, 0x80 },
		{ 0xff, 0x00 }, { 0x00, 0xff }, { 0xff, 0xfe }, { 0xfe, 0xff },
		{ 0xff, 0xff },
	};
	Buf A, B;

	for (unsigned e = 0; e < sizeof edge / sizeof edge[0]; e++) {
		A.fill();
		B.fill();
		memcpy(A.base(), edge[e], 2);
		memcpy(B.base(), edge[e], 2);
		u_int ra = P::ns_get16(A.base());
		u_int rb = ref_ns_get16(B.base());
		note(s, ra == rb && sameraw(A, B),
		    "edge %u: port=%u ref=%u buf@%ld", e, ra, rb,
		    firstdiff(A, B));
	}
	for (unsigned long it = 0; it < 200000; it++) {
		unsigned char v[2];
		v[0] = (unsigned char)rndn(256);
		v[1] = (unsigned char)rndn(256);
		A.fill();
		B.fill();
		memcpy(A.base(), v, 2);
		memcpy(B.base(), v, 2);
		u_int ra = P::ns_get16(A.base());
		u_int rb = ref_ns_get16(B.base());
		note(s, ra == rb && sameraw(A, B),
		    "rand %02x%02x: port=%u ref=%u buf@%ld", v[0], v[1], ra, rb,
		    firstdiff(A, B));
	}
}

static void
t_ns_get32(void)
{
	Stat &s = mkstat("ns_get32");
	static const unsigned char edge[][4] = {
		{ 0, 0, 0, 0 }, { 0, 0, 0, 1 }, { 1, 0, 0, 0 },
		{ 0x12, 0x34, 0x56, 0x78 }, { 0x80, 0, 0, 0 },
		{ 0, 0x80, 0, 0 }, { 0, 0, 0x80, 0 }, { 0, 0, 0, 0x80 },
		{ 0xff, 0xff, 0xff, 0xff }, { 0x7f, 0xff, 0xff, 0xff },
		{ 0xff, 0, 0, 0 }, { 0, 0, 0, 0xff },
		{ 0x01, 0x02, 0x03, 0x04 },
	};
	Buf A, B;

	for (unsigned e = 0; e < sizeof edge / sizeof edge[0]; e++) {
		A.fill();
		B.fill();
		memcpy(A.base(), edge[e], 4);
		memcpy(B.base(), edge[e], 4);
		u_long ra = P::ns_get32(A.base());
		u_long rb = ref_ns_get32(B.base());
		note(s, ra == rb && sameraw(A, B),
		    "edge %u: port=%lu ref=%lu buf@%ld", e, ra, rb,
		    firstdiff(A, B));
	}
	for (unsigned long it = 0; it < 200000; it++) {
		unsigned char v[4];
		for (int i = 0; i < 4; i++)
			v[i] = (unsigned char)rndn(256);
		A.fill();
		B.fill();
		memcpy(A.base(), v, 4);
		memcpy(B.base(), v, 4);
		u_long ra = P::ns_get32(A.base());
		u_long rb = ref_ns_get32(B.base());
		note(s, ra == rb && sameraw(A, B),
		    "rand %02x%02x%02x%02x: port=%lu ref=%lu buf@%ld",
		    v[0], v[1], v[2], v[3], ra, rb, firstdiff(A, B));
	}
}

static void
t_ns_put16(void)
{
	Stat &s = mkstat("ns_put16");
	static const u_int edge[] = {
		0u, 1u, 2u, 0x00ffu, 0x0100u, 0x7fffu, 0x8000u, 0xfffeu,
		0xffffu, 0x10000u, 0x1ffffu, 0xffff0001u, 0xdeadbeefu,
		0x00010002u,
	};
	Buf A, B;

	for (unsigned e = 0; e < sizeof edge / sizeof edge[0]; e++) {
		A.fill();
		B.fill();
		P::ns_put16(edge[e], A.base());
		ref_ns_put16(edge[e], B.base());
		note(s, sameraw(A, B), "edge %u (%#x): buf@%ld", e, edge[e],
		    firstdiff(A, B));
	}
	for (unsigned long it = 0; it < 200000; it++) {
		u_int v = (u_int)rnd32();
		if ((it & 3) == 0)
			v &= 0xffffu;
		A.fill();
		B.fill();
		P::ns_put16(v, A.base());
		ref_ns_put16(v, B.base());
		note(s, sameraw(A, B), "rand %#x: buf@%ld", v,
		    firstdiff(A, B));
	}
}

static void
t_ns_put32(void)
{
	Stat &s = mkstat("ns_put32");
	static const u_long edge[] = {
		0ul, 1ul, 2ul, 0xfful, 0x100ul, 0x7ffffffful, 0x80000000ul,
		0xfffffffeul, 0xfffffffful, 0x100000000ul, 0x123456789aul,
		0xfffffffffffffffful, 0xdeadbeefcafebabeul, 0x0001020304050607ul,
	};
	Buf A, B;

	for (unsigned e = 0; e < sizeof edge / sizeof edge[0]; e++) {
		A.fill();
		B.fill();
		P::ns_put32(edge[e], A.base());
		ref_ns_put32(edge[e], B.base());
		note(s, sameraw(A, B), "edge %u (%#lx): buf@%ld", e, edge[e],
		    firstdiff(A, B));
	}
	for (unsigned long it = 0; it < 200000; it++) {
		u_long v = (u_long)rnd64();
		if ((it & 3) == 0)
			v &= 0xfffffffful;
		A.fill();
		B.fill();
		P::ns_put32(v, A.base());
		ref_ns_put32(v, B.base());
		note(s, sameraw(A, B), "rand %#lx: buf@%ld", v,
		    firstdiff(A, B));
	}
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
	/* Exact boundaries around len + 1 == buflen for a few widths. */
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
			/* Bias heavily towards digits so units get reached. */
			if (rndn(3) != 0)
				buf[i] = (char)('0' + rndn(10));
			else
				buf[i] = alpha[rndn(sizeof alpha)];
		}
		buf[n] = '\0';
		one_parse_ttl(s, buf);
	}
	/* Digits + unit shaped strings, to hammer the switch fall-through. */
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
		/* "<label>." + suffix, i.e. deliberately well-formed. */
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
			/* Long names: exercise the ns_makecanon failure path. */
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
	/* Straddle the NS_MAXDNAME boundary of the internal buffers. */
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
t_ns_makecanon(void)
{
	Stat &s = mkstat("ns_makecanon");
	char buf[16];

	/* Exhaustive over {a . \} up to length 7, every interesting dstsize. */
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

/* -------------------------------------------------------- ns_parse.c */

static void
t_ns_msg_getflag(void)
{
	Stat &s = mkstat("ns_msg_getflag");
	ns_msg h;

	memset(&h, 0, sizeof h);
	for (unsigned long w = 0; w <= 0xffffu; w++) {
		h._flags = (uint16_t)w;
		for (int f = 0; f < 16; f++) {
			int ra = P::ns_msg_getflag(h, f);
			int rb = ref_ns_msg_getflag(h, f);
			note(s, ra == rb, "flags=%#lx flag=%d: %d/%d", w, f,
			    ra, rb);
		}
	}
}

static void
t_setsection(void)
{
	Stat &s = mkstat("setsection");
	Buf A, B;

	for (unsigned long it = 0; it < 200000; it++) {
		A.fill();
		B.fill();
		ns_msg ha, hb;
		prefill_msg(ha, A.base());
		prefill_msg(hb, B.base());
		/* Randomise the fields setsection reads and writes. */
		for (int i = 0; i < ns_s_max; i++) {
			long off = (long)rndn(64);
			bool nul = rndn(8) == 0;
			ha._sections[i] = nul ? NULL : A.base() + off;
			hb._sections[i] = nul ? NULL : B.base() + off;
		}
		ha._rrnum = hb._rrnum = (int)rnd32() % 1000;
		ns_sect prev = (ns_sect)rndn(5);
		ha._sect = hb._sect = prev;
		long mp = (long)rndn(64);
		ha._msg_ptr = A.base() + mp;
		hb._msg_ptr = B.base() + mp;

		ns_sect sect = (ns_sect)(it % 5);
		if ((it & 1) == 0)
			sect = (ns_sect)rndn(5);

		P::setsection(&ha, sect);
		ref_setsection(&hb, sect);

		Snap sa = snap(ha, A.base()), sb = snap(hb, B.base());
		note(s, snapeq(sa, sb) && sameraw(A, B),
		    "sect=%d prev=%d: field %d differs", (int)sect, (int)prev,
		    snapwhich(sa, sb));
	}
	/* Every section value, deterministically. */
	for (int i = 0; i <= ns_s_max; i++) {
		A.fill();
		B.fill();
		ns_msg ha, hb;
		prefill_msg(ha, A.base());
		prefill_msg(hb, B.base());
		for (int k = 0; k < ns_s_max; k++) {
			ha._sections[k] = A.base() + 3 * k;
			hb._sections[k] = B.base() + 3 * k;
		}
		P::setsection(&ha, (ns_sect)i);
		ref_setsection(&hb, (ns_sect)i);
		Snap sa = snap(ha, A.base()), sb = snap(hb, B.base());
		note(s, snapeq(sa, sb) && sameraw(A, B),
		    "edge sect=%d: field %d differs", i, snapwhich(sa, sb));
	}
}

/* --------------------------------------------------- message generator */

static void
gput16(unsigned char *b, int &o, unsigned v)
{
	b[o++] = (unsigned char)(v >> 8);
	b[o++] = (unsigned char)v;
}

static void
gput32(unsigned char *b, int &o, unsigned long v)
{
	b[o++] = (unsigned char)(v >> 24);
	b[o++] = (unsigned char)(v >> 16);
	b[o++] = (unsigned char)(v >> 8);
	b[o++] = (unsigned char)v;
}

enum { MSGCAP = 400 };

/*
 * Builds a mostly-well-formed DNS message, then optionally corrupts or
 * shaves it so that every bounds check in ns_skiprr()/ns_initparse()/
 * ns_parserr() is exercised from both sides of its boundary.
 */
static int
gen_msg(unsigned char *b)
{
	int o = 0;

	if (rndn(6) == 0) {
		/* Short garbage: header-boundary coverage (0..14 bytes). */
		int n = (int)rndn(15);
		for (int i = 0; i < n; i++)
			b[o++] = (unsigned char)rndn(256);
		return o;
	}

	gput16(b, o, rnd32() & 0xffffu);		/* id */
	gput16(b, o, rnd32() & 0xffffu);		/* flags */
	int counts[4];
	for (int i = 0; i < 4; i++)
		counts[i] = (int)rndn(3);
	for (int i = 0; i < 4; i++)
		gput16(b, o, (unsigned)counts[i]);

	int noff[24];
	int nn = 0;
	for (int sect = 0; sect < 4; sect++) {
		for (int k = 0; k < counts[sect]; k++) {
			if (o > MSGCAP - 96)
				break;
			int nameoff = o;
			unsigned kind = rndn(nn ? 4 : 2);
			if (kind == 0) {
				b[o++] = 0;
			} else if (kind == 1) {
				int nl = 1 + (int)rndn(3);
				for (int l = 0; l < nl; l++) {
					int ll = 1 + (int)rndn(4);
					b[o++] = (unsigned char)ll;
					for (int c = 0; c < ll; c++)
						b[o++] = (unsigned char)
						    ('a' + rndn(26));
				}
				b[o++] = 0;
			} else if (kind == 2) {
				int t = noff[rndn((unsigned)nn)];
				b[o++] = (unsigned char)(0xc0 |
				    ((t >> 8) & 0x3f));
				b[o++] = (unsigned char)(t & 0xff);
			} else {
				int ll = 1 + (int)rndn(4);
				b[o++] = (unsigned char)ll;
				for (int c = 0; c < ll; c++)
					b[o++] = (unsigned char)
					    ('a' + rndn(26));
				int t = noff[rndn((unsigned)nn)];
				b[o++] = (unsigned char)(0xc0 |
				    ((t >> 8) & 0x3f));
				b[o++] = (unsigned char)(t & 0xff);
			}
			if (nn < 24)
				noff[nn++] = nameoff;
			gput16(b, o, rnd32() & 0xffffu);	/* type */
			gput16(b, o, rnd32() & 0xffffu);	/* class */
			if (sect != 0) {
				gput32(b, o, rnd32());		/* ttl */
				int rdl = (int)rndn(4);		/* incl. 0 */
				gput16(b, o, (unsigned)rdl);
				for (int c = 0; c < rdl; c++)
					b[o++] = (unsigned char)rndn(256);
			}
		}
	}

	switch (rndn(8)) {
	case 0:
		if (o > 0)
			b[rndn((unsigned)o)] ^= (unsigned char)(1u << rndn(8));
		break;
	case 1:
		o -= 1 + (int)rndn(3);
		if (o < 0)
			o = 0;
		break;
	case 2: {
		int n = 1 + (int)rndn(3);
		for (int i = 0; i < n && o < MSGCAP; i++)
			b[o++] = (unsigned char)rndn(256);
		break;
	}
	case 3:
		if (o > 0)
			b[rndn((unsigned)o)] = (unsigned char)rndn(256);
		break;
	case 4:
		/* Perturb a count so the sections no longer add up. */
		if (o >= 12) {
			int i = 4 + 2 * (int)rndn(4);
			if (rndn(2))
				b[i + 1] = (unsigned char)(b[i + 1] + 1);
			else
				b[i + 1] = (unsigned char)(b[i + 1] - 1);
		}
		break;
	default:
		break;
	}
	return o;
}

static void
t_ns_skiprr(void)
{
	Stat &s = mkstat("ns_skiprr");
	Buf A, B;
	unsigned char msg[MSGCAP];

	/* Hand-written: empty range, count 0/negative, ptr == eom. */
	for (int len = 0; len <= 12; len++) {
		for (int cnt = -1; cnt <= 2; cnt++) {
			for (int sect = 0; sect < 4; sect++) {
				A.fill();
				B.fill();
				for (int i = 0; i < len; i++) {
					A.base()[i] = (unsigned char)(i * 7);
					B.base()[i] = (unsigned char)(i * 7);
				}
				errno = 0;
				int ra = P::ns_skiprr(A.base(),
				    A.base() + len, (ns_sect)sect, cnt);
				int ea = errno;
				errno = 0;
				int rb = ref_ns_skiprr(B.base(),
				    B.base() + len, (ns_sect)sect, cnt);
				int eb = errno;
				note(s, ra == rb && ea == eb && sameraw(A, B),
				    "edge len=%d cnt=%d sect=%d: r %d/%d "
				    "errno %d/%d", len, cnt, sect, ra, rb,
				    ea, eb);
			}
		}
	}

	for (unsigned long it = 0; it < 200000; it++) {
		int len;
		if ((it & 1) == 0) {
			len = gen_msg(msg);
		} else {
			len = (int)rndn(40);
			for (int i = 0; i < len; i++)
				msg[i] = (unsigned char)rndn(256);
		}
		A.fill();
		B.fill();
		memcpy(A.base(), msg, (size_t)len);
		memcpy(B.base(), msg, (size_t)len);

		int off;
		switch (rndn(4)) {
		case 0: off = 0; break;
		case 1: off = len < 12 ? len : 12; break;
		case 2: off = len; break;
		default: off = len ? (int)rndn((unsigned)len + 1) : 0; break;
		}
		int cnt;
		switch (rndn(5)) {
		case 0: cnt = 0; break;
		case 1: cnt = 1; break;
		case 2: cnt = -(int)rndn(4); break;
		case 3: cnt = (int)rndn(6); break;
		default: cnt = (int)rndn(300); break;
		}
		ns_sect sect = (ns_sect)rndn(4);

		errno = 0;
		int ra = P::ns_skiprr(A.base() + off, A.base() + len, sect,
		    cnt);
		int ea = errno;
		errno = 0;
		int rb = ref_ns_skiprr(B.base() + off, B.base() + len, sect,
		    cnt);
		int eb = errno;
		note(s, ra == rb && ea == eb && sameraw(A, B),
		    "len=%d off=%d cnt=%d sect=%d: r %d/%d errno %d/%d buf@%ld",
		    len, off, cnt, (int)sect, ra, rb, ea, eb, firstdiff(A, B));
	}
}

static void
t_ns_parse(void)
{
	Stat &sI = mkstat("ns_initparse");
	Stat &sR = mkstat("ns_parserr");
	Buf A, B;
	unsigned char msg[MSGCAP];
	unsigned long ok_init = 0;
	unsigned long ok_rr = 0;

	static const int sectv[] = { -1, 0, 1, 2, 3, 4, 5, 7 };

	for (unsigned long it = 0; it < 200000; it++) {
		int len;
		if ((it % 7) == 0) {
			len = (int)rndn(20);
			for (int i = 0; i < len; i++)
				msg[i] = (unsigned char)rndn(256);
		} else {
			len = gen_msg(msg);
		}
		A.fill();
		B.fill();
		memcpy(A.base(), msg, (size_t)len);
		memcpy(B.base(), msg, (size_t)len);

		ns_msg ha, hb;
		prefill_msg(ha, A.base());
		prefill_msg(hb, B.base());

		errno = 0;
		int ra = P::ns_initparse(A.base(), len, &ha);
		int ea = errno;
		errno = 0;
		int rb = ref_ns_initparse(B.base(), len, &hb);
		int eb = errno;

		Snap sa = snap(ha, A.base()), sb = snap(hb, B.base());
		note(sI, ra == rb && ea == eb && snapeq(sa, sb) &&
		    sameraw(A, B),
		    "len=%d: r %d/%d errno %d/%d field %d buf@%ld", len, ra,
		    rb, ea, eb, snapwhich(sa, sb), firstdiff(A, B));

		if (ra != 0 || rb != 0)
			continue;
		ok_init++;

		/* Drive each section's iterator to exhaustion (rrnum == -1). */
		for (int sect = 0; sect < ns_s_max; sect++) {
			int guard = (int)ha._counts[sect] + 3;
			for (int step = 0; step < guard; step++) {
				ns_rr rra, rrb;
				prefill_rr(rra, A.base());
				prefill_rr(rrb, B.base());
				errno = 0;
				int pa = P::ns_parserr(&ha, (ns_sect)sect, -1,
				    &rra);
				int ea2 = errno;
				errno = 0;
				int pb = ref_ns_parserr(&hb, (ns_sect)sect, -1,
				    &rrb);
				int eb2 = errno;
				Snap qa = snap(ha, A.base());
				Snap qb = snap(hb, B.base());
				note(sR, pa == pb && ea2 == eb2 &&
				    snapeq(qa, qb) &&
				    rreq(rra, A.base(), rrb, B.base()) &&
				    sameraw(A, B),
				    "iter len=%d sect=%d step=%d: r %d/%d "
				    "errno %d/%d field %d rr(%u/%u,%u/%u,"
				    "%lu/%lu,%u/%u)", len, sect, step, pa, pb,
				    ea2, eb2, snapwhich(qa, qb),
				    rra.type, rrb.type, rra.rr_class,
				    rrb.rr_class, (unsigned long)rra.ttl,
				    (unsigned long)rrb.ttl, rra.rdlength,
				    rrb.rdlength);
				if (pa != 0 || pb != 0)
					break;
				ok_rr++;
			}
		}

		/* Random walk: forward jumps, rewinds, bad sections. */
		unsigned steps = 3 + rndn(5);
		for (unsigned step = 0; step < steps; step++) {
			int sect;
			if (rndn(8) == 0)
				sect = sectv[rndn(sizeof sectv /
				    sizeof sectv[0])];
			else
				sect = (int)rndn(4);
			int rrnum;
			switch (rndn(5)) {
			case 0: rrnum = -1; break;
			case 1: rrnum = 0; break;
			case 2: rrnum = (int)rndn(4); break;
			case 3: rrnum = -2 - (int)rndn(3); break;
			default:
				rrnum = sect >= 0 && sect < ns_s_max
				    ? (int)ha._counts[sect] - 1
				    : (int)rndn(4);
				break;
			}
			ns_rr rra, rrb;
			prefill_rr(rra, A.base());
			prefill_rr(rrb, B.base());
			errno = 0;
			int pa = P::ns_parserr(&ha, (ns_sect)sect, rrnum, &rra);
			int ea2 = errno;
			errno = 0;
			int pb = ref_ns_parserr(&hb, (ns_sect)sect, rrnum,
			    &rrb);
			int eb2 = errno;
			Snap qa = snap(ha, A.base());
			Snap qb = snap(hb, B.base());
			note(sR, pa == pb && ea2 == eb2 && snapeq(qa, qb) &&
			    rreq(rra, A.base(), rrb, B.base()) &&
			    sameraw(A, B),
			    "walk len=%d sect=%d rrnum=%d: r %d/%d errno "
			    "%d/%d field %d rr(%u/%u,%u/%u,%lu/%lu,%u/%u)",
			    len, sect, rrnum, pa, pb, ea2, eb2,
			    snapwhich(qa, qb), rra.type, rrb.type,
			    rra.rr_class, rrb.rr_class,
			    (unsigned long)rra.ttl, (unsigned long)rrb.ttl,
			    rra.rdlength, rrb.rdlength);
			if (pa == 0)
				ok_rr++;
		}
	}
	printf("  (ns_initparse succeeded on %lu messages; %lu successful "
	    "ns_parserr calls)\n", ok_init, ok_rr);
}

/* ------------------------------------------------------------------ main */

int
main(void)
{
	rng_seed(0x0154C0FFEE123456ULL);

	printf("=== b0154 nameser differential test ===\n");

	t_ns_get16();
	t_ns_get32();
	t_ns_put16();
	t_ns_put32();
	t_fmt1();
	t_ns_format_ttl();
	t_ns_parse_ttl();
	t_ns_samedomain();
	t_ns_subdomain();
	t_ns_samename();
	t_ns_makecanon();
	t_ns_msg_getflag();
	t_setsection();
	t_ns_skiprr();
	t_ns_parse();

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
