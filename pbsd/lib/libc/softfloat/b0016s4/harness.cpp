/*
 * harness.cpp -- differential test for PBSD batch b0016s4.
 */

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>

import pbsd.lib.libc.softfloat.b0016s4;

namespace port = pbsd::lib_libc_softfloat::b0016s4;

using u64 = std::uint64_t;

struct c_float128 {
	u64 high, low;
};

extern "C" {
int ref_float128_le(c_float128 a, c_float128 b);
int ref___letf2(c_float128 a, c_float128 b);
extern int __softfloat_float_exception_flags;
}

static_assert(sizeof(c_float128) == sizeof(port::float128));
static_assert(offsetof(c_float128, high) == offsetof(port::float128, high));
static_assert(offsetof(c_float128, low) == offsetof(port::float128, low));

struct Stat {
	const char *name;
	unsigned long long cases;
	unsigned long long failures;
};

static Stat stats[] = {
	{ "float128_le", 0, 0 },
	{ "__letf2", 0, 0 },
};

enum { FN_LE = 0, FN_LETF2 = 1 };

static int reported;

enum {
	COV_LE_TRUE,
	COV_LE_FALSE,
	COV_LETF2_0,
	COV_LETF2_1,
	COV_INVALID,
	COV_NO_INVALID,
	COV_SIGNS_DIFFER_NEG_A,
	COV_SIGNS_DIFFER_POS_A_ZERO,
	COV_SIGNS_DIFFER_POS_A_NZERO,
	COV_NEG_LE,
	COV_NEG_GT,
	COV_POS_LE,
	COV_POS_GT,
	COV_EQUAL,
	COV_HIGH_EQ_LOW_LT,
	COV_HIGH_EQ_LOW_EQ,
	COV_HIGH_EQ_LOW_GT,
	COV_HIGH_LT,
	COV_HIGH_GT,
	COV_ZERO_VS_NEGZERO,
	COV_INF,
	COV_MAX
};

static const char *const cov_name[COV_MAX] = {
	"float128_le == 1",
	"float128_le == 0",
	"__letf2 == 0",
	"__letf2 == 1",
	"float_flag_invalid raised",
	"no exception raised",
	"signs differ, negative a",
	"signs differ, +a, combined==0",
	"signs differ, +a, combined!=0",
	"both negative, a <= b",
	"both negative, a > b",
	"both non-negative, a <= b",
	"both non-negative, a > b",
	"operands bitwise equal",
	"high equal, low <",
	"high equal, low ==",
	"high equal, low >",
	"high word a < b",
	"high word a > b",
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

static void
report(int fn, u64 ah, u64 al, u64 bh, u64 bl, const char *what,
    long long rr, long long rp)
{
	stats[fn].failures++;
	if (reported < 25) {
		reported++;
		std::printf("FAIL %-12s a={%016llx,%016llx} b={%016llx,%016llx} "
		    "%s: oracle=%lld port=%lld\n",
		    stats[fn].name,
		    (unsigned long long)ah, (unsigned long long)al,
		    (unsigned long long)bh, (unsigned long long)bl,
		    what, rr, rp);
	}
}

static void
record_coverage(int rle, int rletf2, int fr, u64 ah, u64 al, u64 bh, u64 bl)
{
	if (rle)
		g_cov[COV_LE_TRUE]++;
	else
		g_cov[COV_LE_FALSE]++;
	if (rletf2 == 0)
		g_cov[COV_LETF2_0]++;
	else
		g_cov[COV_LETF2_1]++;
	if (fr != 0)
		g_cov[COV_INVALID]++;
	else
		g_cov[COV_NO_INVALID]++;
	if (is_inf(ah, al) || is_inf(bh, bl))
		g_cov[COV_INF]++;
	if (ah == bh && al == bl)
		g_cov[COV_EQUAL]++;
	if (is_zero(ah, al) && is_zero(bh, bl) && (ah >> 63) != (bh >> 63))
		g_cov[COV_ZERO_VS_NEGZERO]++;

	if (!is_nan(ah, al) && !is_nan(bh, bl) && (ah >> 63) != (bh >> 63)) {
		if ((ah >> 63) != 0)
			g_cov[COV_SIGNS_DIFFER_NEG_A]++;
		else {
			u64 comb = (((ah | bh) << 1) | al | bl);
			if (comb == 0)
				g_cov[COV_SIGNS_DIFFER_POS_A_ZERO]++;
			else
				g_cov[COV_SIGNS_DIFFER_POS_A_NZERO]++;
		}
	}

	if (!is_nan(ah, al) && !is_nan(bh, bl) && (ah >> 63) == (bh >> 63)) {
		if ((ah >> 63) != 0) {
			if (rle)
				g_cov[COV_NEG_LE]++;
			else
				g_cov[COV_NEG_GT]++;
		} else {
			if (rle)
				g_cov[COV_POS_LE]++;
			else
				g_cov[COV_POS_GT]++;
		}
		if (ah == bh) {
			if (al < bl)
				g_cov[COV_HIGH_EQ_LOW_LT]++;
			else if (al == bl)
				g_cov[COV_HIGH_EQ_LOW_EQ]++;
			else
				g_cov[COV_HIGH_EQ_LOW_GT]++;
		} else if (ah < bh)
			g_cov[COV_HIGH_LT]++;
		else
			g_cov[COV_HIGH_GT]++;
	}
}

static void
check(u64 ah, u64 al, u64 bh, u64 bl)
{
	static const std::size_t BUFSZ = 64;
	static const std::size_t OFF = 16;

	unsigned char bufRef[BUFSZ], bufPort[BUFSZ], pristine[BUFSZ];
	u64 words[4] = { ah, al, bh, bl };

	std::memset(bufRef, 0x7f, BUFSZ);
	std::memset(bufPort, 0x7f, BUFSZ);
	std::memset(pristine, 0x7f, BUFSZ);
	std::memcpy(bufRef + OFF, words, sizeof words);
	std::memcpy(bufPort + OFF, words, sizeof words);
	std::memcpy(pristine + OFF, words, sizeof words);

	c_float128 ra, rb;
	port::float128 pa, pb;
	std::memcpy(&ra, bufRef + OFF, sizeof ra);
	std::memcpy(&rb, bufRef + OFF + 16, sizeof rb);
	std::memcpy(&pa, bufPort + OFF, sizeof pa);
	std::memcpy(&pb, bufPort + OFF + 16, sizeof pb);

	int rle = 0, rletf2 = 0;
	int fr = 0;

	for (int fn = FN_LE; fn <= FN_LETF2; fn++) {
		stats[fn].cases++;

		__softfloat_float_exception_flags = 0;
		int rret = (fn == FN_LE) ? ref_float128_le(ra, rb)
					 : ref___letf2(ra, rb);
		int rflags = __softfloat_float_exception_flags;

		__softfloat_float_exception_flags = 0;
		int pret = (fn == FN_LE) ? port::float128_le(pa, pb)
					 : port::__letf2(pa, pb);
		int pflags = __softfloat_float_exception_flags;

		if (fn == FN_LE) {
			rle = rret;
			fr = rflags;
		} else
			rletf2 = rret;

		if (rret != pret)
			report(fn, ah, al, bh, bl, "return", rret, pret);
		else if (rflags != pflags)
			report(fn, ah, al, bh, bl, "exception flags", rflags,
			    pflags);
		else if (std::memcmp(bufRef, bufPort, BUFSZ) != 0)
			report(fn, ah, al, bh, bl, "buffers differ", 0, 0);
		else if (std::memcmp(bufRef, pristine, BUFSZ) != 0)
			report(fn, ah, al, bh, bl, "oracle buffer clobbered",
			    0, 0);
		else if (std::memcmp(bufPort, pristine, BUFSZ) != 0)
			report(fn, ah, al, bh, bl, "port buffer clobbered", 0,
			    0);
	}

	record_coverage(rle, rletf2, fr, ah, al, bh, bl);
}

static std::vector<u64>
edge_values(void)
{
	static const u64 pairs[][2] = {
		{ 0x0000000000000000ULL, 0x0000000000000000ULL },
		{ 0x8000000000000000ULL, 0x0000000000000000ULL },
		{ 0x0000000000000000ULL, 0x0000000000000001ULL },
		{ 0x8000000000000000ULL, 0x0000000000000001ULL },
		{ 0x0000000000000000ULL, 0xFFFFFFFFFFFFFFFFULL },
		{ 0x8000000000000000ULL, 0xFFFFFFFFFFFFFFFFULL },
		{ 0x0000FFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL },
		{ 0x8000FFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL },
		{ 0x0001000000000000ULL, 0x0000000000000000ULL },
		{ 0x8001000000000000ULL, 0x0000000000000000ULL },
		{ 0x3FFE000000000000ULL, 0x0000000000000000ULL },
		{ 0x3FFF000000000000ULL, 0x0000000000000000ULL },
		{ 0x3FFF000000000000ULL, 0x0000000000000001ULL },
		{ 0x3FFF000000000001ULL, 0x0000000000000000ULL },
		{ 0xBFFF000000000000ULL, 0x0000000000000000ULL },
		{ 0xBFFF000000000000ULL, 0x0000000000000001ULL },
		{ 0xBFFF000000000001ULL, 0x0000000000000000ULL },
		{ 0x4000000000000000ULL, 0x0000000000000000ULL },
		{ 0xC000000000000000ULL, 0x0000000000000000ULL },
		{ 0x7FFE000000000000ULL, 0x0000000000000000ULL },
		{ 0x7FFEFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL },
		{ 0xFFFEFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL },
		{ 0x7FFF000000000000ULL, 0x0000000000000000ULL },
		{ 0xFFFF000000000000ULL, 0x0000000000000000ULL },
		{ 0x7FFF000000000000ULL, 0x0000000000000001ULL },
		{ 0xFFFF000000000000ULL, 0x0000000000000001ULL },
		{ 0x7FFF800000000000ULL, 0x0000000000000000ULL },
		{ 0xFFFF800000000000ULL, 0x0000000000000000ULL },
		{ 0x7FFF000000000001ULL, 0x0000000000000000ULL },
		{ 0x7FFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL },
		{ 0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL },
		{ 0x7F7F7F7F7F7F7F7FULL, 0x7F7F7F7F7F7F7F7FULL },
		{ 0x8080808080808080ULL, 0x8080808080808080ULL },
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

static u64 rng_state = 0xC0FFEE123456789AULL;

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

	switch (next_u64() % 7) {
	case 0:
		*h = next_u64();
		*l = next_u64();
		break;
	case 1: {
		u64 sign = next_u64() & 1;
		u64 e = exps[next_u64() % (sizeof exps / sizeof exps[0])];
		u64 frac0 = next_u64() >> (next_u64() % 63);

		*h = (sign << 63) | (e << 48) |
		    (frac0 & 0x0000FFFFFFFFFFFFULL);
		*l = (next_u64() % 3 == 0) ? 0 :
		    next_u64() >> (next_u64() % 63);
		break;
	}
	case 2:
		*h = (next_u64() & 1) ? 0x8000000000000000ULL : 0;
		*l = 0;
		break;
	case 3: {
		u64 sign = next_u64() & 1;

		*h = sign << 63;
		*l = next_u64() % 4;
		break;
	}
	case 4: {
		u64 sign = next_u64() & 1;
		u64 frac = next_u64() % 4;

		*h = (sign << 63) | (0x7FFFULL << 48) |
		    ((frac & 1) ? 0x0000800000000000ULL : 0);
		*l = (frac >> 1);
		break;
	}
	case 5: {
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
	default: {
		u64 sign = next_u64() & 1;
		u64 eh = next_u64() & 0x7FFFULL;

		*h = (sign << 63) | (eh << 48) | (next_u64() & 0x0000FFFFFFFFFFFFULL);
		*l = next_u64();
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
		switch (next_u64() % 10) {
		case 0:
			bh = ah;
			bl = al;
			break;
		case 1:
			bh = ah;
			bl = al ^ (1ULL << (next_u64() % 64));
			break;
		case 2:
			bh = ah ^ (1ULL << (next_u64() % 64));
			bl = al;
			break;
		case 3:
			bh = ah ^ 0x8000000000000000ULL;
			bl = al;
			break;
		case 4:
			bh = ah;
			bl = al + 1;
			break;
		case 5:
			bh = ah;
			bl = (al == 0) ? 0 : al - 1;
			break;
		case 6:
			gen_value(&bh, &bl);
			bl = al;
			break;
		case 7:
			bh = ah;
			bl = al;
			if (al != 0xFFFFFFFFFFFFFFFFULL)
				bl++;
			break;
		case 8:
			bh = (ah & 0x8000000000000000ULL) |
			    ((ah + 1) & 0x7FFFFFFFFFFFFFFFULL);
			bl = al;
			break;
		default:
			gen_value(&bh, &bl);
			break;
		}
		check(ah, al, bh, bl);
	}
}

int
main(void)
{
	std::vector<u64> edges = edge_values();
	std::size_t n = edges.size() / 2;
	unsigned long long edge_pairs = 0;
	int uncovered = 0;

	std::printf("=== PBSD b0016s4 differential test "
	    "(port vs ref_ oracle) ===\n");

	for (std::size_t i = 0; i < n; i++)
		for (std::size_t j = 0; j < n; j++) {
			check(edges[2 * i], edges[2 * i + 1],
			    edges[2 * j], edges[2 * j + 1]);
			edge_pairs++;
		}

	random_sweep(250000UL);

	std::printf("\n%-16s %12s %12s\n", "function", "cases", "failures");
	std::printf("%-16s %12s %12s\n", "----------------", "------------",
	    "------------");

	unsigned long long total_failures = 0;
	for (unsigned i = 0; i < sizeof stats / sizeof stats[0]; i++) {
		std::printf("%-16s %12llu %12llu\n", stats[i].name,
		    stats[i].cases, stats[i].failures);
		total_failures += stats[i].failures;
	}
	std::printf("%-16s %12s %12llu\n", "TOTAL", "", total_failures);

	std::printf("\n(%llu hand-written edge pairs, %llu randomised pairs, "
	    "fixed seed)\n", edge_pairs, 250000ULL);

	std::printf("\n%-34s %12s\n", "input class reached", "cases");
	for (int i = 0; i < COV_MAX; i++) {
		std::printf("%-34s %12llu%s\n", cov_name[i], g_cov[i],
		    g_cov[i] == 0 ? "   <== NOT REACHED" : "");
		if (g_cov[i] == 0)
			uncovered++;
	}

	if (total_failures != 0) {
		std::printf("\nRESULT: FAIL (%llu mismatches)\n",
		    total_failures);
		return 1;
	}
	if (uncovered != 0) {
		std::printf("\nRESULT: FAIL (%d input class(es) never reached; "
		    "harness does not pin down every branch)\n", uncovered);
		return 1;
	}
	std::printf("\nRESULT: PASS\n");
	return 0;
}
