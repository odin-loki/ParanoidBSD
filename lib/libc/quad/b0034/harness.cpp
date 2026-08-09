/*
 * harness.cpp -- differential test for batch b0034.
 *
 * Every function in the batch is called twice, once in the C++23 port and once
 * in the unmodified C oracle, and the results are compared bit for bit.
 *
 * The batch contains no string or buffer function: __ucmpdi2, __cmpdi2 and
 * __subdi3 take two integers by value and return a value.  The buffer-guard and
 * pointer-offset protocols therefore have no pointer to apply to; instead the
 * returned quad is copied into two 32-byte buffers pre-filled with the guard
 * byte 0x7f and the ENTIRE buffers are compared, so a difference in any single
 * bit of the result (not just in its arithmetic value) fails the case.
 *
 * The "empty / single char / NUL-heavy / high-bit byte / boundary length" edge
 * classes map onto integer inputs as: zero, one, all-zero halves, halves whose
 * top bit is set (0x80000000..0xffffffff, i.e. bytes 0x80-0xff), and the
 * representable boundaries INT64_MIN/INT64_MAX/UINT64_MAX together with their
 * immediate neighbours on both sides.
 *
 * Coverage of the port's own expressions is asserted, not assumed: the input
 * generators are required to drive each comparison in each function to both
 * outcomes (and the borrow in __subdi3 to all three of borrow / no borrow /
 * exactly-equal low words, which is what separates `>' from `>=').  An empty
 * coverage bucket is itself a failure.
 */

#include <cstdint>
#include <cstdio>
#include <cstring>

import pbsd.lib.libc.quad.b0034;

namespace port = pbsd::lib_libc_quad::b0034;

extern "C" {
int ref___ucmpdi2(std::uint64_t a, std::uint64_t b);
int ref___cmpdi2(std::int64_t a, std::int64_t b);
std::int64_t ref___subdi3(std::int64_t a, std::int64_t b);
}

using u64 = std::uint64_t;
using s64 = std::int64_t;
using u32 = std::uint32_t;

/* ---------------------------------------------------------------- plumbing */

struct Stats {
	const char *name;
	unsigned long long cases;
	unsigned long long fails;
	unsigned long long reported;
};

static Stats st_ucmpdi2 = { "__ucmpdi2", 0, 0, 0 };
static Stats st_cmpdi2 = { "__cmpdi2", 0, 0, 0 };
static Stats st_subdi3 = { "__subdi3", 0, 0, 0 };

static const unsigned long long kMaxReports = 12;

/*
 * Copy a returned quad into two guard-filled buffers and compare every byte,
 * including the bytes outside the nominal 8-byte window.
 */
static bool
guarded_bytes_equal(s64 got, s64 want)
{
	unsigned char pbuf[32], obuf[32];

	std::memset(pbuf, 0x7f, sizeof(pbuf));
	std::memset(obuf, 0x7f, sizeof(obuf));
	std::memcpy(pbuf + 8, &got, sizeof(got));
	std::memcpy(obuf + 8, &want, sizeof(want));
	return std::memcmp(pbuf, obuf, sizeof(pbuf)) == 0;
}

/* ------------------------------------------------------------- coverage map */

enum {
	COV_UC_HI_LT, COV_UC_HI_GT, COV_UC_HI_EQ_LO_LT, COV_UC_HI_EQ_LO_GT,
	COV_UC_ALL_EQ,

	COV_C_HI_LT, COV_C_HI_GT, COV_C_HI_EQ_LO_LT, COV_C_HI_EQ_LO_GT,
	COV_C_ALL_EQ, COV_C_HI_SIGN_DIFFERS, COV_C_LO_TOPBIT_DIFFERS,

	COV_S_BORROW, COV_S_NO_BORROW_LO_DIFFERS, COV_S_LO_EQUAL,
	COV_S_B_ZERO, COV_S_B_NONZERO, COV_S_HI_WRAP, COV_S_RESULT_NEGATIVE,
	COV_S_RESULT_POSITIVE,

	COV_N
};

static const char *cov_name[COV_N] = {
	"ucmpdi2: hi(a) <  hi(b)",
	"ucmpdi2: hi(a) >  hi(b)",
	"ucmpdi2: hi equal, lo(a) <  lo(b)",
	"ucmpdi2: hi equal, lo(a) >  lo(b)",
	"ucmpdi2: a == b",

	"cmpdi2:  hi(a) <  hi(b) signed",
	"cmpdi2:  hi(a) >  hi(b) signed",
	"cmpdi2:  hi equal, lo(a) <  lo(b)",
	"cmpdi2:  hi equal, lo(a) >  lo(b)",
	"cmpdi2:  a == b",
	"cmpdi2:  hi words differ in sign (signed vs unsigned split)",
	"cmpdi2:  hi equal, lo top bits differ (unsigned low compare)",

	"subdi3:  borrow out of low word",
	"subdi3:  no borrow, low words differ",
	"subdi3:  low words equal after subtraction (lo(b) == 0)",
	"subdi3:  b == 0",
	"subdi3:  b != 0",
	"subdi3:  high word wraps",
	"subdi3:  result negative",
	"subdi3:  result positive",
};

static unsigned long long cov[COV_N];

static inline u32
hi32(u64 v)
{
	return (u32)(v >> 32);
}

static inline u32
lo32(u64 v)
{
	return (u32)v;
}

static void
note_coverage(u64 ua, u64 ub)
{
	u32 ah = hi32(ua), al = lo32(ua);
	u32 bh = hi32(ub), bl = lo32(ub);

	if (ah < bh)
		cov[COV_UC_HI_LT]++;
	else if (ah > bh)
		cov[COV_UC_HI_GT]++;
	else if (al < bl)
		cov[COV_UC_HI_EQ_LO_LT]++;
	else if (al > bl)
		cov[COV_UC_HI_EQ_LO_GT]++;
	else
		cov[COV_UC_ALL_EQ]++;

	std::int32_t sah = (std::int32_t)ah, sbh = (std::int32_t)bh;
	if (sah < sbh)
		cov[COV_C_HI_LT]++;
	else if (sah > sbh)
		cov[COV_C_HI_GT]++;
	else if (al < bl)
		cov[COV_C_HI_EQ_LO_LT]++;
	else if (al > bl)
		cov[COV_C_HI_EQ_LO_GT]++;
	else
		cov[COV_C_ALL_EQ]++;
	if ((sah < 0) != (sbh < 0))
		cov[COV_C_HI_SIGN_DIFFERS]++;
	if (ah == bh && ((al >> 31) != (bl >> 31)))
		cov[COV_C_LO_TOPBIT_DIFFERS]++;

	u32 dl = (u32)(al - bl);
	if (dl > al)
		cov[COV_S_BORROW]++;
	else if (dl < al)
		cov[COV_S_NO_BORROW_LO_DIFFERS]++;
	else
		cov[COV_S_LO_EQUAL]++;
	if (ub == 0)
		cov[COV_S_B_ZERO]++;
	else
		cov[COV_S_B_NONZERO]++;
	u32 dh = (u32)(ah - bh - (dl > al ? 1u : 0u));
	if (dh > ah)
		cov[COV_S_HI_WRAP]++;
	if ((s64)(ua - ub) < 0)
		cov[COV_S_RESULT_NEGATIVE]++;
	else if ((s64)(ua - ub) > 0)
		cov[COV_S_RESULT_POSITIVE]++;
}

/* ------------------------------------------------------------- the compares */

static void
check_ucmpdi2(u64 a, u64 b, const char *where)
{
	int got = port::__ucmpdi2(a, b);
	int want = ref___ucmpdi2(a, b);

	st_ucmpdi2.cases++;
	if (got != want) {
		st_ucmpdi2.fails++;
		if (st_ucmpdi2.reported++ < kMaxReports)
			std::printf("FAIL __ucmpdi2 [%s] a=0x%016llx b=0x%016llx"
			    " port=%d ref=%d\n", where,
			    (unsigned long long)a, (unsigned long long)b,
			    got, want);
	}
}

static void
check_cmpdi2(s64 a, s64 b, const char *where)
{
	int got = port::__cmpdi2(a, b);
	int want = ref___cmpdi2(a, b);

	st_cmpdi2.cases++;
	if (got != want) {
		st_cmpdi2.fails++;
		if (st_cmpdi2.reported++ < kMaxReports)
			std::printf("FAIL __cmpdi2 [%s] a=0x%016llx b=0x%016llx"
			    " port=%d ref=%d\n", where,
			    (unsigned long long)a, (unsigned long long)b,
			    got, want);
	}
}

static void
check_subdi3(s64 a, s64 b, const char *where)
{
	s64 got = port::__subdi3(a, b);
	s64 want = ref___subdi3(a, b);

	st_subdi3.cases++;
	if (got != want || !guarded_bytes_equal(got, want)) {
		st_subdi3.fails++;
		if (st_subdi3.reported++ < kMaxReports)
			std::printf("FAIL __subdi3 [%s] a=0x%016llx b=0x%016llx"
			    " port=0x%016llx ref=0x%016llx\n", where,
			    (unsigned long long)a, (unsigned long long)b,
			    (unsigned long long)got, (unsigned long long)want);
	}
}

static void
check_all(u64 a, u64 b, const char *where)
{
	note_coverage(a, b);
	check_ucmpdi2(a, b, where);
	check_cmpdi2((s64)a, (s64)b, where);
	check_subdi3((s64)a, (s64)b, where);
}

/* ------------------------------------------------------------- input corpus */

/*
 * Interesting half-words: zero, one, low positives, the byte boundaries 0x7f /
 * 0x80 / 0xff, both sides of the signed 32-bit boundary, the all-ones word, and
 * words made entirely of high-bit bytes.
 */
static const u32 halves[] = {
	0x00000000u, 0x00000001u, 0x00000002u, 0x0000007fu,
	0x00000080u, 0x000000ffu, 0x00007fffu, 0x00008000u,
	0x7ffffffeu, 0x7fffffffu, 0x80000000u, 0x80000001u,
	0xfffffffeu, 0xffffffffu, 0x7f7f7f7fu, 0x80808080u,
};
static const unsigned nhalves = sizeof(halves) / sizeof(halves[0]);

static u64 corpus[nhalves * nhalves];
static const unsigned ncorpus = nhalves * nhalves;

static void
build_corpus(void)
{
	unsigned n = 0;

	for (unsigned i = 0; i < nhalves; i++)
		for (unsigned j = 0; j < nhalves; j++)
			corpus[n++] = ((u64)halves[i] << 32) | halves[j];
}

struct Pair {
	u64 a;
	u64 b;
};

/*
 * Hand-written edge cases.  Each one is chosen so that at least one comparison
 * or arithmetic operator in the port changes the observable result if it is
 * perturbed.
 */
static const Pair edge_pairs[] = {
	{ 0x0000000000000000ull, 0x0000000000000000ull },	/* a == b, all zero */
	{ 0x0000000000000000ull, 0x0000000000000001ull },	/* borrow from zero */
	{ 0x0000000000000001ull, 0x0000000000000000ull },	/* b == 0 */
	{ 0x0000000000000001ull, 0x0000000000000001ull },
	{ 0x0000000000000001ull, 0x0000000000000002ull },
	{ 0x0000000000000002ull, 0x0000000000000001ull },
	{ 0xffffffffffffffffull, 0x0000000000000000ull },	/* -1 vs 0 */
	{ 0x0000000000000000ull, 0xffffffffffffffffull },
	{ 0xffffffffffffffffull, 0xffffffffffffffffull },
	{ 0xffffffffffffffffull, 0xfffffffffffffffeull },
	{ 0xfffffffffffffffeull, 0xffffffffffffffffull },
	{ 0x8000000000000000ull, 0x7fffffffffffffffull },	/* INT64_MIN/MAX */
	{ 0x7fffffffffffffffull, 0x8000000000000000ull },
	{ 0x8000000000000000ull, 0x0000000000000000ull },
	{ 0x0000000000000000ull, 0x8000000000000000ull },
	{ 0x8000000000000000ull, 0x0000000000000001ull },	/* MIN - 1 wraps */
	{ 0x7fffffffffffffffull, 0xffffffffffffffffull },	/* MAX + 1 wraps */
	{ 0x7fffffffffffffffull, 0x7fffffffffffffffull },
	{ 0x8000000000000001ull, 0x8000000000000000ull },
	{ 0x8000000000000000ull, 0x8000000000000001ull },
	/* high halves equal, low halves straddle the signed-32 boundary: the low
	 * compare must be unsigned. */
	{ 0x0000000080000000ull, 0x0000000000000001ull },
	{ 0x0000000000000001ull, 0x0000000080000000ull },
	{ 0x000000007fffffffull, 0x0000000080000000ull },
	{ 0x0000000080000000ull, 0x000000007fffffffull },
	{ 0xffffffff80000000ull, 0xffffffff7fffffffull },
	{ 0xffffffff7fffffffull, 0xffffffff80000000ull },
	/* low half of b is zero: separates `>' from `>=' in the borrow test. */
	{ 0x0000000100000000ull, 0x0000000100000000ull },
	{ 0x0000000200000000ull, 0x0000000100000000ull },
	{ 0x0000000100000000ull, 0x0000000200000000ull },
	{ 0xffffffff00000000ull, 0x0000000100000000ull },
	{ 0x00000001ffffffffull, 0x0000000100000000ull },
	{ 0x8000000000000000ull, 0x0000000100000000ull },
	/* borrow exactly at the low-word wrap. */
	{ 0x0000000100000000ull, 0x0000000000000001ull },
	{ 0x00000001ffffffffull, 0x00000000ffffffffull },
	{ 0x0000000000000000ull, 0x00000000ffffffffull },
	{ 0x00000000ffffffffull, 0x0000000000000001ull },
	{ 0x00000000ffffffffull, 0x00000000ffffffffull },
	{ 0x0000000000000001ull, 0x00000000ffffffffull },
	/* high halves differ in sign both ways. */
	{ 0x8000000000000001ull, 0x0000000000000001ull },
	{ 0x0000000000000001ull, 0x8000000000000001ull },
	{ 0xffffffff00000001ull, 0x0000000000000001ull },
	{ 0x0000000000000001ull, 0xffffffff00000001ull },
	/* high-bit byte patterns. */
	{ 0x8080808080808080ull, 0x7f7f7f7f7f7f7f7full },
	{ 0x7f7f7f7f7f7f7f7full, 0x8080808080808080ull },
	{ 0x80808080ffffffffull, 0x8080808000000000ull },
	{ 0xfefefefefefefefeull, 0x0101010101010101ull },
	{ 0x0101010101010101ull, 0xfefefefefefefefeull },
};
static const unsigned nedge = sizeof(edge_pairs) / sizeof(edge_pairs[0]);

/* ------------------------------------------------------------------- random */

static u64 rng_state;

static u64
next_u64(void)
{
	u64 z = (rng_state += 0x9e3779b97f4a7c15ull);

	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ull;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebull;
	return z ^ (z >> 31);
}

static u32
next_half(void)
{
	u64 r = next_u64();

	/* Half the time pick a boundary half-word, half the time a random one. */
	if ((r & 1) != 0)
		return halves[(r >> 8) % nhalves];
	return (u32)(r >> 32);
}

/* ---------------------------------------------------------------------- main */

int
main(void)
{
	unsigned long long total_fails = 0;

	build_corpus();

	/* 1. Hand-written edge cases, both argument orders. */
	for (unsigned i = 0; i < nedge; i++) {
		check_all(edge_pairs[i].a, edge_pairs[i].b, "edge");
		check_all(edge_pairs[i].b, edge_pairs[i].a, "edge-swapped");
	}

	/* 2. Exhaustive cross product of every boundary half-word combination:
	 * 256 values x 256 values, which covers every ordering of the high and
	 * low words against each other, signed and unsigned. */
	for (unsigned i = 0; i < ncorpus; i++)
		for (unsigned j = 0; j < ncorpus; j++)
			check_all(corpus[i], corpus[j], "corpus");

	/* 3. Fixed-seed randomised sweep. */
	rng_state = 0x0034b0034b0034ull;
	const unsigned long long iters = 250000;
	for (unsigned long long k = 0; k < iters; k++) {
		u64 a, b;

		switch (k % 8) {
		case 0:				/* fully random */
			a = next_u64();
			b = next_u64();
			break;
		case 1:				/* boundary-biased halves */
			a = ((u64)next_half() << 32) | next_half();
			b = ((u64)next_half() << 32) | next_half();
			break;
		case 2: {			/* b near a */
			static const s64 deltas[5] = { -2, -1, 0, 1, 2 };
			a = next_u64();
			b = (u64)((s64)a + deltas[next_u64() % 5]);
			break;
		}
		case 3:				/* shared high word */
			a = next_u64();
			b = (a & 0xffffffff00000000ull) | (next_u64() >> 32);
			break;
		case 4:				/* shared low word */
			a = next_u64();
			b = (next_u64() & 0xffffffff00000000ull) | lo32(a);
			break;
		case 5:				/* lo(b) == 0 */
			a = next_u64();
			b = next_u64() & 0xffffffff00000000ull;
			break;
		case 6:				/* lo(b) == 0xffffffff */
			a = next_u64();
			b = (next_u64() & 0xffffffff00000000ull) | 0xffffffffull;
			break;
		default:			/* sparse bit patterns */
			a = next_u64() & next_u64();
			b = next_u64() | (next_u64() >> 17);
			break;
		}
		check_all(a, b, "random");
	}

	/* ------------------------------------------------------------ report */

	std::printf("\n");
	std::printf("%-12s %14s %12s %10s\n", "function", "cases", "failures",
	    "result");
	std::printf("%-12s %14s %12s %10s\n", "------------",
	    "--------------", "------------", "----------");
	const Stats *table[3] = { &st_ucmpdi2, &st_cmpdi2, &st_subdi3 };
	for (unsigned i = 0; i < 3; i++) {
		std::printf("%-12s %14llu %12llu %10s\n", table[i]->name,
		    table[i]->cases, table[i]->fails,
		    table[i]->fails == 0 ? "PASS" : "FAIL");
		total_fails += table[i]->fails;
	}

	unsigned long long empty_buckets = 0;
	for (unsigned i = 0; i < COV_N; i++)
		if (cov[i] == 0)
			empty_buckets++;
	std::printf("\ncoverage of port expressions (each bucket must be"
	    " non-empty):\n");
	for (unsigned i = 0; i < COV_N; i++)
		std::printf("  %-58s %12llu%s\n", cov_name[i], cov[i],
		    cov[i] == 0 ? "  <== NOT COVERED" : "");

	std::printf("\ntotal cases: %llu   total failures: %llu   empty"
	    " coverage buckets: %llu\n",
	    st_ucmpdi2.cases + st_cmpdi2.cases + st_subdi3.cases,
	    total_fails, empty_buckets);

	if (total_fails != 0 || empty_buckets != 0) {
		std::printf("RESULT: FAIL\n");
		return 1;
	}
	std::printf("RESULT: PASS\n");
	return 0;
}
