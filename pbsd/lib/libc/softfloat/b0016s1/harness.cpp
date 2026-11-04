/*
 * harness.cpp -- PBSD batch b0016s1 differential test.
 *
 * Drives the C++23 port (module pbsd.lib.libc.softfloat.b0016s1) and the C
 * reference (oracle.c, ref_ prefixed) over the same inputs and compares every
 * observable: the return value, the SoftFloat exception-flag delta produced by
 * the call, and the entire guard-filled operand buffer of each side.
 *
 * The batch's only function, __lttf2(), takes its operands by value and returns
 * an int -- it writes to no buffer, returns no pointer and holds no iterator
 * state -- so the buffer/offset/iterator comparisons degenerate to: the two
 * sides must leave both 0x7f-guarded operand buffers byte-for-byte identical to
 * each other and to the pristine copy, over their whole length, including the
 * padding on either side of the operands.
 */

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>

import pbsd.lib.libc.softfloat.b0016s1;

namespace port = pbsd::lib_libc_softfloat::b0016s1;

using u64 = std::uint64_t;

/* Layout of the oracle's float128: two 64-bit words, high first. */
struct c_float128 {
	u64 high, low;
};

extern "C" {
int ref___lttf2(c_float128 a, c_float128 b);
extern int __softfloat_float_exception_flags;
}

/* ------------------------------------------------------------------ stats */

static unsigned long long g_cases;
static unsigned long long g_failures;
static int g_reported;

/*
 * Input classes that the cases must actually reach for a flipped comparison,
 * sign or operand order in the port to be observable.
 */
enum {
	COV_RES_LT,		/* __lttf2 returned -1				*/
	COV_RES_GE,		/* __lttf2 returned 0				*/
	COV_INVALID,		/* NaN operand: float_flag_invalid raised	*/
	COV_NO_INVALID,		/* no exception raised				*/
	COV_SIGNS_DIFFER_LT,	/* aSign != bSign, a < b			*/
	COV_SIGNS_DIFFER_GE,	/* aSign != bSign, !(a < b)			*/
	COV_NEG_LT,		/* both negative, a < b				*/
	COV_NEG_GE,		/* both negative, !(a < b)			*/
	COV_POS_LT,		/* both non-negative, a < b			*/
	COV_POS_GE,		/* both non-negative, !(a < b)			*/
	COV_EQUAL,		/* bitwise identical operands			*/
	COV_HIGH_EQ_LOW_LT,	/* high words equal, low decides, a < b		*/
	COV_HIGH_EQ_LOW_GE,	/* high words equal, low decides, !(a < b)	*/
	COV_ZERO_VS_NEGZERO,	/* -0 against +0				*/
	COV_INF,		/* an infinity operand				*/
	COV_MAX
};

static const char *const cov_name[COV_MAX] = {
	"return == -1",
	"return == 0",
	"float_flag_invalid raised",
	"no exception raised",
	"signs differ, a < b",
	"signs differ, !(a < b)",
	"both negative, a < b",
	"both negative, !(a < b)",
	"both non-negative, a < b",
	"both non-negative, !(a < b)",
	"operands bitwise equal",
	"high equal, low decides, a < b",
	"high equal, low decides, !(a < b)",
	"-0 against +0",
	"infinity operand",
};

static unsigned long long g_cov[COV_MAX];

static bool
is_nan(u64 h, u64 l)
{

	return ((h >> 48) & 0x7FFF) == 0x7FFF &&
	    ((h & 0x0000FFFFFFFFFFFFULL) | l) != 0;
}

static bool
is_inf(u64 h, u64 l)
{

	return ((h >> 48) & 0x7FFF) == 0x7FFF &&
	    ((h & 0x0000FFFFFFFFFFFFULL) | l) == 0;
}

static bool
is_zero(u64 h, u64 l)
{

	return (h & 0x7FFFFFFFFFFFFFFFULL) == 0 && l == 0;
}

/* ------------------------------------------------------------- case driver */

static void
run_case(u64 ah, u64 al, u64 bh, u64 bl, const char *what)
{
	static const std::size_t BUFSZ = 64;
	static const std::size_t OFF = 16;

	unsigned char bufP[BUFSZ], bufR[BUFSZ], pristine[BUFSZ];
	u64 words[4] = { ah, al, bh, bl };

	std::memset(bufP, 0x7f, BUFSZ);
	std::memset(bufR, 0x7f, BUFSZ);
	std::memset(pristine, 0x7f, BUFSZ);
	std::memcpy(bufP + OFF, words, sizeof words);
	std::memcpy(bufR + OFF, words, sizeof words);
	std::memcpy(pristine + OFF, words, sizeof words);

	port::float128 pa, pb;
	c_float128 ra, rb;
	std::memcpy(&pa, bufP + OFF, sizeof pa);
	std::memcpy(&pb, bufP + OFF + 16, sizeof pb);
	std::memcpy(&ra, bufR + OFF, sizeof ra);
	std::memcpy(&rb, bufR + OFF + 16, sizeof rb);

	__softfloat_float_exception_flags = 0;
	int rp = port::__lttf2(pa, pb);
	int fp = __softfloat_float_exception_flags;

	__softfloat_float_exception_flags = 0;
	int rr = ref___lttf2(ra, rb);
	int fr = __softfloat_float_exception_flags;

	bool bad = false;
	const char *why = "";

	if (rp != rr) {
		bad = true;
		why = "return value";
	} else if (fp != fr) {
		bad = true;
		why = "exception flags";
	} else if (std::memcmp(bufP, bufR, BUFSZ) != 0) {
		bad = true;
		why = "operand buffers differ between port and ref";
	} else if (std::memcmp(bufP, pristine, BUFSZ) != 0) {
		bad = true;
		why = "operand buffer clobbered (guard bytes)";
	}

	g_cases++;
	if (bad) {
		g_failures++;
		if (g_reported < 15) {
			g_reported++;
			std::printf("FAIL [%s] %s: a={%016llx,%016llx} "
			    "b={%016llx,%016llx} port=(%d,flags=%d) "
			    "ref=(%d,flags=%d)\n",
			    what, why,
			    (unsigned long long)ah, (unsigned long long)al,
			    (unsigned long long)bh, (unsigned long long)bl,
			    rp, fp, rr, fr);
		}
		return;
	}

	/* Classify (from the reference result) for input-class accounting. */
	bool lt = (rr != 0);

	g_cov[lt ? COV_RES_LT : COV_RES_GE]++;
	g_cov[fr != 0 ? COV_INVALID : COV_NO_INVALID]++;
	if (is_inf(ah, al) || is_inf(bh, bl))
		g_cov[COV_INF]++;
	if (ah == bh && al == bl)
		g_cov[COV_EQUAL]++;
	if (is_zero(ah, al) && is_zero(bh, bl) && (ah >> 63) != (bh >> 63))
		g_cov[COV_ZERO_VS_NEGZERO]++;
	if (!is_nan(ah, al) && !is_nan(bh, bl)) {
		if ((ah >> 63) != (bh >> 63))
			g_cov[lt ? COV_SIGNS_DIFFER_LT :
			    COV_SIGNS_DIFFER_GE]++;
		else if ((ah >> 63) != 0)
			g_cov[lt ? COV_NEG_LT : COV_NEG_GE]++;
		else
			g_cov[lt ? COV_POS_LT : COV_POS_GE]++;
		if (ah == bh && al != bl)
			g_cov[lt ? COV_HIGH_EQ_LOW_LT :
			    COV_HIGH_EQ_LOW_GE]++;
	}
}

/* ------------------------------------------------------ hand-written edges */

static std::vector<u64>
edge_values(void)
{
	static const u64 pairs[][2] = {
		{ 0x0000000000000000ULL, 0x0000000000000000ULL }, /* +0	     */
		{ 0x8000000000000000ULL, 0x0000000000000000ULL }, /* -0	     */
		{ 0x0000000000000000ULL, 0x0000000000000001ULL }, /* +min sub */
		{ 0x8000000000000000ULL, 0x0000000000000001ULL }, /* -min sub */
		{ 0x0000000000000000ULL, 0xFFFFFFFFFFFFFFFFULL },
		{ 0x8000000000000000ULL, 0xFFFFFFFFFFFFFFFFULL },
		{ 0x0000FFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL }, /* +max sub */
		{ 0x8000FFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL }, /* -max sub */
		{ 0x0001000000000000ULL, 0x0000000000000000ULL }, /* min norm */
		{ 0x8001000000000000ULL, 0x0000000000000000ULL },
		{ 0x3FFE000000000000ULL, 0x0000000000000000ULL }, /* 0.5      */
		{ 0x3FFF000000000000ULL, 0x0000000000000000ULL }, /* 1.0      */
		{ 0x3FFF000000000000ULL, 0x0000000000000001ULL }, /* 1.0+eps  */
		{ 0x3FFF000000000001ULL, 0x0000000000000000ULL },
		{ 0xBFFF000000000000ULL, 0x0000000000000000ULL }, /* -1.0     */
		{ 0xBFFF000000000000ULL, 0x0000000000000001ULL },
		{ 0xBFFF000000000001ULL, 0x0000000000000000ULL },
		{ 0x4000000000000000ULL, 0x0000000000000000ULL }, /* 2.0      */
		{ 0xC000000000000000ULL, 0x0000000000000000ULL }, /* -2.0     */
		{ 0x7FFE000000000000ULL, 0x0000000000000000ULL },
		{ 0x7FFEFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL }, /* +max     */
		{ 0xFFFEFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL }, /* -max     */
		{ 0x7FFF000000000000ULL, 0x0000000000000000ULL }, /* +inf     */
		{ 0xFFFF000000000000ULL, 0x0000000000000000ULL }, /* -inf     */
		{ 0x7FFF000000000000ULL, 0x0000000000000001ULL }, /* +sNaN    */
		{ 0xFFFF000000000000ULL, 0x0000000000000001ULL }, /* -sNaN    */
		{ 0x7FFF800000000000ULL, 0x0000000000000000ULL }, /* +qNaN    */
		{ 0xFFFF800000000000ULL, 0x0000000000000000ULL }, /* -qNaN    */
		{ 0x7FFF000000000001ULL, 0x0000000000000000ULL },
		{ 0x7FFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL },
		{ 0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL },
		{ 0x7F7F7F7F7F7F7F7FULL, 0x7F7F7F7F7F7F7F7FULL }, /* guard    */
		{ 0x8080808080808080ULL, 0x8080808080808080ULL }, /* high bit */
		{ 0x0101010101010101ULL, 0x0101010101010101ULL },
		{ 0xFFFFFFFFFFFFFFFFULL, 0x0000000000000000ULL },
		{ 0x0000000000000000ULL, 0x8000000000000000ULL },
		{ 0x8000000000000000ULL, 0x8000000000000000ULL },
	};

	std::vector<u64> v;

	for (std::size_t i = 0; i < sizeof pairs / sizeof pairs[0]; i++) {
		v.push_back(pairs[i][0]);
		v.push_back(pairs[i][1]);
	}

	/*
	 * A single 0x80 and a single 0xFF byte at every one of the sixteen byte
	 * positions of the 128-bit operand.
	 */
	for (int pos = 0; pos < 16; pos++) {
		for (int k = 0; k < 2; k++) {
			u64 byte = (k == 0) ? 0x80ULL : 0xFFULL;
			u64 h = 0, l = 0;

			if (pos < 8)
				h = byte << (8 * pos);
			else
				l = byte << (8 * (pos - 8));
			v.push_back(h);
			v.push_back(l);
		}
	}

	return v;
}

/* ------------------------------------------------------------ random sweep */

static u64 rng_state = 0x0123456789ABCDEFULL;

static u64
next_u64(void)
{
	u64 z = (rng_state += 0x9E3779B97F4A7C15ULL);

	z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
	z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
	return z ^ (z >> 31);
}

static void
gen_value(u64 *h, u64 *l)
{
	static const u64 exps[] = {
		0x0000, 0x0001, 0x0002, 0x1234, 0x3FFE, 0x3FFF, 0x4000,
		0x7FFD, 0x7FFE, 0x7FFF, 0x7FFF, 0x7FFF
	};

	switch (next_u64() % 6) {
	case 0:				/* uniformly random 128 bits	*/
		*h = next_u64();
		*l = next_u64();
		break;
	case 1: {			/* chosen exponent, sparse frac	*/
		u64 sign = next_u64() & 1;
		u64 e = exps[next_u64() % (sizeof exps / sizeof exps[0])];
		u64 frac0 = next_u64() >> (next_u64() % 63);

		*h = (sign << 63) | (e << 48) |
		    (frac0 & 0x0000FFFFFFFFFFFFULL);
		*l = (next_u64() % 3 == 0) ? 0 :
		    next_u64() >> (next_u64() % 63);
		break;
	}
	case 2:				/* signed zero			*/
		*h = (next_u64() & 1) ? 0x8000000000000000ULL : 0;
		*l = 0;
		break;
	case 3: {			/* tiny magnitudes, random sign	*/
		u64 sign = next_u64() & 1;

		*h = sign << 63;
		*l = next_u64() % 4;
		break;
	}
	case 4: {			/* infinities and NaNs		*/
		u64 sign = next_u64() & 1;
		u64 frac = next_u64() % 4;

		*h = (sign << 63) | (0x7FFFULL << 48) |
		    ((frac & 1) ? 0x0000800000000000ULL : 0);
		*l = (frac >> 1);
		break;
	}
	default: {			/* a single byte set		*/
		int pos = (int)(next_u64() % 16);
		u64 byte = next_u64() & 0xFFULL;

		*h = 0;
		*l = 0;
		if (pos < 8)
			*h = byte << (8 * pos);
		else
			*l = byte << (8 * (pos - 8));
		break;
	}
	}
}

static void
random_sweep(unsigned long iterations)
{

	for (unsigned long i = 0; i < iterations; i++) {
		u64 ah, al, bh, bl;

		gen_value(&ah, &al);
		switch (next_u64() % 8) {
		case 0:				/* identical operands	*/
			bh = ah;
			bl = al;
			break;
		case 1:				/* one low bit flipped	*/
			bh = ah;
			bl = al ^ (1ULL << (next_u64() % 64));
			break;
		case 2:				/* one high bit flipped	*/
			bh = ah ^ (1ULL << (next_u64() % 64));
			bl = al;
			break;
		case 3:				/* sign flipped		*/
			bh = ah ^ 0x8000000000000000ULL;
			bl = al;
			break;
		case 4:				/* adjacent magnitude	*/
			bh = ah;
			bl = al + 1;
			break;
		case 5:				/* low words equal	*/
			gen_value(&bh, &bl);
			bl = al;
			break;
		default:
			gen_value(&bh, &bl);
			break;
		}
		run_case(ah, al, bh, bl, "random");
	}
}

/* ------------------------------------------------------------------- main */

int
main(void)
{
	std::vector<u64> edges = edge_values();
	std::size_t n = edges.size() / 2;
	unsigned long long edge_cases;
	int uncovered = 0;

	std::printf("=== PBSD b0016s1 differential test "
	    "(port vs ref_ oracle) ===\n");

	/* Every hand-written value against every other, both orders. */
	for (std::size_t i = 0; i < n; i++)
		for (std::size_t j = 0; j < n; j++)
			run_case(edges[2 * i], edges[2 * i + 1],
			    edges[2 * j], edges[2 * j + 1], "edge");
	edge_cases = g_cases;

	random_sweep(300000UL);

	std::printf("\n%-24s %12s %12s\n", "function", "cases", "failures");
	std::printf("%-24s %12llu %12llu\n", "__lttf2", g_cases, g_failures);
	std::printf("\n(%llu hand-written edge cases, %llu randomised cases, "
	    "fixed seed)\n", edge_cases, g_cases - edge_cases);

	std::printf("\n%-34s %12s\n", "input class reached", "cases");
	for (int i = 0; i < COV_MAX; i++) {
		std::printf("%-34s %12llu%s\n", cov_name[i], g_cov[i],
		    g_cov[i] == 0 ? "   <== NOT REACHED" : "");
		if (g_cov[i] == 0)
			uncovered++;
	}

	if (g_failures != 0) {
		std::printf("\nFAILED: %llu of %llu cases diverged\n",
		    g_failures, g_cases);
		return 1;
	}
	if (uncovered != 0) {
		std::printf("\nFAILED: %d input class(es) never reached; "
		    "the test does not pin down every branch\n", uncovered);
		return 1;
	}
	std::printf("\nPASSED: all %llu cases matched\n", g_cases);
	return 0;
}
