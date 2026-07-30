/*
 * Differential test for batch b0017: pbsd.lib.libc.gen.b0017 against the
 * unmodified C sources in oracle.c.
 *
 * The four functions in this batch share one piece of mutable state
 * (_rand48_seed / _rand48_mult / _rand48_add).  The port and the oracle each
 * own a private copy of that state, so every operation is applied to both and
 * the ENTIRE state, plus every return value and every caller-visible byte, is
 * compared after each single step.  State is never resynchronised: once the two
 * sides disagree the divergence is reported and then allowed to propagate.
 */

#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

import pbsd.lib.libc.gen.b0017;

namespace port = pbsd::lib_libc_gen::b0017;

extern "C" {
long ref_mrand48(void);
void ref_lcong48(unsigned short p[7]);
void ref_srand48(long seed);
unsigned short *ref_seed48(unsigned short xseed[3]);

extern std::uint64_t _rand48_seed;
extern std::uint64_t _rand48_mult;
extern std::uint64_t _rand48_add;
}

/* ------------------------------------------------------------------ */
/* bookkeeping							      */
/* ------------------------------------------------------------------ */

enum { F_MRAND48, F_LCONG48, F_SRAND48, F_SEED48, NFUNC };

static const char *const fname[NFUNC] = {
	"mrand48", "lcong48", "srand48", "seed48"
};

static long long ncases[NFUNC];
static long long nfails[NFUNC];
static int nreported;

static const int MAXREPORT = 15;

static void
report(int f, const char *ctx, const char *detail)
{
	nfails[f]++;
	if (nreported < MAXREPORT) {
		nreported++;
		std::printf("FAIL %-8s %s: %s\n", fname[f], ctx, detail);
	} else if (nreported == MAXREPORT) {
		nreported++;
		std::printf("... further failures suppressed\n");
	}
}

/* coverage witnesses, printed at the end to show the branches were driven */
static long long cov_neg, cov_pos, cov_zero;
static long long cov_add0, cov_mult0, cov_mult1, cov_seed0;
static long long cov_hi_seed, cov_hi_mult;

/*
 * Compare the whole shared state.  Any single word differing is a failure.
 */
static bool
state_ok(char *msg, std::size_t msgsz)
{
	if (port::_rand48_seed != _rand48_seed) {
		std::snprintf(msg, msgsz, "_rand48_seed port=%016llx ref=%016llx",
		    (unsigned long long)port::_rand48_seed,
		    (unsigned long long)_rand48_seed);
		return false;
	}
	if (port::_rand48_mult != _rand48_mult) {
		std::snprintf(msg, msgsz, "_rand48_mult port=%016llx ref=%016llx",
		    (unsigned long long)port::_rand48_mult,
		    (unsigned long long)_rand48_mult);
		return false;
	}
	if (port::_rand48_add != _rand48_add) {
		std::snprintf(msg, msgsz, "_rand48_add port=%016llx ref=%016llx",
		    (unsigned long long)port::_rand48_add,
		    (unsigned long long)_rand48_add);
		return false;
	}
	return true;
}

static void
note_state(void)
{
	if (_rand48_add == 0)
		cov_add0++;
	if (_rand48_mult == 0)
		cov_mult0++;
	if (_rand48_mult == 1)
		cov_mult1++;
	if (_rand48_seed == 0)
		cov_seed0++;
	if (_rand48_seed >> 32)
		cov_hi_seed++;
	if (_rand48_mult >> 32)
		cov_hi_mult++;
}

/*
 * Guarded buffer.  The argument window sits in the middle of a block that is
 * entirely filled with the guard byte 0x7f, so bytes before and well past the
 * nominal window are compared too.
 */
struct SBuf {
	static const int N = 32;	/* 64 bytes */
	static const int OFF = 8;	/* window start, in shorts */
	unsigned short w[N];

	void init(const unsigned short *in, int n)
	{
		std::memset(w, 0x7f, sizeof w);
		for (int i = 0; i < n; i++)
			w[OFF + i] = in[i];
	}
	unsigned short *arg() { return w + OFF; }
	bool contains(const unsigned short *p) const
	{
		return p >= w && p < w + N;
	}
};

static void
hexbuf(char *out, std::size_t outsz, const SBuf &b)
{
	std::size_t k = 0;
	for (int i = 0; i < SBuf::N && k + 6 < outsz; i++)
		k += (std::size_t)std::snprintf(out + k, outsz - k, "%04x ",
		    b.w[i]);
}

/* ------------------------------------------------------------------ */
/* one operation applied to both sides				      */
/* ------------------------------------------------------------------ */

static void
op_mrand48(const char *ctx)
{
	char msg[256];
	bool ok = true;

	ncases[F_MRAND48]++;

	long a = port::mrand48();
	long b = ref_mrand48();

	if (a != b) {
		std::snprintf(msg, sizeof msg,
		    "return port=%ld (%016llx) ref=%ld (%016llx)",
		    a, (unsigned long long)a, b, (unsigned long long)b);
		ok = false;
	} else if (!state_ok(msg, sizeof msg))
		ok = false;

	if (b < 0)
		cov_neg++;
	else if (b > 0)
		cov_pos++;
	else
		cov_zero++;
	note_state();

	if (!ok)
		report(F_MRAND48, ctx, msg);
}

static void
op_srand48(long seed, const char *ctx)
{
	char msg[256], full[384];

	ncases[F_SRAND48]++;

	port::srand48(seed);
	ref_srand48(seed);

	if (!state_ok(msg, sizeof msg)) {
		std::snprintf(full, sizeof full, "seed=%ld (%016llx) %s",
		    seed, (unsigned long long)seed, msg);
		report(F_SRAND48, ctx, full);
	}
	note_state();
}

static void
op_lcong48(const unsigned short p[7], const char *ctx)
{
	SBuf a, b, pristine;
	char msg[256], full[1024], ha[256], hb[256];
	bool ok = true;

	ncases[F_LCONG48]++;

	a.init(p, 7);
	b.init(p, 7);
	pristine.init(p, 7);

	port::lcong48(a.arg());
	ref_lcong48(b.arg());

	if (std::memcmp(a.w, b.w, sizeof a.w) != 0) {
		hexbuf(ha, sizeof ha, a);
		hexbuf(hb, sizeof hb, b);
		std::snprintf(msg, sizeof msg, "buffers diverged");
		std::snprintf(full, sizeof full, "%s\n  port: %s\n  ref : %s",
		    msg, ha, hb);
		report(F_LCONG48, ctx, full);
		ok = false;
	} else if (std::memcmp(a.w, pristine.w, sizeof a.w) != 0) {
		report(F_LCONG48, ctx, "port wrote to the caller's array");
		ok = false;
	} else if (std::memcmp(b.w, pristine.w, sizeof b.w) != 0) {
		report(F_LCONG48, ctx, "oracle wrote to the caller's array");
		ok = false;
	}

	if (ok && !state_ok(msg, sizeof msg)) {
		std::snprintf(full, sizeof full,
		    "p={%04x,%04x,%04x,%04x,%04x,%04x,%04x} %s",
		    p[0], p[1], p[2], p[3], p[4], p[5], p[6], msg);
		report(F_LCONG48, ctx, full);
	}
	note_state();
}

static unsigned short *seed48_pbase;
static unsigned short *seed48_rbase;
static bool seed48_have_base;

static void
op_seed48(const unsigned short xs[3], const char *ctx)
{
	SBuf a, b, pristine;
	char msg[256], full[1024];
	bool ok = true;

	ncases[F_SEED48]++;

	a.init(xs, 3);
	b.init(xs, 3);
	pristine.init(xs, 3);

	unsigned short *ra = port::seed48(a.arg());
	unsigned short *rb = ref_seed48(b.arg());

	if (ra == NULL || rb == NULL) {
		std::snprintf(msg, sizeof msg, "null return port=%p ref=%p",
		    (void *)ra, (void *)rb);
		report(F_SEED48, ctx, msg);
		note_state();
		return;
	}

	if (!seed48_have_base) {
		seed48_pbase = ra;
		seed48_rbase = rb;
		seed48_have_base = true;
	}

	/* pointer identity is compared as offsets from each side's own base */
	std::ptrdiff_t offa = ra - seed48_pbase;
	std::ptrdiff_t offb = rb - seed48_rbase;

	if (offa != offb) {
		std::snprintf(msg, sizeof msg, "return offset port=%td ref=%td",
		    offa, offb);
		report(F_SEED48, ctx, msg);
		ok = false;
	} else if (offa != 0) {
		std::snprintf(msg, sizeof msg,
		    "return not the stable static buffer (offset %td)", offa);
		report(F_SEED48, ctx, msg);
		ok = false;
	} else if (a.contains(ra) || b.contains(rb)) {
		report(F_SEED48, ctx, "return aliases the caller's array");
		ok = false;
	} else if (std::memcmp(ra, rb, 3 * sizeof(unsigned short)) != 0) {
		std::snprintf(msg, sizeof msg,
		    "previous seed port={%04x,%04x,%04x} ref={%04x,%04x,%04x}",
		    ra[0], ra[1], ra[2], rb[0], rb[1], rb[2]);
		report(F_SEED48, ctx, msg);
		ok = false;
	} else if (std::memcmp(a.w, b.w, sizeof a.w) != 0) {
		report(F_SEED48, ctx, "buffers diverged");
		ok = false;
	} else if (std::memcmp(a.w, pristine.w, sizeof a.w) != 0) {
		report(F_SEED48, ctx, "port wrote to the caller's array");
		ok = false;
	} else if (std::memcmp(b.w, pristine.w, sizeof b.w) != 0) {
		report(F_SEED48, ctx, "oracle wrote to the caller's array");
		ok = false;
	}

	if (ok && !state_ok(msg, sizeof msg)) {
		std::snprintf(full, sizeof full,
		    "xseed={%04x,%04x,%04x} %s", xs[0], xs[1], xs[2], msg);
		report(F_SEED48, ctx, full);
	}
	note_state();
}

/* ------------------------------------------------------------------ */
/* fixed-seed PRNG for the sweeps (independent of the code under test) */
/* ------------------------------------------------------------------ */

static std::uint64_t rs = 0x0123456789abcdefULL;

static std::uint64_t
nextr(void)
{
	std::uint64_t z = (rs += 0x9e3779b97f4a7c15ULL);

	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return z ^ (z >> 31);
}

static const long seed_edges[] = {
	0L, 1L, -1L, 2L, -2L, 3L, -3L,
	0x7fffL, 0x8000L, 0x8001L, 0xfffeL, 0xffffL, 0x10000L, 0x10001L,
	-0x7fffL, -0x8000L, -0x8001L, -0xffffL, -0x10000L, -0x10001L,
	0x330eL, 0x330e0000L, 0xabcdL, 0x1234L,
	0x7fffffffL, -0x7fffffffL - 1L, 0x80000000L, 0xffffffffL,
	0x100000000L, 0x1ffffffffL, 0xffff0000L, 0x0000ffffL,
	0x7fff7fffL, (long)0x8000800080008000ULL, (long)0xaaaa5555aaaa5555ULL,
	0x5555aaaaL, (long)0xdeadbeefcafebabeULL, 0x1234abcdL,
	LONG_MAX, LONG_MIN, LONG_MAX - 1L, LONG_MIN + 1L,
	(long)0xffffffffffff0000ULL, (long)0xffffffff0000ffffULL,
	(long)0x0000ffffffffffffULL, (long)0xffff00000000ffffULL,
};

static const int nseed_edges = (int)(sizeof seed_edges / sizeof seed_edges[0]);

static long
rand_long(void)
{
	std::uint64_t r = nextr();

	switch (r % 6) {
	case 0:
		return seed_edges[(int)((r >> 8) % (unsigned)nseed_edges)];
	case 1:
		return (long)(short)(r >> 12);		/* small, both signs */
	case 2:
		return (long)(std::int32_t)(r >> 16);	/* 32-bit range */
	case 3:
		return (long)(r >> 40);			/* small positive */
	default:
		return (long)nextr();			/* full 64-bit */
	}
}

static const unsigned short short_edges[] = {
	0x0000, 0x0001, 0x0002, 0x7ffe, 0x7fff, 0x8000, 0x8001,
	0xfffe, 0xffff, 0x00ff, 0xff00, 0x80ff, 0xff80, 0x0080, 0x8080,
	0x330e, 0xabcd, 0x1234, 0xe66d, 0xdeec, 0x0005, 0x000b
};

static const int nshort_edges =
    (int)(sizeof short_edges / sizeof short_edges[0]);

static unsigned short
rand_short(void)
{
	std::uint64_t r = nextr();

	if (r % 3 == 0)
		return short_edges[(int)((r >> 8) % (unsigned)nshort_edges)];
	if (r % 7 == 0)
		return (unsigned short)(0x8000u | (unsigned)(r >> 20));
	return (unsigned short)(r >> 13);
}

/* ------------------------------------------------------------------ */
/* hand-written edge cases					      */
/* ------------------------------------------------------------------ */

static void
edge_srand48(void)
{
	char ctx[64];

	for (int i = 0; i < nseed_edges; i++) {
		std::snprintf(ctx, sizeof ctx, "edge/srand48[%d]", i);
		op_srand48(seed_edges[i], ctx);
		/* the generated sequence is part of srand48's observable effect */
		for (int k = 0; k < 6; k++)
			op_mrand48(ctx);
	}

	/*
	 * Walk single bits through the seed: every bit of the low 32 must
	 * reach the state and every bit above it must be discarded.
	 */
	for (int bit = 0; bit < 64; bit++) {
		std::snprintf(ctx, sizeof ctx, "edge/srand48/bit%d", bit);
		op_srand48((long)(1ULL << bit), ctx);
		op_mrand48(ctx);
		op_srand48(-(long)(1ULL << bit), ctx);
		op_mrand48(ctx);
		op_srand48((long)~(1ULL << bit), ctx);
		op_mrand48(ctx);
	}
}

static void
edge_lcong48(void)
{
	unsigned short p[7];
	char ctx[64];

	static const unsigned short fixed[][7] = {
		{ 0, 0, 0, 0, 0, 0, 0 },
		{ 1, 1, 1, 1, 1, 1, 1 },
		{ 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff },
		{ 0, 1, 2, 3, 4, 5, 6 },
		{ 1, 2, 3, 4, 5, 6, 7 },
		{ 7, 6, 5, 4, 3, 2, 1 },
		{ 0x330e, 0xabcd, 0x1234, 0xe66d, 0xdeec, 0x0005, 0x000b },
		{ 0x1111, 0x2222, 0x3333, 0x4444, 0x5555, 0x6666, 0x7777 },
		{ 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000 },
		{ 0x80ff, 0xff80, 0x8000, 0xffff, 0x8001, 0x7fff, 0xfffe },
		{ 0x00ff, 0xff00, 0x00ff, 0xff00, 0x00ff, 0xff00, 0x00ff },
		/* mult == 0: seed collapses to add every step */
		{ 0xdead, 0xbeef, 0xcafe, 0, 0, 0, 0x0007 },
		/* mult == 1: seed advances by add every step */
		{ 0xdead, 0xbeef, 0xcafe, 1, 0, 0, 0x0007 },
		/* add == 0 */
		{ 0xdead, 0xbeef, 0xcafe, 0xe66d, 0xdeec, 0x0005, 0 },
		/* mult == 0 and add == 0: seed frozen at 0 */
		{ 0xffff, 0xffff, 0xffff, 0, 0, 0, 0 },
		/* seed == 0, everything else live */
		{ 0, 0, 0, 0xe66d, 0xdeec, 0x0005, 0x000b },
		/* only the top word of seed and mult set */
		{ 0, 0, 0xffff, 0, 0, 0xffff, 0xffff },
		/* only the middle word set */
		{ 0, 0xffff, 0, 0, 0xffff, 0, 0x0001 },
	};
	static const int nfixed = (int)(sizeof fixed / sizeof fixed[0]);

	for (int i = 0; i < nfixed; i++) {
		std::snprintf(ctx, sizeof ctx, "edge/lcong48[%d]", i);
		op_lcong48(fixed[i], ctx);
		for (int k = 0; k < 5; k++)
			op_mrand48(ctx);
	}

	/*
	 * Isolation: exactly one element non-zero.  Any index or shift slip in
	 * LOADRAND48 / the &p[0], &p[3], p[6] arguments moves the value into a
	 * different word of the state, or drops it, and is caught here.
	 */
	static const unsigned short marks[] = { 0x0001, 0xbeef, 0x8000, 0xffff };

	for (unsigned m = 0; m < sizeof marks / sizeof marks[0]; m++) {
		for (int i = 0; i < 7; i++) {
			std::memset(p, 0, sizeof p);
			p[i] = marks[m];
			std::snprintf(ctx, sizeof ctx,
			    "edge/lcong48/iso[%u][%d]", m, i);
			op_lcong48(p, ctx);
			for (int k = 0; k < 3; k++)
				op_mrand48(ctx);
		}
		/* and the complement: exactly one element zero */
		for (int i = 0; i < 7; i++) {
			for (int j = 0; j < 7; j++)
				p[j] = marks[m];
			p[i] = 0;
			std::snprintf(ctx, sizeof ctx,
			    "edge/lcong48/hole[%u][%d]", m, i);
			op_lcong48(p, ctx);
			for (int k = 0; k < 3; k++)
				op_mrand48(ctx);
		}
	}

	/* single bit in each of the seven words */
	for (int i = 0; i < 7; i++) {
		for (int bit = 0; bit < 16; bit++) {
			std::memset(p, 0, sizeof p);
			p[i] = (unsigned short)(1u << bit);
			std::snprintf(ctx, sizeof ctx,
			    "edge/lcong48/bit[%d][%d]", i, bit);
			op_lcong48(p, ctx);
			op_mrand48(ctx);
		}
	}
}

static void
edge_seed48(void)
{
	unsigned short xs[3];
	unsigned short p[7];
	char ctx[64];

	static const unsigned short fixed[][3] = {
		{ 0, 0, 0 },
		{ 1, 1, 1 },
		{ 0xffff, 0xffff, 0xffff },
		{ 1, 2, 3 },
		{ 3, 2, 1 },
		{ 0x330e, 0xabcd, 0x1234 },
		{ 0x8000, 0x8000, 0x8000 },
		{ 0x00ff, 0xff00, 0x80ff },
		{ 0xbeef, 0, 0 },
		{ 0, 0xbeef, 0 },
		{ 0, 0, 0xbeef },
		{ 0xffff, 0, 0 },
		{ 0, 0xffff, 0 },
		{ 0, 0, 0xffff },
		{ 0x1111, 0x2222, 0x3333 },
	};
	static const int nfixed = (int)(sizeof fixed / sizeof fixed[0]);

	/*
	 * seed48 returns the PREVIOUS seed through STORERAND48, so the state it
	 * starts from must have three distinct, recognisable 16-bit words for
	 * an index or shift slip in the store to be visible.
	 */
	static const unsigned short pre[][3] = {
		{ 0x1111, 0x2222, 0x3333 },
		{ 0xaaaa, 0x5555, 0x0f0f },
		{ 0x0001, 0x0002, 0x0004 },
		{ 0xffff, 0x0000, 0xffff },
		{ 0x0000, 0xffff, 0x0000 },
		{ 0x8000, 0x0001, 0x7fff },
		{ 0xdead, 0xbeef, 0xcafe },
		{ 0, 0, 0 },
	};
	static const int npre = (int)(sizeof pre / sizeof pre[0]);

	for (int q = 0; q < npre; q++) {
		for (int i = 0; i < nfixed; i++) {
			/* install a known previous seed without using seed48 */
			p[0] = pre[q][0];
			p[1] = pre[q][1];
			p[2] = pre[q][2];
			p[3] = 0xe66d;
			p[4] = 0xdeec;
			p[5] = 0x0005;
			p[6] = 0x000b;
			std::snprintf(ctx, sizeof ctx, "edge/seed48/pre[%d][%d]",
			    q, i);
			op_lcong48(p, ctx);
			op_seed48(fixed[i], ctx);
			for (int k = 0; k < 3; k++)
				op_mrand48(ctx);
		}
	}

	/* single bit walk through xseed, and back-to-back seed48 calls */
	for (int i = 0; i < 3; i++) {
		for (int bit = 0; bit < 16; bit++) {
			std::memset(xs, 0, sizeof xs);
			xs[i] = (unsigned short)(1u << bit);
			std::snprintf(ctx, sizeof ctx, "edge/seed48/bit[%d][%d]",
			    i, bit);
			op_seed48(xs, ctx);
			op_seed48(xs, ctx);
			op_mrand48(ctx);
			op_seed48(xs, ctx);
		}
	}

	/* seed48 immediately after srand48 and after mrand48 */
	for (int i = 0; i < nseed_edges; i++) {
		std::snprintf(ctx, sizeof ctx, "edge/seed48/after[%d]", i);
		op_srand48(seed_edges[i], ctx);
		op_seed48(fixed[i % nfixed], ctx);
		op_mrand48(ctx);
		op_seed48(fixed[(i + 1) % nfixed], ctx);
	}
}

static void
edge_mrand48(void)
{
	unsigned short p[7];
	char ctx[64];

	/*
	 * Drive the (int) narrowing in both directions: bit 47 of the seed
	 * decides the sign of the result, so set the state so that the next
	 * value has it clear, then set, then straddle it.  mult == 1 makes the
	 * sequence a simple walk so the values are predictable.
	 */
	static const unsigned short states[][7] = {
		{ 0x0000, 0x0000, 0x0000, 1, 0, 0, 0x0001 },
		{ 0xffff, 0xffff, 0x7fff, 1, 0, 0, 0x0001 },
		{ 0xffff, 0xffff, 0x7fff, 1, 0, 0, 0x0000 },
		{ 0x0000, 0x0000, 0x8000, 1, 0, 0, 0x0001 },
		{ 0xffff, 0xffff, 0xffff, 1, 0, 0, 0x0001 },
		{ 0xffff, 0xffff, 0xffff, 1, 0, 0, 0x0000 },
		{ 0x0000, 0x8000, 0x0000, 1, 0, 0, 0x0000 },
		{ 0x0000, 0x0000, 0x0001, 1, 0, 0, 0x0000 },
		{ 0xffff, 0x0000, 0x0000, 1, 0, 0, 0x0001 },
	};
	static const int nstates = (int)(sizeof states / sizeof states[0]);

	for (int i = 0; i < nstates; i++) {
		std::snprintf(ctx, sizeof ctx, "edge/mrand48[%d]", i);
		op_lcong48(states[i], ctx);
		for (int k = 0; k < 8; k++)
			op_mrand48(ctx);
	}

	/* long runs from the two canonical starting points */
	std::snprintf(ctx, sizeof ctx, "edge/mrand48/run0");
	op_srand48(0, ctx);
	for (int k = 0; k < 512; k++)
		op_mrand48(ctx);

	std::snprintf(ctx, sizeof ctx, "edge/mrand48/run1");
	op_srand48(1, ctx);
	for (int k = 0; k < 512; k++)
		op_mrand48(ctx);

	/* default state, untouched by any seeding call */
	p[0] = 0x330e;
	p[1] = 0xabcd;
	p[2] = 0x1234;
	p[3] = 0xe66d;
	p[4] = 0xdeec;
	p[5] = 0x0005;
	p[6] = 0x000b;
	std::snprintf(ctx, sizeof ctx, "edge/mrand48/default");
	op_lcong48(p, ctx);
	for (int k = 0; k < 512; k++)
		op_mrand48(ctx);
}

/* ------------------------------------------------------------------ */
/* fixed-seed randomised sweeps					      */
/* ------------------------------------------------------------------ */

static const long SWEEP = 200000;

static void
sweep_mrand48(void)
{
	char ctx[64];
	unsigned short p[7];

	for (long i = 0; i < SWEEP; i++) {
		if (i % 977 == 0) {
			/* revisit mult/add so l*mult+add is exercised widely */
			std::snprintf(ctx, sizeof ctx, "sweep/mrand48/reseed%ld",
			    i);
			if ((i / 977) % 2 == 0)
				op_srand48(rand_long(), ctx);
			else {
				for (int j = 0; j < 7; j++)
					p[j] = rand_short();
				op_lcong48(p, ctx);
			}
		}
		std::snprintf(ctx, sizeof ctx, "sweep/mrand48[%ld]", i);
		op_mrand48(ctx);
	}
}

static void
sweep_srand48(void)
{
	char ctx[64];

	for (long i = 0; i < SWEEP; i++) {
		std::snprintf(ctx, sizeof ctx, "sweep/srand48[%ld]", i);
		op_srand48(rand_long(), ctx);
		op_mrand48(ctx);
	}
}

static void
sweep_lcong48(void)
{
	unsigned short p[7];
	char ctx[64];

	for (long i = 0; i < SWEEP; i++) {
		for (int j = 0; j < 7; j++)
			p[j] = rand_short();
		std::snprintf(ctx, sizeof ctx, "sweep/lcong48[%ld]", i);
		op_lcong48(p, ctx);
		op_mrand48(ctx);
	}
}

static void
sweep_seed48(void)
{
	unsigned short xs[3];
	char ctx[64];

	for (long i = 0; i < SWEEP; i++) {
		for (int j = 0; j < 3; j++)
			xs[j] = rand_short();
		std::snprintf(ctx, sizeof ctx, "sweep/seed48[%ld]", i);
		op_seed48(xs, ctx);
		if (i % 3 == 0)
			op_mrand48(ctx);
	}
}

static void
sweep_mixed(void)
{
	unsigned short buf[7];
	char ctx[64];

	for (long i = 0; i < SWEEP; i++) {
		std::uint64_t r = nextr();

		std::snprintf(ctx, sizeof ctx, "sweep/mixed[%ld]", i);
		switch (r % 8) {
		case 0:
		case 1:
		case 2:
			op_mrand48(ctx);
			break;
		case 3:
		case 4:
			op_srand48(rand_long(), ctx);
			break;
		case 5:
		case 6:
			for (int j = 0; j < 3; j++)
				buf[j] = rand_short();
			op_seed48(buf, ctx);
			break;
		default:
			for (int j = 0; j < 7; j++)
				buf[j] = rand_short();
			op_lcong48(buf, ctx);
			break;
		}
	}
}

/* ------------------------------------------------------------------ */

int
main(void)
{
	long long total_cases = 0, total_fails = 0;

	/*
	 * The very first comparison must happen before anything reseeds, so the
	 * statically initialised default state is covered too.
	 */
	{
		char msg[256];
		if (!state_ok(msg, sizeof msg)) {
			std::printf("FAIL initial state: %s\n", msg);
			nfails[F_SRAND48]++;
		}
		ncases[F_SRAND48]++;
	}
	op_mrand48("initial/default-state");

	edge_mrand48();
	edge_srand48();
	edge_lcong48();
	edge_seed48();

	sweep_mrand48();
	sweep_srand48();
	sweep_lcong48();
	sweep_seed48();
	sweep_mixed();

	std::printf("\n");
	std::printf("function      cases    failures\n");
	std::printf("------------------------------------\n");
	for (int f = 0; f < NFUNC; f++) {
		std::printf("%-10s %10lld  %10lld\n", fname[f], ncases[f],
		    nfails[f]);
		total_cases += ncases[f];
		total_fails += nfails[f];
	}
	std::printf("------------------------------------\n");
	std::printf("%-10s %10lld  %10lld\n", "TOTAL", total_cases,
	    total_fails);

	std::printf("\ncoverage: mrand48 return <0=%lld >0=%lld ==0=%lld\n",
	    cov_neg, cov_pos, cov_zero);
	std::printf("coverage: state add==0=%lld mult==0=%lld mult==1=%lld "
	    "seed==0=%lld\n", cov_add0, cov_mult0, cov_mult1, cov_seed0);
	std::printf("coverage: state seed>>32!=0=%lld mult>>32!=0=%lld\n",
	    cov_hi_seed, cov_hi_mult);

	if (total_fails != 0) {
		std::printf("\nRESULT: FAIL (%lld of %lld cases diverged)\n",
		    total_fails, total_cases);
		return 1;
	}
	std::printf("\nRESULT: PASS (%lld cases, 0 failures)\n", total_cases);
	return 0;
}
