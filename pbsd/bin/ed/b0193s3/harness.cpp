/* harness.cpp: differential test for PBSD batch b0193s3.
 *
 * Every function of the batch is driven through hand-written edge cases and a
 * fixed-seed randomised sweep.  Both the C++23 port and the ref_ oracle are
 * called on identical inputs and identical global state; return values,
 * mutated globals and whole buffers (including the bytes past the nominal
 * write window) are compared byte for byte.
 */

#include <sys/types.h>

#include <climits>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

import pbsd.bin.ed.b0193s3;

namespace P = pbsd::bin_ed::b0193s3;

/* ------------------------------------------------------------------ */
/* Oracle interface                                                     */
/* ------------------------------------------------------------------ */

struct ref_line {
	struct ref_line *q_forw;
	struct ref_line *q_back;
	off_t seek;
	int len;
};
typedef struct ref_line ref_line_t;

extern "C" {
extern const char *errmsg;
extern int mutex;
extern int red;
extern long current_addr;
extern long addr_last;
extern long first_addr;
extern long second_addr;
extern long addr_cnt;
extern ref_line_t *mark[26];
extern int markno;

int ref_check_addr_range(long n, long m);
int ref_mark_line_node(ref_line_t *lp, int n);
void ref_unmark_line_node(ref_line_t *lp);
ref_line_t *ref_dup_line_node(ref_line_t *lp);
int ref_has_trailing_escape(char *s, char *t);
char *ref_strip_escapes(char *s);
int ref_is_legal_filename(char *s);
}

/* filesz that strip_escapes() settles on: max(PATH_MAX, MINBUFSZ), taken
   from the port so that the harness cannot drift away from the code. */
static const int FILESZ = (P::kPathMax > P::kMinBufSz) ? P::kPathMax
    : P::kMinBufSz;

static const char SENT[] = "<no-errmsg-set>";

/* ------------------------------------------------------------------ */
/* Bookkeeping                                                          */
/* ------------------------------------------------------------------ */

struct Stat {
	const char *name;
	long cases;
	long fails;
	long shown;
};

static Stat st_car   = { "check_addr_range",    0, 0, 0 };
static Stat st_mark  = { "mark_line_node",      0, 0, 0 };
static Stat st_unmk  = { "unmark_line_node",    0, 0, 0 };
static Stat st_dup   = { "dup_line_node",       0, 0, 0 };
static Stat st_hte   = { "has_trailing_escape", 0, 0, 0 };
static Stat st_strip = { "strip_escapes",       0, 0, 0 };
static Stat st_ilf   = { "is_legal_filename",   0, 0, 0 };

static void
fail(Stat &s, const char *fmt, ...) __attribute__((format(printf, 2, 3)));

static void
fail(Stat &s, const char *fmt, ...)
{
	va_list ap;

	s.fails++;
	if (s.shown < 12) {
		s.shown++;
		fprintf(stderr, "  [%s] MISMATCH: ", s.name);
		va_start(ap, fmt);
		vfprintf(stderr, fmt, ap);
		va_end(ap);
		fputc('\n', stderr);
	}
}

/* ------------------------------------------------------------------ */
/* Fixed-seed PRNG (xorshift64*)                                        */
/* ------------------------------------------------------------------ */

static uint64_t rng_state = 0x9e3779b97f4a7c15ULL;

static void
rng_seed(uint64_t s)
{
	rng_state = s ? s : 0x123456789abcdefULL;
}

static inline uint64_t
rnd(void)
{
	uint64_t x = rng_state;

	x ^= x << 13;
	x ^= x >> 7;
	x ^= x << 17;
	rng_state = x;
	return x * 0x2545f4914f6cdd1dULL;
}

static inline long
rnd_range(long lo, long hi)
{
	return lo + (long)(rnd() % (uint64_t)(hi - lo + 1));
}

/* ------------------------------------------------------------------ */
/* check_addr_range                                                     */
/* ------------------------------------------------------------------ */

static void
car_case(long ac, long fa, long sa, long al, long n, long m)
{
	int r1, r2;
	long f1, s1, f2, s2;
	const char *e1;
	const char *e2;

	st_car.cases++;

	errmsg = SENT;
	addr_cnt = ac;
	first_addr = fa;
	second_addr = sa;
	addr_last = al;
	r1 = ref_check_addr_range(n, m);
	f1 = first_addr;
	s1 = second_addr;
	e1 = errmsg;

	P::errmsg = SENT;
	P::addr_cnt = ac;
	P::first_addr = fa;
	P::second_addr = sa;
	P::addr_last = al;
	r2 = P::check_addr_range(n, m);
	f2 = P::first_addr;
	s2 = P::second_addr;
	e2 = P::errmsg;

	if (r1 != r2 || f1 != f2 || s1 != s2 || strcmp(e1, e2) != 0)
		fail(st_car,
		    "ac=%ld fa=%ld sa=%ld al=%ld n=%ld m=%ld -> "
		    "ref(r=%d fa=%ld sa=%ld \"%s\") port(r=%d fa=%ld sa=%ld \"%s\")",
		    ac, fa, sa, al, n, m, r1, f1, s1, e1, r2, f2, s2, e2);
}

static void
test_check_addr_range(void)
{
	static const long vals[] = {
		LONG_MIN, LONG_MIN + 1, -1000000L, -3, -2, -1, 0, 1, 2, 3,
		5, 1000000L, LONG_MAX - 1, LONG_MAX
	};
	static const long acs[] = { -2, -1, 0, 1, 2, 7 };
	const int nv = (int)(sizeof(vals) / sizeof(vals[0]));
	const int na = (int)(sizeof(acs) / sizeof(acs[0]));
	int a, i, j, k, l, m;

	/* Exhaustive cross product over the interesting small values plus the
	   extremes; every comparison in the function sits on a boundary here. */
	static const long small[] = { -2, -1, 0, 1, 2, 3 };
	const int ns = (int)(sizeof(small) / sizeof(small[0]));

	for (a = 0; a < na; a++)
		for (i = 0; i < ns; i++)
			for (j = 0; j < ns; j++)
				for (k = 0; k < ns; k++)
					for (l = 0; l < ns; l++)
						for (m = 0; m < ns; m++)
							car_case(acs[a],
							    small[i], small[j],
							    small[k], small[l],
							    small[m]);

	/* Extremes: one axis at a time, so no arithmetic can overflow. */
	for (a = 0; a < na; a++)
		for (i = 0; i < nv; i++)
			for (j = 0; j < nv; j++) {
				car_case(acs[a], vals[i], vals[j], 1, 1, 2);
				car_case(acs[a], 1, 2, vals[i], vals[j], 3);
				car_case(acs[a], 0, 0, vals[i], vals[j], vals[i]);
				car_case(acs[a], vals[i], vals[j], vals[j],
				    vals[i], vals[j]);
			}

	/* Randomised sweep. */
	rng_seed(0xC0FFEE01ULL);
	for (long it = 0; it < 240000; it++) {
		long v[5];

		for (int q = 0; q < 5; q++) {
			if ((rnd() & 7) == 0)
				v[q] = vals[rnd() % nv];
			else
				v[q] = rnd_range(-4, 12);
		}
		car_case(((rnd() & 3) == 0) ? rnd_range(-2, 4) : 0,
		    v[0], v[1], v[2], v[3], v[4]);
	}
}

/* ------------------------------------------------------------------ */
/* mark_line_node / unmark_line_node                                    */
/* ------------------------------------------------------------------ */

#define NPOOL 8
#define NULLIDX NPOOL		/* pool index that stands for NULL */

static ref_line_t rpool[NPOOL];
static P::line_t ppool[NPOOL];

static ref_line_t *
rp(int i)
{
	return (i >= NPOOL) ? NULL : &rpool[i];
}

static P::line_t *
pp(int i)
{
	return (i >= NPOOL) ? NULL : &ppool[i];
}

static int
ridx(ref_line_t *p)
{
	if (p == NULL)
		return NULLIDX;
	for (int i = 0; i < NPOOL; i++)
		if (p == &rpool[i])
			return i;
	return 99;
}

static int
pidx(P::line_t *p)
{
	if (p == NULL)
		return NULLIDX;
	for (int i = 0; i < NPOOL; i++)
		if (p == &ppool[i])
			return i;
	return 99;
}

static void
set_marks(const int *state, int mno)
{
	for (int i = 0; i < 26; i++) {
		mark[i] = rp(state[i]);
		P::mark[i] = pp(state[i]);
	}
	markno = mno;
	P::markno = mno;
	errmsg = SENT;
	P::errmsg = SENT;
}

static int
cmp_marks(int *ro, int *po)
{
	int bad = 0;

	for (int i = 0; i < 26; i++) {
		ro[i] = ridx(mark[i]);
		po[i] = pidx(P::mark[i]);
		if (ro[i] != po[i])
			bad = 1;
	}
	if (markno != P::markno)
		bad = 1;
	return bad;
}

static void
marks_to_str(const int *a, char *out)
{
	for (int i = 0; i < 26; i++)
		out[i] = (char)('0' + (a[i] > 9 ? 9 : a[i]));
	out[26] = '\0';
}

static void
mark_case(const int *state, int mno, int lpi, int n)
{
	int r1, r2;
	int ro[26], po[26];
	char sb[32], sb2[32];

	st_mark.cases++;
	set_marks(state, mno);

	r1 = ref_mark_line_node(rp(lpi), n);
	{
		/* snapshot the oracle side before touching the port side */
		int rsnap[26], rmno;
		const char *e1 = errmsg;

		for (int i = 0; i < 26; i++)
			rsnap[i] = ridx(mark[i]);
		rmno = markno;

		r2 = P::mark_line_node(pp(lpi), n);

		for (int i = 0; i < 26; i++) {
			ro[i] = rsnap[i];
			po[i] = pidx(P::mark[i]);
		}
		if (r1 != r2 || rmno != P::markno ||
		    memcmp(ro, po, sizeof(ro)) != 0 ||
		    strcmp(e1, P::errmsg) != 0) {
			marks_to_str(ro, sb);
			marks_to_str(po, sb2);
			fail(st_mark,
			    "mno=%d lp=%d n=%d -> ref(r=%d mno=%d %s \"%s\") "
			    "port(r=%d mno=%d %s \"%s\")",
			    mno, lpi, n, r1, rmno, sb, e1,
			    r2, P::markno, sb2, P::errmsg);
		}
	}
}

static void
unmark_case(const int *state, int mno, int lpi)
{
	int ro[26], po[26];
	int rsnap[26], rmno;
	char sb[32], sb2[32];

	st_unmk.cases++;
	set_marks(state, mno);

	ref_unmark_line_node(rp(lpi));
	for (int i = 0; i < 26; i++)
		rsnap[i] = ridx(mark[i]);
	rmno = markno;

	P::unmark_line_node(pp(lpi));
	for (int i = 0; i < 26; i++) {
		ro[i] = rsnap[i];
		po[i] = pidx(P::mark[i]);
	}
	if (rmno != P::markno || memcmp(ro, po, sizeof(ro)) != 0) {
		marks_to_str(ro, sb);
		marks_to_str(po, sb2);
		fail(st_unmk, "mno=%d lp=%d -> ref(mno=%d %s) port(mno=%d %s)",
		    mno, lpi, rmno, sb, P::markno, sb2);
	}
}

/* n values that stay inside the range for which the original is defined.
   (n >= 256 or n <= -129 makes islower((unsigned char)n) true for an index
   n - 'a' that is far outside mark[], i.e. the original reads/writes out of
   bounds; those inputs are undefined for the C source itself.) */
static const int mark_ns[] = {
	-128, -127, -100, -57, -31, -1, 0, 1, 9, 32, 47, 48, 57, 64,
	'A', 'Z', '[', '`', 'a', 'b', 'c', 'm', 'y', 'z', '{', '|',
	127, 128, 129, 160, 200, 0xdf, 0xe0, 0xfe, 0xff
};

static void
test_marks(void)
{
	int state[26];
	const int nn = (int)(sizeof(mark_ns) / sizeof(mark_ns[0]));

	/* --- hand-written mark_line_node cases --- */

	/* all empty */
	for (int i = 0; i < 26; i++)
		state[i] = NULLIDX;
	for (int k = 0; k < nn; k++)
		for (int lp = 0; lp <= NPOOL; lp++)
			for (int mno = -1; mno <= 2; mno++)
				mark_case(state, mno, lp, mark_ns[k]);

	/* all occupied */
	for (int i = 0; i < 26; i++)
		state[i] = i % NPOOL;
	for (int k = 0; k < nn; k++)
		for (int lp = 0; lp <= NPOOL; lp++)
			for (int mno = 25; mno <= 27; mno++)
				mark_case(state, mno, lp, mark_ns[k]);

	/* exactly one slot occupied, walked across the whole array: this puts
	   the `mark[n - 'a'] == NULL' test on both sides of its boundary for
	   every legal index. */
	for (int slot = 0; slot < 26; slot++) {
		for (int i = 0; i < 26; i++)
			state[i] = NULLIDX;
		state[slot] = 3;
		for (int k = 0; k < nn; k++)
			for (int lp = 0; lp <= NPOOL; lp += NPOOL)
				mark_case(state, 1, lp, mark_ns[k]);
		for (int c = 'a'; c <= 'z'; c++)
			mark_case(state, 1, 5, c);
	}

	/* --- hand-written unmark_line_node cases --- */

	for (int i = 0; i < 26; i++)
		state[i] = NULLIDX;
	for (int lp = 0; lp <= NPOOL; lp++)
		for (int mno = -2; mno <= 3; mno++)
			unmark_case(state, mno, lp);

	/* single occupant at each slot, with consistent and inconsistent
	   markno values (markno==0 while marks exist, markno==1 while two
	   matching marks exist, ... all of which separate `markno &&' from
	   `markno ||' and `markno--' from `markno++'). */
	for (int slot = 0; slot < 26; slot++) {
		for (int i = 0; i < 26; i++)
			state[i] = NULLIDX;
		state[slot] = 2;
		for (int mno = -1; mno <= 2; mno++) {
			unmark_case(state, mno, 2);
			unmark_case(state, mno, 3);
			unmark_case(state, mno, NULLIDX);
		}
	}

	/* two identical occupants, deliberately understated markno */
	for (int a = 0; a < 26; a++)
		for (int b = a + 1; b < 26; b++) {
			for (int i = 0; i < 26; i++)
				state[i] = NULLIDX;
			state[a] = 1;
			state[b] = 1;
			unmark_case(state, 1, 1);
			unmark_case(state, 2, 1);
			unmark_case(state, 0, 1);
		}

	/* every slot holds the same node */
	for (int i = 0; i < 26; i++)
		state[i] = 4;
	for (int mno = -1; mno <= 27; mno++) {
		unmark_case(state, mno, 4);
		unmark_case(state, mno, 5);
		unmark_case(state, mno, NULLIDX);
	}

	/* --- randomised sweeps --- */

	rng_seed(0xBEEF0002ULL);
	for (long it = 0; it < 200000; it++) {
		int mno;

		for (int i = 0; i < 26; i++)
			state[i] = ((rnd() & 1) ? NULLIDX
			    : (int)rnd_range(0, NPOOL - 1));
		mno = (int)rnd_range(-2, 28);
		mark_case(state, mno, (int)rnd_range(0, NPOOL),
		    mark_ns[rnd() % nn]);
	}

	rng_seed(0xBEEF0003ULL);
	for (long it = 0; it < 200000; it++) {
		int mno;
		int dense = (int)(rnd() % 3);

		for (int i = 0; i < 26; i++) {
			if (dense == 0)
				state[i] = ((rnd() & 3) == 0)
				    ? (int)rnd_range(0, NPOOL - 1) : NULLIDX;
			else if (dense == 1)
				state[i] = ((rnd() & 1) ? NULLIDX
				    : (int)rnd_range(0, NPOOL - 1));
			else
				state[i] = (int)rnd_range(0, 2);
		}
		mno = (int)rnd_range(-2, 28);
		unmark_case(state, mno, (int)rnd_range(0, NPOOL));
	}
}

/* ------------------------------------------------------------------ */
/* dup_line_node                                                        */
/* ------------------------------------------------------------------ */

static void
dup_case(long long seek, int len)
{
	ref_line_t src;
	P::line_t psrc;
	ref_line_t *r;
	P::line_t *p;

	st_dup.cases++;

	memset(&src, 0x7f, sizeof(src));
	memset(&psrc, 0x7f, sizeof(psrc));
	src.seek = (off_t)seek;
	src.len = len;
	psrc.seek = (off_t)seek;
	psrc.len = len;

	errmsg = SENT;
	P::errmsg = SENT;

	r = ref_dup_line_node(&src);
	p = P::dup_line_node(&psrc);

	if (r == NULL || p == NULL) {
		fail(st_dup, "seek=%lld len=%d -> ref=%p port=%p",
		    seek, len, (void *)r, (void *)p);
	} else if ((long long)r->seek != (long long)p->seek ||
	    r->len != p->len ||
	    (long long)src.seek != (long long)psrc.seek ||
	    src.len != psrc.len || strcmp(errmsg, P::errmsg) != 0) {
		fail(st_dup,
		    "seek=%lld len=%d -> ref(seek=%lld len=%d) "
		    "port(seek=%lld len=%d)",
		    seek, len, (long long)r->seek, r->len,
		    (long long)p->seek, p->len);
	}
	free(r);
	free(p);
}

static void
test_dup_line_node(void)
{
	static const long long seeks[] = {
		0, 1, -1, 2, -2, 127, 128, 255, 256, 65535, 65536,
		2147483647LL, -2147483648LL, 4294967295LL, 4294967296LL,
		(long long)0x7fffffffffffffffLL, (long long)-0x7fffffffffffffffLL - 1
	};
	static const int lens[] = {
		0, 1, -1, 2, 127, 128, 255, 256, 65535, 65536,
		INT_MAX, INT_MIN, INT_MAX - 1, INT_MIN + 1
	};
	const int nsk = (int)(sizeof(seeks) / sizeof(seeks[0]));
	const int nl = (int)(sizeof(lens) / sizeof(lens[0]));

	for (int i = 0; i < nsk; i++)
		for (int j = 0; j < nl; j++)
			dup_case(seeks[i], lens[j]);

	rng_seed(0xD00D0004ULL);
	for (long it = 0; it < 200000; it++) {
		long long s;
		int l;

		if ((rnd() & 7) == 0)
			s = seeks[rnd() % nsk];
		else
			s = (long long)rnd();
		if ((rnd() & 7) == 0)
			l = lens[rnd() % nl];
		else
			l = (int)(uint32_t)rnd();
		dup_case(s, l);
	}
}

/* ------------------------------------------------------------------ */
/* has_trailing_escape                                                  */
/* ------------------------------------------------------------------ */

#define HTE_BUF 512
#define HTE_OFF 64		/* content offset; leaves room to step back */

static unsigned char *hte_b1;
static unsigned char *hte_b2;

static void
hte_case(const unsigned char *data, int len, int si, int ti)
{
	int r1, r2;

	st_hte.cases++;

	memset(hte_b1, 0x7f, HTE_BUF);
	memset(hte_b2, 0x7f, HTE_BUF);
	memcpy(hte_b1 + HTE_OFF, data, (size_t)len);
	memcpy(hte_b2 + HTE_OFF, data, (size_t)len);

	r1 = ref_has_trailing_escape((char *)hte_b1 + HTE_OFF + si,
	    (char *)hte_b1 + HTE_OFF + ti);
	r2 = P::has_trailing_escape((char *)hte_b2 + HTE_OFF + si,
	    (char *)hte_b2 + HTE_OFF + ti);

	if (r1 != r2 || memcmp(hte_b1, hte_b2, HTE_BUF) != 0) {
		char hex[128];
		int k = 0;

		for (int i = 0; i < len && k < 120; i++)
			k += snprintf(hex + k, sizeof(hex) - (size_t)k, "%02x",
			    data[i]);
		hex[k] = '\0';
		fail(st_hte, "data=%s len=%d s=+%d t=+%d -> ref=%d port=%d%s",
		    hex, len, si, ti, r1, r2,
		    memcmp(hte_b1, hte_b2, HTE_BUF) ? " (buffer differs)" : "");
	}
}

struct HteContent {
	int len;
	unsigned char b[24];
};

static const HteContent hte_contents[] = {
	{ 0,  { 0 } },
	{ 1,  { '\\' } },
	{ 1,  { 'a' } },
	{ 1,  { 0x00 } },
	{ 1,  { 0x80 } },
	{ 1,  { 0xff } },
	{ 2,  { '\\', '\\' } },
	{ 2,  { 'a', '\\' } },
	{ 2,  { '\\', 'a' } },
	{ 2,  { 0x00, '\\' } },
	{ 2,  { 0x80, '\\' } },
	{ 2,  { 0xff, '\\' } },
	{ 3,  { '\\', '\\', '\\' } },
	{ 3,  { 'a', '\\', '\\' } },
	{ 3,  { '\\', 'a', '\\' } },
	{ 3,  { '\\', '\\', 'a' } },
	{ 4,  { '\\', '\\', '\\', '\\' } },
	{ 4,  { 'a', '\\', '\\', '\\' } },
	{ 4,  { 0x00, '\\', '\\', '\\' } },
	{ 5,  { '\\', '\\', '\\', '\\', '\\' } },
	{ 5,  { 'x', '\\', '\\', 0x80, '\\' } },
	{ 6,  { '\\', 'a', '\\', '\\', 'b', '\\' } },
	{ 7,  { '\\', '\\', '\\', '\\', '\\', '\\', '\\' } },
	{ 8,  { 0xff, 0x80, '\\', '\\', 0x00, '\\', '\\', '\\' } },
	{ 9,  { '\\', '\\', '\\', '\\', '\\', '\\', '\\', '\\', '\\' } },
	{ 10, { 'a', 'b', '\\', '\\', '\\', 'c', '\\', '\\', 'd', '\\' } },
	{ 12, { '\\', '\\', '\\', '\\', '\\', '\\', '\\', '\\', '\\', '\\',
	        '\\', '\\' } },
	{ 16, { 0x5c, 0x5c, 0x5b, 0x5c, 0x5d, 0x5c, 0x5c, 0x5c,
	        0x00, 0x5c, 0x5c, 0xff, 0x5c, 0x5c, 0x5c, 0x5c } },
};

static void
test_has_trailing_escape(void)
{
	const int nc = (int)(sizeof(hte_contents) / sizeof(hte_contents[0]));

	hte_b1 = (unsigned char *)malloc(HTE_BUF);
	hte_b2 = (unsigned char *)malloc(HTE_BUF);
	if (hte_b1 == NULL || hte_b2 == NULL) {
		fprintf(stderr, "out of memory\n");
		exit(1);
	}

	/* Every (s, t) pair over each content, including t < s, t == s and
	   both a couple of bytes into the 0x7f guard region on either side. */
	for (int c = 0; c < nc; c++) {
		int len = hte_contents[c].len;

		for (int si = -2; si <= len + 2; si++)
			for (int ti = -2; ti <= len + 2; ti++)
				hte_case(hte_contents[c].b, len, si, ti);
	}

	/* Long runs of escapes: parity must alternate. */
	{
		unsigned char run[40];

		for (int n = 0; n <= 32; n++) {
			memset(run, '\\', sizeof(run));
			run[0] = 'q';		/* a definite terminator */
			for (int si = 0; si <= 1; si++)
				for (int ti = 1; ti <= n + 1 && ti <= 33; ti++)
					hte_case(run, n + 1, si, ti);
		}
	}

	rng_seed(0xFEED0005ULL);
	{
		static const unsigned char alpha[] = {
			'\\', '\\', '\\', 'a', 'b', 0x00, 0x80, 0xff, 0x5b, 0x5d
		};
		unsigned char buf[48];

		for (long it = 0; it < 200000; it++) {
			int len = (int)rnd_range(0, 40);
			int si, ti;

			for (int i = 0; i < len; i++)
				buf[i] = alpha[rnd() % (sizeof(alpha) /
				    sizeof(alpha[0]))];
			si = (int)rnd_range(-2, len + 2);
			ti = (int)rnd_range(-2, len + 2);
			hte_case(buf, len, si, ti);
		}
	}

	free(hte_b1);
	free(hte_b2);
}

/* ------------------------------------------------------------------ */
/* strip_escapes                                                        */
/* ------------------------------------------------------------------ */

#define SE_BUF 8320

static unsigned char *se_b1;
static unsigned char *se_b2;

static void
strip_case(const unsigned char *data, int len)
{
	char *r1;
	char *r2;
	int bad = 0;

	st_strip.cases++;

	memset(se_b1, 0x7f, SE_BUF);
	memset(se_b2, 0x7f, SE_BUF);
	memcpy(se_b1, data, (size_t)len);
	memcpy(se_b2, data, (size_t)len);
	se_b1[len] = '\0';
	se_b2[len] = '\0';

	errmsg = SENT;
	P::errmsg = SENT;

	r1 = ref_strip_escapes((char *)se_b1);
	r2 = P::strip_escapes((char *)se_b2);

	if (r1 == NULL || r2 == NULL)
		bad = 1;
	else if (memcmp(r1, r2, (size_t)FILESZ) != 0)
		bad = 1;
	if (memcmp(se_b1, se_b2, SE_BUF) != 0)	/* input must be untouched */
		bad = 2;
	if (strcmp(errmsg, P::errmsg) != 0)
		bad = 3;

	if (bad) {
		int first = -1;

		if (bad == 1 && r1 != NULL && r2 != NULL)
			for (int i = 0; i < FILESZ; i++)
				if (r1[i] != r2[i]) {
					first = i;
					break;
				}
		fail(st_strip,
		    "len=%d head=%02x%02x%02x tail=%02x%02x kind=%d "
		    "firstdiff=%d ref=%02x port=%02x",
		    len,
		    len > 0 ? data[0] : 0, len > 1 ? data[1] : 0,
		    len > 2 ? data[2] : 0,
		    len > 1 ? data[len - 2] : 0, len > 0 ? data[len - 1] : 0,
		    bad, first,
		    (first >= 0) ? (unsigned char)r1[first] : 0,
		    (first >= 0) ? (unsigned char)r2[first] : 0);
	}
}

static void
test_strip_escapes(void)
{
	static const unsigned char alpha[] = {
		'\\', '\\', 'a', 'b', 0x01, 0x7f, 0x80, 0xff, '/', '.'
	};
	unsigned char *buf;

	se_b1 = (unsigned char *)malloc(SE_BUF);
	se_b2 = (unsigned char *)malloc(SE_BUF);
	buf = (unsigned char *)malloc(SE_BUF);
	if (se_b1 == NULL || se_b2 == NULL || buf == NULL) {
		fprintf(stderr, "out of memory\n");
		exit(1);
	}

	/* --- hand-written short cases --- */
	{
		static const HteContent shorts[] = {
			{ 0,  { 0 } },
			{ 1,  { 'a' } },
			{ 1,  { '\\' } },
			{ 1,  { 0x80 } },
			{ 1,  { 0xff } },
			{ 2,  { '\\', '\\' } },
			{ 2,  { '\\', 'a' } },
			{ 2,  { 'a', '\\' } },
			{ 2,  { '\\', 0x00 } },
			{ 3,  { 'a', '\\', 'b' } },
			{ 3,  { '\\', '\\', '\\' } },
			{ 3,  { 'a', 0x00, 'b' } },
			{ 3,  { 0x00, 'a', 'b' } },
			{ 4,  { '\\', '\\', '\\', '\\' } },
			{ 4,  { '\\', 0xff, '\\', 0x80 } },
			{ 5,  { 'a', '\\', 0x00, 'b', 'c' } },
			{ 6,  { '\\', 'a', '\\', 'b', '\\', 'c' } },
			{ 8,  { 0xff, 0x80, 0x7f, 0x01, '\\', '\\', 0x00, 'z' } },
		};
		const int ns = (int)(sizeof(shorts) / sizeof(shorts[0]));

		for (int i = 0; i < ns; i++)
			strip_case(shorts[i].b, shorts[i].len);
		/* repeat: the static buffer keeps state across calls */
		for (int i = ns - 1; i >= 0; i--)
			strip_case(shorts[i].b, shorts[i].len);
	}

	/* --- boundary of `i < filesz - 1' --- */
	for (int L = FILESZ - 8; L <= FILESZ + 8; L++) {
		memset(buf, 'A', (size_t)L);
		strip_case(buf, L);
		buf[L - 1] = '\\';
		strip_case(buf, L);
		buf[L - 1] = 'A';
		buf[L - 2] = '\\';
		strip_case(buf, L);
		buf[L - 2] = 'A';
		buf[0] = '\\';
		strip_case(buf, L);
	}

	/* escaped pairs: two input bytes per output byte, so the boundary is
	   reached at about twice the length */
	for (int L = 2 * FILESZ - 8; L <= 2 * FILESZ + 8 && L < SE_BUF - 2;
	    L += 1) {
		for (int i = 0; i < L; i++)
			buf[i] = (i & 1) ? 'z' : '\\';
		strip_case(buf, L);
		for (int i = 0; i < L; i++)
			buf[i] = (i & 1) ? '\\' : 'z';
		strip_case(buf, L);
	}

	/* all escapes */
	for (int L = 1; L <= 24; L++) {
		memset(buf, '\\', (size_t)L);
		strip_case(buf, L);
	}
	{
		int L = SE_BUF - 4;

		memset(buf, '\\', (size_t)L);
		strip_case(buf, L);
	}

	/* strings that stop early on an embedded NUL */
	for (int L = 1; L <= 40; L++) {
		memset(buf, 'k', (size_t)L);
		for (int z = 0; z < L; z++) {
			buf[z] = '\0';
			strip_case(buf, L);
			buf[z] = 'k';
		}
	}

	/* --- randomised sweep --- */
	rng_seed(0x5EED0006ULL);
	for (long it = 0; it < 200000; it++) {
		int len;

		if (it % 1021 == 0)
			len = (int)rnd_range(FILESZ - 6, FILESZ + 6);
		else if (it % 509 == 0)
			len = (int)rnd_range(2 * FILESZ - 6, 2 * FILESZ + 6);
		else if (it % 97 == 0)
			len = (int)rnd_range(0, 600);
		else
			len = (int)rnd_range(0, 48);
		for (int i = 0; i < len; i++)
			buf[i] = alpha[rnd() % (sizeof(alpha) /
			    sizeof(alpha[0]))];
		strip_case(buf, len);
	}

	free(se_b1);
	free(se_b2);
	free(buf);
}

/* ------------------------------------------------------------------ */
/* is_legal_filename                                                    */
/* ------------------------------------------------------------------ */

#define ILF_BUF 64

static void
ilf_case(int redval, const unsigned char *data, int len)
{
	unsigned char b1[ILF_BUF];
	unsigned char b2[ILF_BUF];
	int r1, r2;
	const char *e1;

	st_ilf.cases++;

	memset(b1, 0x7f, sizeof(b1));
	memset(b2, 0x7f, sizeof(b2));
	memcpy(b1, data, (size_t)len);
	memcpy(b2, data, (size_t)len);
	b1[len] = '\0';
	b2[len] = '\0';

	errmsg = SENT;
	P::errmsg = SENT;
	red = redval;
	P::red = redval;

	r1 = ref_is_legal_filename((char *)b1);
	e1 = errmsg;
	r2 = P::is_legal_filename((char *)b2);

	if (r1 != r2 || strcmp(e1, P::errmsg) != 0 ||
	    memcmp(b1, b2, sizeof(b1)) != 0) {
		char hex[160];
		int k = 0;

		for (int i = 0; i < len && k < 150; i++)
			k += snprintf(hex + k, sizeof(hex) - (size_t)k, "%02x",
			    data[i]);
		hex[k] = '\0';
		fail(st_ilf, "red=%d s=%s len=%d -> ref(%d \"%s\") port(%d \"%s\")",
		    redval, hex, len, r1, e1, r2, P::errmsg);
	}
}

static void
test_is_legal_filename(void)
{
	static const char *strs[] = {
		"", "!", "!!", "!a", "a!", ".", "..", "...", "..a", "a..",
		".a.", "/", "//", "/a", "a/", "a/b", "ab", "a", "!/", "/!",
		"!..", "../", "..!", "\x80", "\xff", "\x80/", "/\x80",
		"\x80..", "..\x80", "!\x80", "\x7f", " ", " /", "-", "-!",
		"a\\b", "\\", "\\/", ".!", "!.", "b/c/d", "..b", "x"
	};
	const int nstr = (int)(sizeof(strs) / sizeof(strs[0]));

	for (int r = 0; r <= 1; r++)
		for (int i = 0; i < nstr; i++)
			ilf_case(r, (const unsigned char *)strs[i],
			    (int)strlen(strs[i]));

	/* every one-, two- and three-byte word over the interesting alphabet:
	   this puts each of the three restricted-mode tests independently on
	   both sides of its boundary */
	{
		static const unsigned char alpha[] = {
			'!', '.', '/', 'a', 0x80, 0xff
		};
		const int na = (int)(sizeof(alpha) / sizeof(alpha[0]));
		unsigned char w[4];

		for (int r = 0; r <= 1; r++) {
			for (int i = 0; i < na; i++) {
				w[0] = alpha[i];
				ilf_case(r, w, 1);
				for (int j = 0; j < na; j++) {
					w[1] = alpha[j];
					ilf_case(r, w, 2);
					for (int k = 0; k < na; k++) {
						w[2] = alpha[k];
						ilf_case(r, w, 3);
					}
				}
			}
		}
	}

	rng_seed(0xABCD0007ULL);
	{
		static const unsigned char alpha[] = {
			'!', '.', '.', '/', 'a', 'b', 0x01, 0x80, 0xff, 0x7f
		};
		unsigned char buf[ILF_BUF];

		for (long it = 0; it < 200000; it++) {
			int len = (int)rnd_range(0, 12);

			for (int i = 0; i < len; i++)
				buf[i] = alpha[rnd() % (sizeof(alpha) /
				    sizeof(alpha[0]))];
			ilf_case((int)(rnd() & 1), buf, len);
		}
		/* also with non-boolean `red' values */
		for (long it = 0; it < 20000; it++) {
			int len = (int)rnd_range(0, 8);

			for (int i = 0; i < len; i++)
				buf[i] = alpha[rnd() % (sizeof(alpha) /
				    sizeof(alpha[0]))];
			ilf_case((int)rnd_range(-3, 3), buf, len);
		}
	}
	red = 0;
	P::red = 0;
}

/* ------------------------------------------------------------------ */

static void
row(const Stat &s)
{
	printf("  %-22s %10ld %10ld  %s\n", s.name, s.cases, s.fails,
	    s.fails ? "FAIL" : "ok");
}

int
main(void)
{
	long total_fail = 0;

	test_check_addr_range();
	test_marks();
	test_dup_line_node();
	test_has_trailing_escape();
	test_strip_escapes();
	test_is_legal_filename();

	printf("\nPBSD b0193s3 -- bin/ed/main.c differential test\n");
	printf("  %-22s %10s %10s  %s\n", "function", "cases", "failures",
	    "status");
	printf("  ----------------------------------------------------------\n");
	row(st_car);
	row(st_mark);
	row(st_unmk);
	row(st_dup);
	row(st_hte);
	row(st_strip);
	row(st_ilf);
	printf("  ----------------------------------------------------------\n");

	total_fail = st_car.fails + st_mark.fails + st_unmk.fails +
	    st_dup.fails + st_hte.fails + st_strip.fails + st_ilf.fails;
	printf("  %-22s %10ld %10ld  %s\n", "TOTAL",
	    st_car.cases + st_mark.cases + st_unmk.cases + st_dup.cases +
	    st_hte.cases + st_strip.cases + st_ilf.cases,
	    total_fail, total_fail ? "FAIL" : "ok");

	return total_fail ? 1 : 0;
}
