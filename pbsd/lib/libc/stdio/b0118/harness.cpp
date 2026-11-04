// Differential test for PBSD batch b0118.
//
// Every function of the batch is executed twice on identical inputs: once
// through the reference (ref_*, compiled from the unmodified C bodies in
// oracle.c) and once through the C++23 port.  Return values, errno, the whole
// output buffer including the guard bytes beyond the nominal write window and
// the whole mock FILE state (position, residue, flags, orientation, multibyte
// state, refill accounting, sink contents, fcntl call trace) are compared.

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

import pbsd.lib.libc.stdio.b0118;

namespace P = pbsd::lib_libc_stdio::b0118;

extern "C" {
char *ref_fgets(char *, int, pb_file_t *);
size_t ref_fwrite_unlocked(const void *, size_t, size_t, pb_file_t *);
size_t ref_fwrite(const void *, size_t, size_t, pb_file_t *);
wchar_t *ref_fgetws_l(wchar_t *, int, pb_file_t *, pb_locale_t);
wchar_t *ref_fgetws(wchar_t *, int, pb_file_t *);
pb_file_t *ref_fdopen(int, const char *);

void pb_file_init(pb_file_t *, unsigned char *, size_t, size_t);
void pb_file_input(pb_file_t *, const unsigned char *, size_t, size_t, int, int);
int pb_srefill(pb_file_t *);
void pb_fcntl_reset(void);
pb_locale_t pb_get_locale(void);

extern int pb_fcntl_getfl_ret;
extern int pb_fcntl_getfd_ret;
extern int pb_fcntl_setfd_ret;
extern int pb_fcntl_errno;
extern int pb_fcntl_log_n;
extern int pb_fcntl_log_fd[PB_FCNTL_LOG];
extern int pb_fcntl_log_cmd[PB_FCNTL_LOG];
extern int pb_fcntl_log_arg[PB_FCNTL_LOG];
extern int pb_sfp_fail;
extern pb_file_t *pb_sfp_last;
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

static Stat st_fgets = { "fgets", 0, 0, 0 };
static Stat st_fwrite_unlocked = { "fwrite_unlocked", 0, 0, 0 };
static Stat st_fwrite = { "fwrite", 0, 0, 0 };
static Stat st_fgetws_l = { "fgetws_l", 0, 0, 0 };
static Stat st_fgetws = { "fgetws", 0, 0, 0 };
static Stat st_fdopen = { "fdopen", 0, 0, 0 };

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

// Compares everything about the two mock FILEs that a caller could observe.
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
	DIFF(sfvwrite_calls);
	DIFF(sink_len);
	if (memcmp(a->_buf, b->_buf, sizeof(a->_buf)) != 0) {
		snprintf(why, whysz, "FILE buffer contents differ");
		return (false);
	}
	if (a->sink != nullptr && b->sink != nullptr &&
	    a->sink_size == b->sink_size &&
	    memcmp(a->sink, b->sink, a->sink_size) != 0) {
		size_t i;
		for (i = 0; i < a->sink_size; i++)
			if (a->sink[i] != b->sink[i])
				break;
		snprintf(why, whysz,
		    "sink[%zu]: ref=%02x port=%02x", i, a->sink[i], b->sink[i]);
		return (false);
	}
	return (true);
#undef DIFF
}

// ---------------------------------------------------------------- fgets

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

	pb_file_init(&fgA, nullptr, 0, 0);
	pb_file_init(&fgB, nullptr, 0, 0);
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

	/* exact boundary between "line fits" and "line does not fit" */
	for (int n = 1; n <= 9; n++)
		for (size_t c = 1; c <= 8; c++) {
			check_fgets(fg_d5, sizeof(fg_d5), c, n, 0, 0, 0, 0);
			check_fgets(fg_d5, sizeof(fg_d5), c, n, 0, 1, 0, 0);
		}
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

// --------------------------------------------------------------- fwrite

#define	FW_SINK	4096

static pb_file_t fwA, fwB;
static unsigned char fwSinkA[FW_SINK], fwSinkB[FW_SINK];
static unsigned char fwSrc[FW_SINK];

static void
check_fwrite(size_t size, size_t count, size_t cap, short orient, short flags,
    int locked)
{
	Stat &st = locked ? st_fwrite : st_fwrite_unlocked;
	char why[160];
	size_t ra, rb;
	int ea, eb;

	if (cap > FW_SINK)
		cap = FW_SINK;
	pb_file_init(&fwA, fwSinkA, FW_SINK, cap);
	pb_file_init(&fwB, fwSinkB, FW_SINK, cap);
	fwA._flags = flags;
	fwB._flags = flags;
	fwA._orientation = orient;
	fwB._orientation = orient;

	errno = 0;
	ra = locked ? ref_fwrite(fwSrc, size, count, &fwA)
		    : ref_fwrite_unlocked(fwSrc, size, count, &fwA);
	ea = errno;
	errno = 0;
	rb = locked ? P::fwrite(fwSrc, size, count, &fwB)
		    : P::fwrite_unlocked(fwSrc, size, count, &fwB);
	eb = errno;

	st.cases++;
	if (ra != rb) {
		fail(st, "size=%zu count=%zu cap=%zu: ret ref=%zu port=%zu",
		    size, count, cap, ra, rb);
		return;
	}
	if (ea != eb) {
		fail(st, "size=%zu count=%zu cap=%zu: errno ref=%d port=%d",
		    size, count, cap, ea, eb);
		return;
	}
	if (!file_eq(&fwA, &fwB, why, sizeof(why)))
		fail(st, "size=%zu count=%zu cap=%zu: %s", size, count, cap,
		    why);
}

static void
test_fwrite_fixed(void)
{
	static const size_t sizes[] = { 0, 1, 2, 3, 4, 7, 8, 16, 0xFFFF,
	    0x10000 };
	static const size_t counts[] = { 0, 1, 2, 3, 5, 12, 100, 4096, 0xFFFF,
	    0x10000 };
	size_t si, ci, cap;
	int locked;

	for (locked = 0; locked < 2; locked++) {
		for (si = 0; si < sizeof(sizes) / sizeof(sizes[0]); si++)
			for (ci = 0; ci < sizeof(counts) / sizeof(counts[0]);
			    ci++) {
				size_t n = sizes[si] * counts[ci];
				static const size_t caps[] = { 0, 1, 2, 3, 5,
				    11, 12, 13, 100, 4096 };
				size_t k;

				for (k = 0; k < sizeof(caps) / sizeof(caps[0]);
				    k++)
					check_fwrite(sizes[si], counts[ci],
					    caps[k], 0, 0, locked);
				/* caps exactly around the transfer size */
				if (n <= FW_SINK) {
					for (cap = (n > 2 ? n - 2 : 0);
					    cap <= n + 2 && cap <= FW_SINK;
					    cap++)
						check_fwrite(sizes[si],
						    counts[ci], cap, 0, 0,
						    locked);
				}
			}

		/* the SIZE_MAX overflow guard, on both sides of the edge */
		check_fwrite(0x10000, SIZE_MAX / 0x10000, 64, 0, 0, locked);
		check_fwrite(0x10000, SIZE_MAX / 0x10000 + 1, 64, 0, 0, locked);
		check_fwrite(SIZE_MAX / 0x10000, 0x10000, 64, 0, 0, locked);
		check_fwrite(SIZE_MAX / 0x10000 + 1, 0x10000, 64, 0, 0, locked);
		check_fwrite(SIZE_MAX, 1, 64, 0, 0, locked);
		check_fwrite(1, SIZE_MAX, 64, 0, 0, locked);
		check_fwrite(SIZE_MAX, 2, 64, 0, 0, locked);
		check_fwrite(2, SIZE_MAX, 64, 0, 0, locked);
		check_fwrite(SIZE_MAX / 2, 2, 64, 0, 0, locked);
		check_fwrite(2, SIZE_MAX / 2, 64, 0, 0, locked);
		check_fwrite(0xFFFF, 0xFFFF, 64, 0, 0, locked);
		check_fwrite(0x10000, 1, 64, 0, 0, locked);
		check_fwrite(1, 0x10000, 64, 0, 0, locked);
		check_fwrite(0x10000, 1, 4096, 0, 0, locked);
		check_fwrite(0, 0x10000, 64, 0, 0, locked);
		check_fwrite(0x10000, 0, 64, 0, 0, locked);

		/* orientation and pre-existing flags */
		for (int o = -1; o <= 1; o++)
			for (int f = 0; f < 3; f++) {
				static const short fl[] = { 0, 0x0008, 0x0040 };
				check_fwrite(0, 3, 8, (short)o, fl[f], locked);
				check_fwrite(3, 0, 8, (short)o, fl[f], locked);
				check_fwrite(3, 3, 8, (short)o, fl[f], locked);
				check_fwrite(3, 3, 9, (short)o, fl[f], locked);
				check_fwrite(3, 3, 64, (short)o, fl[f], locked);
			}
	}
}

static void
test_fwrite_random(void)
{
	static const size_t bigs[] = { 0xFFFF, 0x10000, 0x10001, SIZE_MAX,
	    SIZE_MAX / 2, SIZE_MAX / 0x10000, SIZE_MAX / 0x10000 + 1, 1, 0 };
	long it;

	for (it = 0; it < 200000; it++) {
		size_t size, count, cap;
		int locked = (int)rndn(2);
		short orient = (short)((int)rndn(3) - 1);
		short flags = (short)(rndn(2) ? 0 : 0x0040);

		if (rndn(8) == 0) {
			size = bigs[rndn(9)];
			count = bigs[rndn(9)];
			cap = rndn(FW_SINK + 1);
		} else {
			size = rndn(8);
			count = rndn(300);
			cap = rndn((uint32_t)(size * count) + 4);
		}
		check_fwrite(size, count, cap, orient, flags, locked);
	}
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

	pb_file_init(&wsA, nullptr, 0, 0);
	pb_file_init(&wsB, nullptr, 0, 0);
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
	ds[17] = { fg_d10, sizeof(fg_d10) };

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

// --------------------------------------------------------------- fdopen

static void
check_fdopen(int fd, const char *mode, int getfl, int getfd, int setfd,
    int sfpfail, int fcerr)
{
	int logA_fd[PB_FCNTL_LOG], logA_cmd[PB_FCNTL_LOG], logA_arg[PB_FCNTL_LOG];
	int nA, nB, i, ea, eb;
	pb_file_t *ra, *rb, *fpA, *fpB;
	char why[160];

	pb_fcntl_getfl_ret = getfl;
	pb_fcntl_getfd_ret = getfd;
	pb_fcntl_setfd_ret = setfd;
	pb_fcntl_errno = fcerr;
	pb_sfp_fail = sfpfail;

	pb_fcntl_reset();
	pb_sfp_last = nullptr;
	errno = 0;
	ra = ref_fdopen(fd, mode);
	ea = errno;
	fpA = pb_sfp_last;
	nA = pb_fcntl_log_n;
	for (i = 0; i < PB_FCNTL_LOG; i++) {
		logA_fd[i] = pb_fcntl_log_fd[i];
		logA_cmd[i] = pb_fcntl_log_cmd[i];
		logA_arg[i] = pb_fcntl_log_arg[i];
	}

	pb_fcntl_reset();
	pb_sfp_last = nullptr;
	errno = 0;
	rb = P::fdopen(fd, mode);
	eb = errno;
	fpB = pb_sfp_last;
	nB = pb_fcntl_log_n;

	st_fdopen.cases++;
	for (;;) {
		if ((ra == nullptr) != (rb == nullptr)) {
			fail(st_fdopen, "fd=%d mode=\"%s\" getfl=%#x: ret "
			    "ref=%s port=%s", fd, mode, getfl,
			    ra == nullptr ? "NULL" : "fp",
			    rb == nullptr ? "NULL" : "fp");
			break;
		}
		if (ea != eb) {
			fail(st_fdopen, "fd=%d mode=\"%s\" getfl=%#x: errno "
			    "ref=%d port=%d", fd, mode, getfl, ea, eb);
			break;
		}
		if (nA != nB) {
			fail(st_fdopen, "fd=%d mode=\"%s\" getfl=%#x: fcntl "
			    "calls ref=%d port=%d", fd, mode, getfl, nA, nB);
			break;
		}
		for (i = 0; i < nA && i < PB_FCNTL_LOG; i++) {
			if (logA_fd[i] != pb_fcntl_log_fd[i] ||
			    logA_cmd[i] != pb_fcntl_log_cmd[i] ||
			    logA_arg[i] != pb_fcntl_log_arg[i]) {
				fail(st_fdopen, "fd=%d mode=\"%s\" getfl=%#x: "
				    "fcntl[%d] ref=(%d,%d,%d) port=(%d,%d,%d)",
				    fd, mode, getfl, i, logA_fd[i],
				    logA_cmd[i], logA_arg[i],
				    pb_fcntl_log_fd[i], pb_fcntl_log_cmd[i],
				    pb_fcntl_log_arg[i]);
				goto done;
			}
		}
		if ((fpA == nullptr) != (fpB == nullptr)) {
			fail(st_fdopen, "fd=%d mode=\"%s\": __sfp ref=%s "
			    "port=%s", fd, mode, fpA ? "fp" : "NULL",
			    fpB ? "fp" : "NULL");
			break;
		}
		if (fpA != nullptr) {
			if ((ra == fpA) != (rb == fpB)) {
				fail(st_fdopen, "fd=%d mode=\"%s\": returned "
				    "pointer identity differs", fd, mode);
				break;
			}
			if ((fpA->_cookie == (void *)fpA) !=
			    (fpB->_cookie == (void *)fpB)) {
				fail(st_fdopen, "fd=%d mode=\"%s\": _cookie "
				    "differs", fd, mode);
				break;
			}
			if (fpA->_read != fpB->_read ||
			    fpA->_write != fpB->_write ||
			    fpA->_seek != fpB->_seek ||
			    fpA->_close != fpB->_close) {
				fail(st_fdopen, "fd=%d mode=\"%s\": I/O vector "
				    "differs", fd, mode);
				break;
			}
			fpA->_cookie = nullptr;
			fpB->_cookie = nullptr;
			if (!file_eq(fpA, fpB, why, sizeof(why))) {
				fail(st_fdopen, "fd=%d mode=\"%s\" getfl=%#x: "
				    "%s", fd, mode, getfl, why);
				break;
			}
		}
		break;
	}
done:
	free(fpA);
	free(fpB);
	pb_sfp_fail = 0;
}

static const char *fd_modes[] = {
	"", "r", "w", "a", "r+", "w+", "a+", "rb", "wb", "ab", "r+b", "re",
	"we", "ae", "r+e", "rx", "wx", "ax", "wxe", "q", "+", "b", "e",
	"rbe+x", "rt", "a+e", "rw", "R", "aE", "abe", "web", "reb", "a+be",
	"rex", "wex"
};
#define	NMODES	((int)(sizeof(fd_modes) / sizeof(fd_modes[0])))

static const int fd_getfl[] = {
	0, 1, 2, 3, 0x0008, 0x0009, 0x000a, 0x000b, 0x00040000, 0x00040001,
	0x00040002, 0x00040008, 0x8000, 0x0200 | 0x0400 | 1, -1, -22,
	0x00100000, 0x00100002
};
#define	NGETFL	((int)(sizeof(fd_getfl) / sizeof(fd_getfl[0])))

static const int fd_fds[] = { -5, -1, 0, 1, 2, 3, 7, 255, 32766, 32767,
	32768, 40000, 65535, 2147483647 };
#define	NFDS	((int)(sizeof(fd_fds) / sizeof(fd_fds[0])))

static const int fd_getfd[] = { 0, 1, 2, 5, -1 };
static const int fd_setfd[] = { 0, -1, 3 };

static void
test_fdopen_fixed(void)
{
	int i, m, g, gd, sd, sf;

	for (i = 0; i < NFDS; i++)
		for (m = 0; m < NMODES; m++)
			for (g = 0; g < NGETFL; g++)
				check_fdopen(fd_fds[i], fd_modes[m], fd_getfl[g],
				    0, 0, 0, EBADF);

	for (m = 0; m < NMODES; m++)
		for (g = 0; g < NGETFL; g++)
			for (gd = 0; gd < 5; gd++)
				for (sd = 0; sd < 3; sd++)
					for (sf = 0; sf < 2; sf++)
						check_fdopen(3, fd_modes[m],
						    fd_getfl[g], fd_getfd[gd],
						    fd_setfd[sd], sf, EBADF);
}

static void
test_fdopen_random(void)
{
	char mode[8];
	static const char mchars[] = "rwa+bxeqt";
	long it;

	for (it = 0; it < 200000; it++) {
		int fd, getfl, getfd, setfd, sfpfail;
		const char *m;
		size_t k, mlen;

		fd = (int)rndn(4) == 0 ? fd_fds[rndn(NFDS)]
				       : (int)rndn(70000) - 2;
		if (rndn(2)) {
			m = fd_modes[rndn((uint32_t)NMODES)];
		} else {
			mlen = rndn(6);
			for (k = 0; k < mlen; k++)
				mode[k] = mchars[rndn(9)];
			mode[mlen] = '\0';
			m = mode;
		}
		getfl = rndn(4) == 0 ? (int)rnd() : fd_getfl[rndn(NGETFL)];
		getfd = fd_getfd[rndn(5)];
		setfd = fd_setfd[rndn(3)];
		sfpfail = rndn(16) == 0 ? 1 : 0;
		check_fdopen(fd, m, getfl, getfd, setfd, sfpfail, EBADF);
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
	Stat *all[] = { &st_fgets, &st_fwrite_unlocked, &st_fwrite,
	    &st_fgetws_l, &st_fgetws, &st_fdopen };
	long total_fail = 0;
	size_t i;

	signal(SIGALRM, on_alarm);
	alarm(300);

	rngstate = 0x0123456789abcdefULL;
	test_fgets_fixed();
	test_fgets_random();

	rngstate = 0xfedcba9876543210ULL;
	test_fwrite_fixed();
	test_fwrite_random();

	rngstate = 0x243f6a8885a308d3ULL;
	test_fgetws_fixed();
	test_fgetws_random();

	rngstate = 0x13198a2e03707344ULL;
	test_fdopen_fixed();
	test_fdopen_random();

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
