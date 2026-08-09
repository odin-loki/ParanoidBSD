// Differential test for PBSD batch b0118s1 (fgets).

#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <csignal>
#include <unistd.h>

import pbsd.lib.libc.stdio.b0118s1;

namespace P = pbsd::lib_libc_stdio::b0118s1;

extern "C" {
char *ref_fgets(char *, int, pb_file_t *);
void pb_file_init(pb_file_t *);
void pb_file_input(pb_file_t *, const unsigned char *, size_t, size_t, int, int);
}

struct Stat {
	const char *name;
	long cases;
	long fails;
	int printed;
};

static Stat st_fgets = { "fgets", 0, 0, 0 };

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
	DIFF(in_pos);
	DIFF(refill_calls);
	if (memcmp(a->_buf, b->_buf, sizeof(a->_buf)) != 0) {
		snprintf(why, whysz, "FILE buffer contents differ");
		return (false);
	}
	return (true);
#undef DIFF
}

#define	FG_PAD	128
#define	FG_TAIL	256
#define	FG_MAXN	100

static pb_file_t fgA, fgB;
static char fgRawA[FG_PAD + FG_TAIL], fgRawB[FG_PAD + FG_TAIL];

static void
check_fgets(const unsigned char *data, size_t len, size_t chunk, int n,
    int fail_at, int prefill, short orient, short flags)
{
	char *bufA = fgRawA + FG_PAD;
	char *bufB = fgRawB + FG_PAD;
	char why[160], dump[80];
	char *ra, *rb;
	int ea, eb;
	long oa, ob;

	pb_file_init(&fgA);
	pb_file_init(&fgB);
	pb_file_input(&fgA, data, len, chunk, fail_at, prefill);
	pb_file_input(&fgB, data, len, chunk, fail_at, prefill);
	fgA._flags = flags;
	fgB._flags = flags;
	fgA._orientation = orient;
	fgB._orientation = orient;

	memset(fgRawA, 0x7f, sizeof(fgRawA));
	memset(fgRawB, 0x7f, sizeof(fgRawB));

	errno = 0;
	ra = ref_fgets(bufA, n, &fgA);
	ea = errno;
	errno = 0;
	rb = P::fgets(bufB, n, &fgB);
	eb = errno;

	st_fgets.cases++;
	oa = ra == nullptr ? -1 : ra - bufA;
	ob = rb == nullptr ? -1 : rb - bufB;
	hexdump(dump, sizeof(dump), data, len);
	if (oa != ob) {
		fail(st_fgets, "n=%d chunk=%zu data=%s: ret ref=%ld port=%ld",
		    n, chunk, dump, oa, ob);
		return;
	}
	if (ea != eb) {
		fail(st_fgets, "n=%d chunk=%zu data=%s: errno ref=%d port=%d",
		    n, chunk, dump, ea, eb);
		return;
	}
	if (memcmp(fgRawA, fgRawB, sizeof(fgRawA)) != 0) {
		size_t i;
		for (i = 0; i < sizeof(fgRawA); i++)
			if (fgRawA[i] != fgRawB[i])
				break;
		fail(st_fgets,
		    "n=%d chunk=%zu data=%s: buf[%zd] ref=%02x port=%02x",
		    n, chunk, dump, (ptrdiff_t)i - FG_PAD,
		    (unsigned char)fgRawA[i], (unsigned char)fgRawB[i]);
		return;
	}
	if (!file_eq(&fgA, &fgB, why, sizeof(why)))
		fail(st_fgets, "n=%d chunk=%zu data=%s: %s", n, chunk, dump,
		    why);
}

static const unsigned char fg_d0[] = { 0 };
static const unsigned char fg_d1[] = { '\n' };
static const unsigned char fg_d2[] = { 'a' };
static const unsigned char fg_d3[] = { 'a', '\n' };
static const unsigned char fg_d4[] = { '\0' };
static const unsigned char fg_d5[] = { 'a', 'b', '\n', 'c', 'd', '\n' };
static const unsigned char fg_d6[] = { 'a', 'b', 'c' };
static const unsigned char fg_d7[] = { 0x80, 0xff, '\n', 0x00, 0x7f, 0x81 };
static const unsigned char fg_d8[] = { '\n', '\n', '\n' };
static const unsigned char fg_d9[] = { 0x00, 0x00, 0x00, '\n' };
static unsigned char fg_d10[120];

struct DataRef {
	const unsigned char *p;
	size_t n;
};

static void
test_fgets_fixed(void)
{
	static const int ns[] = { -5, -1, 0, 1, 2, 3, 4, 6, 7, 8, 64, FG_MAXN };
	static const size_t chunks[] = { 1, 2, 3, 5, 8, 256 };
	static const int fails[] = { 0, 1, 2, 3 };
	static const short orients[] = { 0, 1, -1 };
	static const short flagset[] = { 0, 0x0020 /*__SEOF*/, 0x0040 /*__SERR*/ };
	DataRef ds[11];
	size_t i, ci, fi;
	int nidx, pf, oi, gi;

	memset(fg_d10, 'x', sizeof(fg_d10));
	fg_d10[50] = '\n';
	fg_d10[119] = '\n';

	ds[0] = { fg_d0, 0 };
	ds[1] = { fg_d1, sizeof(fg_d1) };
	ds[2] = { fg_d2, sizeof(fg_d2) };
	ds[3] = { fg_d3, sizeof(fg_d3) };
	ds[4] = { fg_d4, sizeof(fg_d4) };
	ds[5] = { fg_d5, sizeof(fg_d5) };
	ds[6] = { fg_d6, sizeof(fg_d6) };
	ds[7] = { fg_d7, sizeof(fg_d7) };
	ds[8] = { fg_d8, sizeof(fg_d8) };
	ds[9] = { fg_d9, sizeof(fg_d9) };
	ds[10] = { fg_d10, sizeof(fg_d10) };

	for (i = 0; i < 11; i++)
		for (ci = 0; ci < sizeof(chunks) / sizeof(chunks[0]); ci++)
			for (nidx = 0; nidx < (int)(sizeof(ns) / sizeof(ns[0]));
			    nidx++)
				for (fi = 0; fi < sizeof(fails) / sizeof(fails[0]);
				    fi++)
					for (pf = 0; pf < 2; pf++) {
						oi = (int)((i + ci + (size_t)nidx) % 3);
						gi = (int)((i + fi + (size_t)pf) % 3);
						check_fgets(ds[i].p, ds[i].n,
						    chunks[ci], ns[nidx],
						    fails[fi], pf, orients[oi],
						    flagset[gi]);
					}

	for (int n = 1; n <= 9; n++)
		for (size_t c = 1; c <= 8; c++) {
			check_fgets(fg_d5, sizeof(fg_d5), c, n, 0, 0, 0, 0);
			check_fgets(fg_d5, sizeof(fg_d5), c, n, 0, 1, 0, 0);
		}

	/* newline exactly at chunk boundary */
	{
		static const unsigned char bd[] = { 'a', 'b', '\n', 'c' };
		for (size_t c = 1; c <= 4; c++)
			for (int n = 1; n <= 6; n++)
				check_fgets(bd, sizeof(bd), c, n, 0, 0, 0, 0);
	}

	/* partial line at EOF with and without prior bytes */
	{
		static const unsigned char pl[] = { 'x', 'y', 'z' };
		for (int n = 1; n <= 6; n++)
			for (size_t c = 1; c <= 5; c++)
				check_fgets(pl, sizeof(pl), c, n, 0, 0, 0, 0);
	}

	/* refill error on first vs subsequent attempt */
	for (int fa = 1; fa <= 3; fa++)
		for (int n = 0; n <= 4; n++)
			check_fgets(fg_d5, sizeof(fg_d5), 2, n, fa, 0, 0, 0);
}

static void
test_fgets_random(void)
{
	static const unsigned char alpha[] = { '\n', '\0', 0x80, 0xff, 0x7f,
	    'a', 'b', '\n' };
	unsigned char data[70];
	long it;

	for (it = 0; it < 200000; it++) {
		size_t len = rndn(65);
		size_t chunk = 1 + rndn(70);
		int n = (int)rndn(FG_MAXN + 3) - 2;
		int fail_at = (int)rndn(5);
		int prefill = (int)rndn(2);
		short orient = (short)((int)rndn(3) - 1);
		static const short flagset[] = { 0, 0x0004, 0x0020, 0x0040,
		    0x0060 };
		short flags = flagset[rndn(5)];
		size_t i;
		int mode = (int)rndn(4);

		for (i = 0; i < len; i++)
			data[i] = mode == 0 ? (unsigned char)rnd()
					    : alpha[rndn(8)];
		if (fail_at > 3)
			fail_at = 0;
		check_fgets(data, len, chunk, n, fail_at, prefill, orient,
		    flags);
	}
}

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
	long total_fail;

	signal(SIGALRM, on_alarm);
	alarm(300);

	rngstate = 0x0123456789abcdefULL;
	test_fgets_fixed();
	test_fgets_random();

	printf("\n%-20s %12s %12s\n", "function", "cases", "failures");
	printf("%-20s %12s %12s\n", "--------------------", "------------",
	    "------------");
	printf("%-20s %12ld %12ld\n", st_fgets.name, st_fgets.cases,
	    st_fgets.fails);
	total_fail = st_fgets.fails;
	printf("%-20s %12s %12ld\n", "TOTAL", "", total_fail);
	return (total_fail == 0 ? 0 : 1);
}
