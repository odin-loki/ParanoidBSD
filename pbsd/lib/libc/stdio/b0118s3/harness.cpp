// Differential test for PBSD batch b0118s3.
//
// Every function of the batch is executed twice on identical inputs: once
// through the reference (ref_*, compiled from the unmodified C bodies in
// oracle.c) and once through the C++23 port.  Return values, errno, the whole
// output buffer including the guard bytes beyond the nominal write window and
// the whole mock FILE state (position, residue, flags, orientation, multibyte
// state, refill accounting) are compared.

#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <climits>
#include <csignal>
#include <unistd.h>

import pbsd.lib.libc.stdio.b0118s3;

namespace P = pbsd::lib_libc_stdio::b0118s3;

extern "C" {
wchar_t *ref_fgetws_l(wchar_t *, int, pb_file_t *, pb_locale_t);
wchar_t *ref_fgetws(wchar_t *, int, pb_file_t *);

void pb_file_init(pb_file_t *);
void pb_file_input(pb_file_t *, const unsigned char *, size_t, size_t, int, int);
pb_locale_t pb_get_locale(void);

extern xlocale_ctype pb_utf8_locale;
extern xlocale_ctype pb_sb_locale;
extern pb_locale_t pb_global_locale;
}

// ---------------------------------------------------------------- stats

struct Stat {
	const char *name;
	long cases;
	long fails;
	int printed;
};

static Stat st_fgetws_l = { "fgetws_l", 0, 0, 0 };
static Stat st_fgetws = { "fgetws", 0, 0, 0 };

static void
fail(Stat &s, const char *fmt, ...)
{
	va_list ap;

	s.fails++;
	if (s.printed < 5) {
		s.printed++;
		fprintf(stderr, "FAIL %s: ", s.name);
		va_start(ap, fmt);
		vfprintf(stderr, fmt, ap);
		va_end(ap);
		fputc('\n', stderr);
	}
}

static void
hexdump(char *out, size_t outsz, const unsigned char *p, size_t n)
{
	size_t i, o = 0;

	if (n > 24)
		n = 24;
	for (i = 0; i < n && o + 3 < outsz; i++)
		o += (size_t)snprintf(out + o, outsz - o, "%02x", p[i]);
	out[o] = '\0';
}

// ---------------------------------------------------------------- rng

static uint64_t rngstate;

static inline uint32_t
rnd(void)
{
	rngstate ^= rngstate << 13;
	rngstate ^= rngstate >> 7;
	rngstate ^= rngstate << 17;
	return (uint32_t)(rngstate >> 32);
}

static inline uint32_t
rndn(uint32_t n)
{
	return rnd() % n;
}

// ------------------------------------------------------- state compare

static long
poff(const unsigned char *p, const unsigned char *base)
{
	return p == nullptr ? -1L : (long)(p - base);
}

static bool
file_eq(const pb_file_t *a, const pb_file_t *b, char *why, size_t whysz)
{
#define	DIFF(field)							\
	do {								\
		if (a->field != b->field) {				\
			snprintf(why, whysz, "%s: ref=%lld port=%lld",	\
			    #field, (long long)a->field,		\
			    (long long)b->field);			\
			return (false);					\
		}							\
	} while (0)

	if (poff(a->_p, a->_buf) != poff(b->_p, b->_buf)) {
		snprintf(why, whysz, "_p: ref=%ld port=%ld",
		    poff(a->_p, a->_buf), poff(b->_p, b->_buf));
		return (false);
	}
	DIFF(_r);
	DIFF(_w);
	DIFF(_flags);
	DIFF(_file);
	DIFF(_orientation);
	DIFF(_flags2);
	DIFF(_mbstate.__want);
	DIFF(_mbstate.__ch);
	DIFF(_mbstate.__lbound);
	DIFF(in_pos);
	DIFF(refill_calls);
	if (memcmp(a->_buf, b->_buf, sizeof(a->_buf)) != 0) {
		snprintf(why, whysz, "FILE buffer contents differ");
		return (false);
	}
	return (true);
#undef DIFF
}

// --------------------------------------------------------------- fgetws

#define	WS_PAD	64
#define	WS_TAIL	128
#define	WS_MAXN	40

static pb_file_t wsA, wsB;
static wchar_t wsRawA[WS_PAD + WS_TAIL], wsRawB[WS_PAD + WS_TAIL];

static void
check_fgetws(const unsigned char *data, size_t len, size_t chunk, int n,
    int fail_at, int prefill, short orient, short flags, int locsel,
    int use_l, int globsel)
{
	Stat &st = use_l ? st_fgetws_l : st_fgetws;
	wchar_t *bufA = wsRawA + WS_PAD;
	wchar_t *bufB = wsRawB + WS_PAD;
	char why[160], dump[80];
	wchar_t *ra, *rb;
	pb_locale_t loc;
	int ea, eb;
	long oa, ob;

	pb_global_locale = globsel ? &pb_sb_locale : &pb_utf8_locale;
	switch (locsel) {
	case 0:
		loc = &pb_utf8_locale;
		break;
	case 1:
		loc = &pb_sb_locale;
		break;
	default:
		loc = nullptr;
		break;
	}

	pb_file_init(&wsA);
	pb_file_init(&wsB);
	pb_file_input(&wsA, data, len, chunk, fail_at, prefill);
	pb_file_input(&wsB, data, len, chunk, fail_at, prefill);
	wsA._flags = flags;
	wsB._flags = flags;
	wsA._orientation = orient;
	wsB._orientation = orient;

	memset(wsRawA, 0x7f, sizeof(wsRawA));
	memset(wsRawB, 0x7f, sizeof(wsRawB));

	errno = 0;
	ra = use_l ? ref_fgetws_l(bufA, n, &wsA, loc)
		   : ref_fgetws(bufA, n, &wsA);
	ea = errno;
	errno = 0;
	rb = use_l ? P::fgetws_l(bufB, n, &wsB, loc)
		   : P::fgetws(bufB, n, &wsB);
	eb = errno;

	st.cases++;
	oa = ra == nullptr ? -1 : ra - bufA;
	ob = rb == nullptr ? -1 : rb - bufB;
	hexdump(dump, sizeof(dump), data, len);
	if (oa != ob) {
		fail(st, "n=%d chunk=%zu loc=%d data=%s: ret ref=%ld port=%ld",
		    n, chunk, locsel, dump, oa, ob);
		return;
	}
	if (ea != eb) {
		fail(st, "n=%d chunk=%zu loc=%d data=%s: errno ref=%d port=%d",
		    n, chunk, locsel, dump, ea, eb);
		return;
	}
	if (memcmp(wsRawA, wsRawB, sizeof(wsRawA)) != 0) {
		size_t i;
		for (i = 0; i < WS_PAD + WS_TAIL; i++)
			if (wsRawA[i] != wsRawB[i])
				break;
		fail(st, "n=%d chunk=%zu loc=%d data=%s: ws[%zd] ref=%08lx "
		    "port=%08lx", n, chunk, locsel, dump,
		    (ptrdiff_t)i - WS_PAD, (unsigned long)wsRawA[i],
		    (unsigned long)wsRawB[i]);
		return;
	}
	if (!file_eq(&wsA, &wsB, why, sizeof(why)))
		fail(st, "n=%d chunk=%zu loc=%d data=%s: %s", n, chunk, locsel,
		    dump, why);
}

/* byte sequences covering every branch of the mock UTF-8 decoder */
static const unsigned char ws_seq[][5] = {
	{ 1, 'a' },
	{ 1, '\n' },
	{ 1, '\0' },
	{ 1, 0x7f },
	{ 1, 0x80 },			/* stray continuation: EILSEQ */
	{ 1, 0xff },			/* invalid lead: EILSEQ */
	{ 2, 0xc3, 0xa9 },		/* two byte */
	{ 1, 0xc3 },			/* truncated two byte */
	{ 3, 0xe2, 0x82, 0xac },	/* three byte */
	{ 2, 0xe2, 0x82 },		/* truncated three byte */
	{ 4, 0xf0, 0x9f, 0x98, 0x80 },	/* four byte */
	{ 3, 0xf0, 0x9f, 0x98 },	/* truncated four byte */
	{ 2, 0xc0, 0x80 },		/* overlong: EILSEQ */
	{ 2, 0xc3, 'a' },		/* bad continuation: EILSEQ */
};

struct DataRef {
	const unsigned char *p;
	size_t n;
};

static void
test_fgetws_fixed(void)
{
	static const unsigned char d0[] = { 0 };
	static const unsigned char d1[] = { '\n' };
	static const unsigned char d2[] = { 'a', '\n' };
	static const unsigned char d3[] = { 'a', 'b', 'c' };
	static const unsigned char d4[] = { '\0' };
	static const unsigned char d5[] = { 'a', '\0', 'b', '\n' };
	static const unsigned char d6[] = { 0xc3, 0xa9 };
	static const unsigned char d7[] = { 0xc3 };
	static const unsigned char d8[] = { 0xc3, 0xa9, '\n' };
	static const unsigned char d9[] = { 0xe2, 0x82, 0xac, '\n' };
	static const unsigned char d10[] = { 0xf0, 0x9f, 0x98, 0x80 };
	static const unsigned char d11[] = { 0x80 };
	static const unsigned char d12[] = { 0xc0, 0x80 };
	static const unsigned char d13[] = { 'a', 0xc3, 0xa9, '\n', 'b' };
	static const unsigned char d14[] = { '\0', '\0', '\n' };
	static const unsigned char d15[] = { 'x', 'y', 'z', '\n', '\0', 0xff };
	static unsigned char d16[45];
	DataRef ds[18];
	static const int ns[] = { -3, -1, 0, 1, 2, 3, 4, 5, 8, WS_MAXN };
	static const size_t chunks[] = { 1, 2, 3, 4, 7, 256 };
	static const int fails[] = { 0, 1, 2 };
	size_t i, ci;
	int ni, fi, pf, ls;

	memset(d16, 'q', sizeof(d16));
	d16[44] = '\n';

	ds[0] = { d0, 0 };
	ds[1] = { d1, sizeof(d1) };
	ds[2] = { d2, sizeof(d2) };
	ds[3] = { d3, sizeof(d3) };
	ds[4] = { d4, sizeof(d4) };
	ds[5] = { d5, sizeof(d5) };
	ds[6] = { d6, sizeof(d6) };
	ds[7] = { d7, sizeof(d7) };
	ds[8] = { d8, sizeof(d8) };
	ds[9] = { d9, sizeof(d9) };
	ds[10] = { d10, sizeof(d10) };
	ds[11] = { d11, sizeof(d11) };
	ds[12] = { d12, sizeof(d12) };
	ds[13] = { d13, sizeof(d13) };
	ds[14] = { d14, sizeof(d14) };
	ds[15] = { d15, sizeof(d15) };
	ds[16] = { d16, sizeof(d16) };
	ds[17] = { d15, sizeof(d15) };

	for (i = 0; i < 18; i++)
		for (ci = 0; ci < sizeof(chunks) / sizeof(chunks[0]); ci++)
			for (ni = 0; ni < (int)(sizeof(ns) / sizeof(ns[0])); ni++)
				for (fi = 0;
				    fi < (int)(sizeof(fails) / sizeof(fails[0]));
				    fi++)
					for (pf = 0; pf < 2; pf++)
						for (ls = 0; ls < 3; ls++) {
							short o = (short)((int)((i + (size_t)ni) % 3) - 1);
							short fl = (short)(((i + (size_t)fi) % 3) == 0 ? 0 : (((i + (size_t)fi) % 3) == 1 ? 0x0020 : 0x0040));
							check_fgetws(ds[i].p,
							    ds[i].n, chunks[ci],
							    ns[ni], fails[fi],
							    pf, o, fl, ls,
							    (int)((i + (size_t)ni) & 1),
							    (int)((i + (size_t)ci) & 1));
						}

	/* exact boundary between "line fits" and "line does not fit" */
	for (int n = 1; n <= 9; n++)
		for (size_t c = 1; c <= 8; c++) {
			check_fgetws(d5, sizeof(d5), c, n, 0, 0, 0, 0, 0, 0, 0);
			check_fgetws(d5, sizeof(d5), c, n, 0, 1, 0, 0, 0, 0, 0);
			check_fgetws(d8, sizeof(d8), c, n, 0, 0, 0, 0, 0, 1, 0);
			check_fgetws(d8, sizeof(d8), c, n, 0, 1, 0, 0, 0, 1, 0);
		}
}

static void
test_fgetws_random(void)
{
	unsigned char data[48];
	long it;

	for (it = 0; it < 200000; it++) {
		size_t len = 0;
		size_t chunk = 1 + rndn(20);
		int n = (int)rndn(WS_MAXN + 3) - 2;
		int fail_at = (int)rndn(4);
		int prefill = (int)rndn(2);
		short orient = (short)((int)rndn(3) - 1);
		static const short flagset[] = { 0, 0x0020, 0x0040 };
		short flags = flagset[rndn(3)];
		int locsel = (int)rndn(3);
		int use_l = (int)rndn(2);
		int globsel = (int)rndn(2);
		size_t target = rndn(41);

		while (len < target) {
			const unsigned char *sq = ws_seq[rndn(14)];
			size_t k = sq[0];

			if (len + k > sizeof(data))
				break;
			for (size_t j = 0; j < k; j++)
				data[len++] = sq[1 + j];
		}
		if (fail_at > 2)
			fail_at = 0;
		check_fgetws(data, len, chunk, n, fail_at, prefill, orient,
		    flags, locsel, use_l, globsel);
	}
}

// ----------------------------------------------------------------- main

static void
on_alarm(int)
{
	const char msg[] = "TIMEOUT: a call failed to terminate\n";

	(void)!write(2, msg, sizeof(msg) - 1);
	_exit(1);
}

int
main(void)
{
	Stat *all[] = { &st_fgetws_l, &st_fgetws };
	long total_fail = 0;
	size_t i;

	signal(SIGALRM, on_alarm);
	alarm(300);

	rngstate = 0x243f6a8885a308d3ULL;
	test_fgetws_fixed();
	test_fgetws_random();

	printf("\n%-20s %12s %12s\n", "function", "cases", "failures");
	printf("%-20s %12s %12s\n", "--------------------", "------------",
	    "------------");
	for (i = 0; i < sizeof(all) / sizeof(all[0]); i++) {
		printf("%-20s %12ld %12ld\n", all[i]->name, all[i]->cases,
		    all[i]->fails);
		total_fail += all[i]->fails;
	}
	printf("%-20s %12s %12ld\n", "TOTAL", "", total_fail);
	return (total_fail == 0 ? 0 : 1);
}
