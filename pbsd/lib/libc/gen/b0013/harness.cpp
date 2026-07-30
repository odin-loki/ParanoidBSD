/*
 * b0013 differential test: pbsd::lib_libc_gen::b0013 vs. the ref_ oracle.
 *
 * Every case runs the port and the oracle on byte-identical inputs held in
 * two separate guard-filled buffers and compares
 *	- the return value (doubles are compared as raw bit patterns, so that
 *	  0.0 vs -0.0 and differing NaNs are caught), and
 *	- the ENTIRE buffer afterwards, including the guard bytes on both sides
 *	  of the three-element xseed window, so that a stray store or a wrong
 *	  STORERAND48 index cannot slip through.
 * None of these functions returns a pointer; the xseed array is the only
 * mutable state, and the stateful (iterator-like) use is covered by the chain
 * tests, which re-check the return value and the whole buffer after every
 * single call in a long chain.
 *
 * The seed that produces a given internal LCG output is computable (the low 48
 * bits of the LCG step are a bijection), so the targeted cases below hit every
 * value of erand48's normalisation shift count s (0..47), both sides of
 * jrand48's sign boundary, and the extremes of nrand48's mask, in addition to
 * a 250000-iteration fixed-seed random sweep per function.
 */

#include <cstdio>
#include <cstring>
#include <cstdint>
#include <cstdlib>
#include <climits>

import pbsd.lib.libc.gen.b0013;

namespace port = pbsd::lib_libc_gen::b0013;

extern "C" {
extern std::uint64_t ref__rand48_seed;
extern std::uint64_t ref__rand48_mult;
extern std::uint64_t ref__rand48_add;
long ref_nrand48(unsigned short *);
double ref_erand48(unsigned short *);
long ref_jrand48(unsigned short *);
}

/* ------------------------------------------------------------------ */

static const std::uint64_t MASK48 = 0xffffffffffffULL;

enum { S_GLOB, S_N, S_E, S_J, NSTAT };

struct Stats {
	const char *name;
	long long cases;
	long long fails;
	int printed;
};

static Stats g_stat[NSTAT] = {
	{ "_rand48 globals", 0, 0, 0 },
	{ "nrand48",         0, 0, 0 },
	{ "erand48",         0, 0, 0 },
	{ "jrand48",         0, 0, 0 },
};

static const int MAXPRINT = 8;

/* Guard layout: 12 shorts, the xseed[3] window lives at index 4. */
static const int NSH = 12;
static const int SI = 4;
static const std::size_t BUFB = NSH * sizeof(unsigned short);

static void
fill(unsigned short *buf, const unsigned short s[3])
{
	std::memset(buf, 0x7f, BUFB);
	buf[SI + 0] = s[0];
	buf[SI + 1] = s[1];
	buf[SI + 2] = s[2];
}

static void
hexbuf(char *out, const unsigned short *b)
{
	int n = 0;

	for (int i = 0; i < NSH; i++)
		n += std::sprintf(out + n, "%04x%s", b[i],
		    i == NSH - 1 ? "" : " ");
}

static void
mkseed(std::uint64_t v, unsigned short s[3])
{
	s[0] = (unsigned short)v;
	s[1] = (unsigned short)(v >> 16);
	s[2] = (unsigned short)(v >> 32);
}

static std::uint64_t
seedval(const unsigned short s[3])
{
	return ((std::uint64_t)s[0]) | (((std::uint64_t)s[1]) << 16) |
	    (((std::uint64_t)s[2]) << 32);
}

/*
 * Inverse of one LCG step modulo 2^48, so that a seed can be built which makes
 * the next internal value (and therefore the exact bit pattern each function
 * consumes) whatever we want.
 */
static std::uint64_t
mult_inv48(void)
{
	std::uint64_t m = ref__rand48_mult & MASK48;
	std::uint64_t x = 1;

	for (int i = 0; i < 8; i++)
		x = x * (2 - m * x);
	return x & MASK48;
}

static std::uint64_t g_inv;

static void
seed_for(std::uint64_t target, unsigned short s[3])
{
	std::uint64_t l;

	l = ((target - ref__rand48_add) * g_inv) & MASK48;
	mkseed(l, s);
}

/* ------------------------------------------------------------------ */
/* Coverage, always measured on the ORACLE so that a planted bug in the
 * port cannot quietly shrink what the harness demands.  */

static long long cov_e_shift[48];	/* erand48: normalisation count s */
static long long cov_e_zero;		/* erand48: u.u64 == 0 early-out */
static long long cov_n_zero, cov_n_max, cov_n_mid;
static long long cov_j_neg, cov_j_pos, cov_j_zero, cov_j_m1;
static long long cov_j_intmin, cov_j_intmax;

static void
note_e_cov(std::uint64_t tmp48)
{
	if (tmp48 == 0) {
		cov_e_zero++;
		return;
	}
	int k = 47;
	while (k > 0 && !((tmp48 >> k) & 1))
		k--;
	cov_e_shift[47 - k]++;
}

/* ------------------------------------------------------------------ */

static void
fail(int idx, const char *label, const unsigned short in[3])
{
	g_stat[idx].fails++;
	if (g_stat[idx].printed++ < MAXPRINT)
		std::printf("  FAIL %-8s %-28s in=%04x,%04x,%04x\n",
		    g_stat[idx].name, label, in[0], in[1], in[2]);
}

static void
show_bufs(const unsigned short *A, const unsigned short *B)
{
	char sa[NSH * 6], sb[NSH * 6];

	hexbuf(sa, A);
	hexbuf(sb, B);
	std::printf("        port buf: %s\n        ref  buf: %s\n", sa, sb);
}

static bool
chk_n(const unsigned short in[3], const char *label)
{
	unsigned short A[NSH], B[NSH];
	long r1, r2;
	bool ok;

	fill(A, in);
	fill(B, in);
	r1 = port::nrand48(&A[SI]);
	r2 = ref_nrand48(&B[SI]);
	ok = (r1 == r2) && std::memcmp(A, B, BUFB) == 0;
	g_stat[S_N].cases++;
	if (r2 == 0)
		cov_n_zero++;
	else if (r2 == 0x7fffffffL)
		cov_n_max++;
	else
		cov_n_mid++;
	if (!ok) {
		bool print = g_stat[S_N].printed < MAXPRINT;
		fail(S_N, label, in);
		if (print) {
			std::printf("        port ret: %ld  ref ret: %ld\n",
			    r1, r2);
			show_bufs(A, B);
		}
	}
	return ok;
}

static bool
chk_j(const unsigned short in[3], const char *label)
{
	unsigned short A[NSH], B[NSH];
	long r1, r2;
	bool ok;

	fill(A, in);
	fill(B, in);
	r1 = port::jrand48(&A[SI]);
	r2 = ref_jrand48(&B[SI]);
	ok = (r1 == r2) && std::memcmp(A, B, BUFB) == 0;
	g_stat[S_J].cases++;
	if (r2 < 0)
		cov_j_neg++;
	else if (r2 > 0)
		cov_j_pos++;
	else
		cov_j_zero++;
	if (r2 == -1L)
		cov_j_m1++;
	if (r2 == (long)INT_MIN)
		cov_j_intmin++;
	if (r2 == (long)INT_MAX)
		cov_j_intmax++;
	if (!ok) {
		bool print = g_stat[S_J].printed < MAXPRINT;
		fail(S_J, label, in);
		if (print) {
			std::printf("        port ret: %ld  ref ret: %ld\n",
			    r1, r2);
			show_bufs(A, B);
		}
	}
	return ok;
}

static bool
chk_e(const unsigned short in[3], const char *label)
{
	unsigned short A[NSH], B[NSH];
	double d1, d2;
	std::uint64_t u1, u2;
	bool ok;

	fill(A, in);
	fill(B, in);
	d1 = port::erand48(&A[SI]);
	d2 = ref_erand48(&B[SI]);
	std::memcpy(&u1, &d1, sizeof u1);
	std::memcpy(&u2, &d2, sizeof u2);
	ok = (u1 == u2) && std::memcmp(A, B, BUFB) == 0;
	g_stat[S_E].cases++;
	note_e_cov(seedval(&B[SI]));
	if (!ok) {
		bool print = g_stat[S_E].printed < MAXPRINT;
		fail(S_E, label, in);
		if (print) {
			std::printf("        port ret: %.17g [%016llx]  "
			    "ref ret: %.17g [%016llx]\n", d1,
			    (unsigned long long)u1, d2,
			    (unsigned long long)u2);
			show_bufs(A, B);
		}
	}
	return ok;
}

static void
chk_all(const unsigned short in[3], const char *label)
{
	chk_n(in, label);
	chk_e(in, label);
	chk_j(in, label);
}

/* ------------------------------------------------------------------ */

static void
check_globals(void)
{
	struct {
		const char *n;
		std::uint64_t got, want;
	} v[3] = {
		{ "_rand48_seed", port::_rand48_seed, ref__rand48_seed },
		{ "_rand48_mult", port::_rand48_mult, ref__rand48_mult },
		{ "_rand48_add",  port::_rand48_add,  ref__rand48_add  },
	};

	for (int i = 0; i < 3; i++) {
		g_stat[S_GLOB].cases++;
		if (v[i].got != v[i].want) {
			g_stat[S_GLOB].fails++;
			if (g_stat[S_GLOB].printed++ < MAXPRINT)
				std::printf("  FAIL _rand48 %s: port %016llx "
				    "ref %016llx\n", v[i].n,
				    (unsigned long long)v[i].got,
				    (unsigned long long)v[i].want);
		}
	}
}

/*
 * Iterator-style use: keep calling with the same buffer until the chain is
 * exhausted, checking the return value and the complete buffer after every
 * single step so that a divergence cannot be hidden by a later step.
 */
static void
chain(std::uint64_t start, int steps)
{
	unsigned short A[NSH], B[NSH], in[3];

	for (int fn = 0; fn < 3; fn++) {
		mkseed(start, in);
		fill(A, in);
		fill(B, in);
		for (int i = 0; i < steps; i++) {
			bool ok;
			int idx;
			char label[64];

			std::sprintf(label, "chain@%d", i);
			if (fn == 0) {
				long r1 = port::nrand48(&A[SI]);
				long r2 = ref_nrand48(&B[SI]);
				idx = S_N;
				ok = (r1 == r2);
				if (r2 == 0)
					cov_n_zero++;
				else if (r2 == 0x7fffffffL)
					cov_n_max++;
				else
					cov_n_mid++;
			} else if (fn == 1) {
				double d1 = port::erand48(&A[SI]);
				double d2 = ref_erand48(&B[SI]);
				std::uint64_t u1, u2;
				std::memcpy(&u1, &d1, sizeof u1);
				std::memcpy(&u2, &d2, sizeof u2);
				idx = S_E;
				ok = (u1 == u2);
				note_e_cov(seedval(&B[SI]));
			} else {
				long r1 = port::jrand48(&A[SI]);
				long r2 = ref_jrand48(&B[SI]);
				idx = S_J;
				ok = (r1 == r2);
				if (r2 < 0)
					cov_j_neg++;
				else if (r2 > 0)
					cov_j_pos++;
				else
					cov_j_zero++;
			}
			if (std::memcmp(A, B, BUFB) != 0)
				ok = false;
			g_stat[idx].cases++;
			if (!ok) {
				bool print = g_stat[idx].printed < MAXPRINT;
				unsigned short st[3];
				mkseed(start, st);
				fail(idx, label, st);
				if (print)
					show_bufs(A, B);
				break;	/* state has diverged; stop this chain */
			}
		}
	}
}

/* ------------------------------------------------------------------ */

static std::uint64_t rng_state = 0x0123456789abcdefULL;

static std::uint64_t
rnd(void)
{
	std::uint64_t z;

	rng_state += 0x9e3779b97f4a7c15ULL;
	z = rng_state;
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return z ^ (z >> 31);
}

static std::uint64_t
gen_val(void)
{
	std::uint64_t r = rnd();
	unsigned k;

	switch ((unsigned)(r & 7)) {
	case 0:
	case 1:
	case 2:
	case 3:
		return rnd() & MASK48;
	case 4:
		k = (unsigned)(rnd() % 48);
		return 1ULL << k;
	case 5:
		k = (unsigned)(rnd() % 49);
		return (k == 48 ? MASK48 : (1ULL << k) - 1);
	case 6: {
		std::uint64_t b = rnd() & 0xff;
		return (b * 0x010101010101ULL) & MASK48;
	}
	default:
		k = (unsigned)(rnd() % 48);
		return MASK48 ^ (1ULL << k);
	}
}

/* ------------------------------------------------------------------ */

static long long
total_fails(void)
{
	long long t = 0;

	for (int i = 0; i < NSTAT; i++)
		t += g_stat[i].fails;
	return t;
}

static void
coverage_check(void)
{
	/*
	 * These are properties of the oracle's behaviour over the inputs the
	 * harness generates; if one is unmet the harness is not driving the
	 * code hard enough and must not be trusted, so it counts as a failure.
	 */
	for (int s = 0; s < 48; s++) {
		if (cov_e_shift[s] == 0) {
			std::printf("  FAIL erand48  no case with "
			    "normalisation shift s=%d\n", s);
			g_stat[S_E].fails++;
		}
	}
	struct { const char *w; long long v; int idx; } req[] = {
		{ "nrand48 result 0",          cov_n_zero,   S_N },
		{ "nrand48 result 0x7fffffff", cov_n_max,    S_N },
		{ "nrand48 result in between", cov_n_mid,    S_N },
		{ "jrand48 negative result",   cov_j_neg,    S_J },
		{ "jrand48 positive result",   cov_j_pos,    S_J },
		{ "jrand48 zero result",       cov_j_zero,   S_J },
		{ "jrand48 result -1",         cov_j_m1,     S_J },
		{ "jrand48 result INT_MIN",    cov_j_intmin, S_J },
		{ "jrand48 result INT_MAX",    cov_j_intmax, S_J },
	};
	for (unsigned i = 0; i < sizeof req / sizeof req[0]; i++) {
		if (req[i].v == 0) {
			std::printf("  FAIL coverage: %s never produced\n",
			    req[i].w);
			g_stat[req[i].idx].fails++;
		}
	}
}

static void
report(void)
{
	long long cases = 0, fails = 0;

	std::printf("\n%-18s %12s %12s\n", "function", "cases", "failures");
	std::printf("--------------------------------------------\n");
	for (int i = 0; i < NSTAT; i++) {
		std::printf("%-18s %12lld %12lld\n", g_stat[i].name,
		    g_stat[i].cases, g_stat[i].fails);
		cases += g_stat[i].cases;
		fails += g_stat[i].fails;
	}
	std::printf("--------------------------------------------\n");
	std::printf("%-18s %12lld %12lld\n", "TOTAL", cases, fails);
	std::printf("\n%s\n", fails == 0 ? "PASS" : "FAIL");
	std::fflush(stdout);
}

/* ------------------------------------------------------------------ */

int
main(void)
{
	unsigned short in[3];
	char label[64];

	g_inv = mult_inv48();
	if (((g_inv * (ref__rand48_mult & MASK48)) & MASK48) != 1) {
		std::printf("harness: modular inverse of the multiplier is "
		    "wrong\n");
		return 1;
	}

	check_globals();

	/* ---- hand-written edge cases on the input state ---- */

	static const std::uint64_t edge[] = {
		0x000000000000ULL,		/* all zero */
		0x000000000001ULL,
		0x000000000002ULL,
		0x00000000ffffULL,		/* only x[0] */
		0x0000ffff0000ULL,		/* only x[1] */
		0xffff00000000ULL,		/* only x[2] */
		0x000000008000ULL,		/* high bit of x[0] */
		0x000080000000ULL,
		0x800000000000ULL,		/* high bit of x[2] */
		0x000000007fffULL,
		0x000000008001ULL,
		0xffffffffffffULL,		/* all ones */
		0xfffffffffffeULL,
		0x7fffffffffffULL,
		0x800000000001ULL,
		0x1234abcd330eULL,		/* the RAND48 default seed */
		0x8080808080ULL,
		0x8080808080ffULL,
		0xff80ff80ff80ULL,
		0x80ff80ff80ffULL,
		0xaaaaaaaaaaaaULL,
		0x555555555555ULL,
		0x0000ffff0001ULL,
		0x00010000ffffULL,
	};
	for (unsigned i = 0; i < sizeof edge / sizeof edge[0]; i++) {
		mkseed(edge[i], in);
		std::sprintf(label, "edge-state[%u]", i);
		chk_all(in, label);
	}

	/* Every byte value, including the whole 0x80..0xff high-bit range. */
	for (unsigned b = 0; b < 256; b++) {
		std::sprintf(label, "byte-%02x-splat", b);
		mkseed(b * 0x010101010101ULL, in);
		chk_all(in, label);
		std::sprintf(label, "byte-%02x-top", b);
		mkseed((std::uint64_t)b << 40, in);
		chk_all(in, label);
		std::sprintf(label, "byte-%02x-low", b);
		mkseed(b, in);
		chk_all(in, label);
	}

	/* Single-bit and one-hole input states: exercises TOUINT48's shifts
	 * and STORERAND48's three stores at every bit position. */
	for (int k = 0; k < 48; k++) {
		std::sprintf(label, "state-bit%d", k);
		mkseed(1ULL << k, in);
		chk_all(in, label);
		std::sprintf(label, "state-hole%d", k);
		mkseed(MASK48 ^ (1ULL << k), in);
		chk_all(in, label);
		std::sprintf(label, "state-low%d", k);
		mkseed((1ULL << k) - 1, in);
		chk_all(in, label);
	}

	/* ---- targeted internal values ---- */

	/*
	 * erand48: drive the normalisation loop through every shift count
	 * s = 47 - (index of the top set bit), and both sides of each
	 * boundary of the mantissa it keeps.
	 */
	for (int k = 0; k < 48; k++) {
		std::uint64_t t[4];
		t[0] = 1ULL << k;
		t[1] = (1ULL << k) | 1ULL;
		t[2] = ((1ULL << k) << 1) - 1;	/* all ones up to bit k */
		t[3] = (1ULL << k) | (0x555555555555ULL & ((1ULL << k) - 1));
		for (int j = 0; j < 4; j++) {
			std::uint64_t v = t[j] & MASK48;
			if (v == 0)
				continue;
			seed_for(v, in);
			std::sprintf(label, "erand-top%d-%d", k, j);
			chk_e(in, label);
			chk_n(in, label);
			chk_j(in, label);
		}
	}

	/* nrand48/jrand48 boundaries, built by inverting the LCG step. */
	static const std::uint64_t tgt[] = {
		0x000000000001ULL,		/* nrand48 0, jrand48 0 */
		0x00000001ffffULL,		/* nrand48 0, jrand48 1 */
		0x000000020000ULL,		/* nrand48 1 */
		0x00000001ffffULL,
		0xfffffffe0000ULL,		/* nrand48 0x7fffffff */
		0xfffffffdffffULL,
		0xffffffffffffULL,		/* jrand48 -1 */
		0xfffffffeffffULL,
		0x800000000000ULL,		/* jrand48 INT_MIN */
		0x8000ffffffffULL,
		0x7fffffff0000ULL,		/* jrand48 INT_MAX */
		0x7fffffffffffULL,
		0x800000010000ULL,		/* INT_MIN + 1 */
		0x7ffffffe0000ULL,		/* INT_MAX - 1 */
		0x0000ffff0000ULL,
		0x000100000000ULL,
		0x0000fffeffffULL,
		0xaaaaaaaaaaaaULL,
		0x555555555555ULL,
	};
	for (unsigned i = 0; i < sizeof tgt / sizeof tgt[0]; i++) {
		seed_for(tgt[i], in);
		std::sprintf(label, "target[%u]", i);
		/*
		 * Confirm the constructed seed really does make the oracle
		 * produce the intended internal value; otherwise the targeted
		 * coverage above would be a lie.
		 */
		unsigned short v[NSH];
		fill(v, in);
		(void)ref_nrand48(&v[SI]);
		if (seedval(&v[SI]) != (tgt[i] & MASK48)) {
			std::printf("  FAIL harness: seed_for(%012llx) gave "
			    "%012llx\n", (unsigned long long)tgt[i],
			    (unsigned long long)seedval(&v[SI]));
			g_stat[S_N].fails++;
		}
		chk_all(in, label);
	}

	/* ---- iterator-style chains ---- */

	for (int i = 0; i < 24; i++) {
		std::uint64_t st = (i < 4) ? (std::uint64_t)i
		    : (i == 4 ? MASK48 : gen_val());
		chain(st, 256);
	}

	/* ---- fixed-seed random sweep: 250000 iterations per function ---- */

	const long SWEEP = 250000;
	for (long i = 0; i < SWEEP; i++) {
		std::uint64_t v = gen_val();
		mkseed(v, in);
		std::sprintf(label, "sweep-state[%ld]", i);
		chk_n(in, label);
		chk_j(in, label);
		chk_e(in, label);
	}

	/* A second sweep over constructed internal values, so the outputs of
	 * all three functions are swept uniformly as well as their inputs. */
	for (long i = 0; i < SWEEP; i++) {
		std::uint64_t t = gen_val();
		if (t == 0)
			t = 1;
		seed_for(t, in);
		std::sprintf(label, "sweep-target[%ld]", i);
		chk_n(in, label);
		chk_j(in, label);
		chk_e(in, label);
	}

	coverage_check();

	/*
	 * Last of all: the one state whose internal value is 0, i.e. the only
	 * input that takes ERAND48_END's early-out.  A planted bug that
	 * inverts that test turns the normalisation loop into an infinite
	 * loop, so everything found so far is reported first.
	 */
	if (total_fails() > 0) {
		report();
		return 1;
	}

	seed_for(0, in);
	{
		unsigned short v[NSH];
		fill(v, in);
		(void)ref_nrand48(&v[SI]);
		if (seedval(&v[SI]) != 0) {
			std::printf("  FAIL harness: seed_for(0) is wrong\n");
			g_stat[S_E].fails++;
		}
	}
	chk_n(in, "internal-zero");
	chk_j(in, "internal-zero");
	chk_e(in, "internal-zero");
	if (cov_e_zero == 0) {
		std::printf("  FAIL erand48  zero early-out never taken\n");
		g_stat[S_E].fails++;
	}

	report();
	return total_fails() == 0 ? 0 : 1;
}
