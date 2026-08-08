/*
 * Differential harness for batch b0152 (lib/msun/src/s_tanhf.c).
 *
 * The batch contains a single pure function, float tanhf(float), which takes
 * no buffers and returns no pointers, so the buffer/guard-byte and pointer
 * offset protocols do not apply here.  What does apply is that a float result
 * carries more information than "==" exposes: -0.0 == +0.0 and NaN != NaN.
 * Every comparison below is therefore made on the raw 32-bit encoding, so a
 * port that returns -0.0 where the oracle returns +0.0, or that returns a
 * different NaN encoding, is reported as a failure.
 */

#include <cstdio>
#include <cstdint>
#include <cstring>

import pbsd.lib.msun.src.b0152;

extern "C" float ref_tanhf(float);

static uint32_t to_bits(float f)
{
	uint32_t u;
	std::memcpy(&u, &f, sizeof u);
	return u;
}

static float from_bits(uint32_t u)
{
	float f;
	std::memcpy(&f, &u, sizeof f);
	return f;
}

struct Stat {
	const char *name;
	unsigned long long cases;
	unsigned long long failures;
	unsigned long long reported;
};

static Stat st_tanhf = { "tanhf", 0, 0, 0 };

/* Exact bit-for-bit comparison of the two results for one input encoding. */
static void check_tanhf(uint32_t xb)
{
	const float x = from_bits(xb);

	const float got = pbsd::lib_msun_src::b0152::tanhf(x);
	const float want = ref_tanhf(x);

	const uint32_t gb = to_bits(got);
	const uint32_t wb = to_bits(want);

	st_tanhf.cases++;
	if (gb != wb) {
		st_tanhf.failures++;
		if (st_tanhf.reported < 20) {
			st_tanhf.reported++;
			std::printf("  FAIL tanhf: x=0x%08x (%.9g)  port=0x%08x (%.9g)"
				    "  oracle=0x%08x (%.9g)\n",
				    xb, (double)x, gb, (double)got, wb, (double)want);
		}
	}
}

/* Feed both the encoding and its sign-flipped twin. */
static void check_both_signs(uint32_t xb)
{
	check_tanhf(xb);
	check_tanhf(xb ^ 0x80000000u);
}

/* xorshift64*, fixed seed: the sweep is reproducible. */
static uint64_t rng_state = 0x9e3779b97f4a7c15ull;

static uint64_t next_rand(void)
{
	uint64_t x = rng_state;
	x ^= x >> 12;
	x ^= x << 25;
	x ^= x >> 27;
	rng_state = x;
	return x * 0x2545f4914f6cdd1dull;
}

static uint32_t rand32(void)
{
	return (uint32_t)(next_rand() >> 32);
}

/* Uniform encoding in [lo, hi]. */
static uint32_t rand_in(uint32_t lo, uint32_t hi)
{
	const uint32_t span = hi - lo;
	if (span == 0xffffffffu)
		return rand32();
	return lo + (uint32_t)(next_rand() % ((uint64_t)span + 1));
}

/*
 * The four thresholds the port branches on, as float encodings:
 *   0x39800000  2**-12   (below it: return x directly)
 *   0x3f800000  1.0      (at or above: the expm1f(+2|x|) arm)
 *   0x41100000  9.0      (at or above: the constant +-(1 - tiny) arm)
 *   0x7f800000  Inf      (at or above: the Inf/NaN arm)
 */
static const uint32_t boundaries[] = {
	0x00000000u,	/* zero / subnormal boundary */
	0x00800000u,	/* smallest normal */
	0x39800000u,
	0x3f800000u,
	0x41100000u,
	0x7f800000u,
	0x7f7fffffu,	/* largest finite */
};

static void hand_written_cases(void)
{
	/* Zeroes, subnormals, and the exact threshold encodings with their
	   immediate neighbours on both sides. */
	static const uint32_t explicit_bits[] = {
		0x00000000u,		/* +0 */
		0x00000001u,		/* smallest subnormal */
		0x00000002u,
		0x00400000u,
		0x007fffffu,		/* largest subnormal */
		0x00800000u,		/* smallest normal */
		0x00800001u,
		0x1f000000u,
		0x2f000000u,
		0x33800000u,
		0x387fffffu,
		0x38800000u,		/* 2**-14 */
		0x397fffffu,		/* just below 2**-12 */
		0x39800000u,		/* 2**-12 exactly */
		0x39800001u,		/* just above 2**-12 */
		0x3a800000u,
		0x3c000000u,		/* 2**-7 */
		0x3d000000u,
		0x3e000000u,		/* 0.125 */
		0x3f000000u,		/* 0.5 */
		0x3f400000u,		/* 0.75 */
		0x3f7ffffeu,
		0x3f7fffffu,		/* just below 1.0 */
		0x3f800000u,		/* 1.0 exactly */
		0x3f800001u,		/* just above 1.0 */
		0x3fc00000u,		/* 1.5 */
		0x40000000u,		/* 2.0 */
		0x40490fdbu,		/* pi */
		0x40800000u,		/* 4.0 */
		0x40e00000u,		/* 7.0 */
		0x410ffffeu,
		0x410fffffu,		/* just below 9.0 */
		0x41100000u,		/* 9.0 exactly */
		0x41100001u,		/* just above 9.0 */
		0x41200000u,		/* 10.0 */
		0x42c80000u,		/* 100.0 */
		0x4b000000u,		/* 2**23 */
		0x7f7ffffeu,
		0x7f7fffffu,		/* FLT_MAX */
		0x7f800000u,		/* +Inf */
		0x7f800001u,		/* smallest signalling-encoding NaN */
		0x7fbfffffu,
		0x7fc00000u,		/* default quiet NaN */
		0x7fc00001u,
		0x7fffffffu,		/* NaN, all payload bits set */
	};

	for (unsigned i = 0; i < sizeof explicit_bits / sizeof explicit_bits[0]; i++)
		check_both_signs(explicit_bits[i]);

	/* Dense neighbourhoods: every encoding within 512 ulps of each
	   threshold, both signs.  A comparison that is flipped from < to <=
	   (or the reverse) only changes behaviour for the single encoding
	   sitting exactly on the threshold, so the threshold itself and both
	   of its neighbours must be exercised. */
	for (unsigned b = 0; b < sizeof boundaries / sizeof boundaries[0]; b++) {
		const uint32_t base = boundaries[b];
		for (int32_t d = -512; d <= 512; d++) {
			const int64_t v = (int64_t)base + d;
			if (v < 0 || v > 0x7fffffff)
				continue;
			check_both_signs((uint32_t)v);
		}
	}

	/* A walk through every binade: one encoding per exponent, plus a few
	   mantissa positions, so no exponent range goes untested. */
	for (uint32_t e = 0; e < 256; e++) {
		static const uint32_t mant[] = {
			0x000000u, 0x000001u, 0x155555u,
			0x400000u, 0x7fffffu,
		};
		for (unsigned m = 0; m < sizeof mant / sizeof mant[0]; m++)
			check_both_signs((e << 23) | mant[m]);
	}
}

static void random_sweep(void)
{
	/* 1. Whole encoding space, including NaNs and infinities. */
	for (int i = 0; i < 400000; i++)
		check_tanhf(rand32());

	/* 2. |x| < 2**-12: the early-return arm. */
	for (int i = 0; i < 120000; i++)
		check_both_signs(rand_in(0x00000000u, 0x397fffffu));

	/* 3. 2**-12 <= |x| < 1: the expm1f(-2|x|) arm. */
	for (int i = 0; i < 120000; i++)
		check_both_signs(rand_in(0x39800000u, 0x3f7fffffu));

	/* 4. 1 <= |x| < 9: the expm1f(+2|x|) arm. */
	for (int i = 0; i < 120000; i++)
		check_both_signs(rand_in(0x3f800000u, 0x410fffffu));

	/* 5. 9 <= |x| < Inf: the constant arm. */
	for (int i = 0; i < 120000; i++)
		check_both_signs(rand_in(0x41100000u, 0x7f7fffffu));

	/* 6. NaN encodings, both signs. */
	for (int i = 0; i < 60000; i++)
		check_both_signs(rand_in(0x7f800001u, 0x7fffffffu));

	/* 7. Small perturbations of the thresholds: a mutated comparison or a
	   +/- swap in the threshold arithmetic shows up first right here. */
	for (int i = 0; i < 120000; i++) {
		const uint32_t base =
		    boundaries[rand32() % (sizeof boundaries / sizeof boundaries[0])];
		const int32_t d = (int32_t)(rand32() % 4097) - 2048;
		const int64_t v = (int64_t)base + d;
		if (v < 0 || v > 0x7fffffff)
			continue;
		check_both_signs((uint32_t)v);
	}
}

int main(void)
{
	hand_written_cases();
	random_sweep();

	std::printf("\n");
	std::printf("%-24s %14s %12s %8s\n", "function", "cases", "failures", "result");
	std::printf("%-24s %14s %12s %8s\n", "------------------------",
		    "--------------", "------------", "--------");
	std::printf("%-24s %14llu %12llu %8s\n", st_tanhf.name, st_tanhf.cases,
		    st_tanhf.failures, st_tanhf.failures == 0 ? "PASS" : "FAIL");
	std::printf("\n");

	const unsigned long long total_failures = st_tanhf.failures;
	if (total_failures != 0) {
		std::printf("b0152: %llu mismatch(es) between port and oracle\n",
			    total_failures);
		return 1;
	}
	std::printf("b0152: all %llu cases matched bit-for-bit\n", st_tanhf.cases);
	return 0;
}
