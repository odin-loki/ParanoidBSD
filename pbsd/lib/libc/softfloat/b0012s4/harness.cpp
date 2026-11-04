/*
 * PBSD batch b0012s4 differential harness.
 *
 * Drives the C++23 port in port.cppm and the untouched C oracle in oracle.c
 * over the same inputs and compares every observable: the `flag' return value
 * and the softfloat exception-flag word raised by the call.
 *
 * No function in this batch writes through a caller-supplied buffer, returns a
 * pointer, or carries iterator state, so the buffer-guard / offset-comparison
 * machinery does not apply here; the full observable surface of __gtxf2 is
 * (return value, exception flags), and both are compared on every case.
 */

#include <cstdint>
#include <cstdio>

import pbsd.lib.libc.softfloat.b0012s4;

namespace P = pbsd::lib_libc_softfloat::b0012s4;

extern "C" {

struct ref_floatx80 {
	std::uint64_t low;
	std::uint16_t high;
};

int ref___gtxf2(ref_floatx80, ref_floatx80);
int ref_floatx80_lt(ref_floatx80, ref_floatx80);
extern int float_exception_flags;

} /* extern "C" */

namespace {

struct Stats {
	const char *name;
	unsigned long long cases;
	unsigned long long failures;
	unsigned long long printed;
};

Stats st_gtxf2{ "gtxf2 (__gtxf2)", 0, 0, 0 };
Stats st_lt{ "floatx80_lt", 0, 0, 0 };

const unsigned long long kMaxPrint = 12;

void
report(Stats &s, const char *phase, std::uint16_t ah, std::uint64_t al,
    std::uint16_t bh, std::uint64_t bl, int pv, int pf, int rv, int rf)
{

	++s.failures;
	if (s.printed >= kMaxPrint)
		return;
	++s.printed;
	std::printf("FAIL %-16s [%s] a={high=0x%04X low=0x%016llX} "
	    "b={high=0x%04X low=0x%016llX} port=(%d,flags=%d) "
	    "ref=(%d,flags=%d)\n",
	    s.name, phase, (unsigned)ah, (unsigned long long)al,
	    (unsigned)bh, (unsigned long long)bl, pv, pf, rv, rf);
}

void
run_pair(const char *phase, std::uint16_t ah, std::uint64_t al,
    std::uint16_t bh, std::uint64_t bl)
{
	P::floatx80 pa{ al, ah };
	P::floatx80 pb{ bl, bh };
	ref_floatx80 ra{ al, ah };
	ref_floatx80 rb{ bl, bh };

	/* __gtxf2 */
	P::float_exception_flags = 0;
	int pv = P::gtxf2(pa, pb);
	int pf = P::float_exception_flags;

	float_exception_flags = 0;
	int rv = ref___gtxf2(ra, rb);
	int rf = float_exception_flags;

	++st_gtxf2.cases;
	if (pv != rv || pf != rf)
		report(st_gtxf2, phase, ah, al, bh, bl, pv, pf, rv, rf);

	/* floatx80_lt, the predicate __gtxf2 is defined in terms of */
	P::float_exception_flags = 0;
	pv = P::floatx80_lt(pa, pb);
	pf = P::float_exception_flags;

	float_exception_flags = 0;
	rv = ref_floatx80_lt(ra, rb);
	rf = float_exception_flags;

	++st_lt.cases;
	if (pv != rv || pf != rf)
		report(st_lt, phase, ah, al, bh, bl, pv, pf, rv, rf);
}

/*
 * Hand-picked field values.  The high half covers both signs of zero, both
 * signs of the smallest denormal exponent, the +-1.0 exponent, the largest
 * finite exponent (0x7FFE) and the all-ones exponent (0x7FFF) that selects the
 * NaN/infinity path.  The low half covers zero significands, the explicit
 * integer bit alone (which makes frac<<1 zero, i.e. infinity rather than NaN),
 * the integer bit plus one (the smallest signalling NaN payload), and both
 * halves of the 64-bit ordering boundary.
 */
const std::uint16_t kHighs[] = {
	0x0000, 0x0001, 0x0002, 0x3FFE, 0x3FFF, 0x4000, 0x7FFE,
	0x7FFF, 0x8000, 0x8001, 0x8002, 0xBFFF, 0xC000, 0xFFFE, 0xFFFF
};

const std::uint64_t kLows[] = {
	0x0000000000000000ULL, 0x0000000000000001ULL, 0x0000000000000002ULL,
	0x7FFFFFFFFFFFFFFFULL, 0x8000000000000000ULL, 0x8000000000000001ULL,
	0xC000000000000000ULL, 0xFFFFFFFFFFFFFFFFULL
};

const unsigned kNumHighs = sizeof(kHighs) / sizeof(kHighs[0]);
const unsigned kNumLows = sizeof(kLows) / sizeof(kLows[0]);

std::uint64_t rng_state = 0x0123456789ABCDEFULL;

std::uint64_t
next_rand(void)
{
	std::uint64_t z;

	rng_state += 0x9E3779B97F4A7C15ULL;
	z = rng_state;
	z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
	z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
	return z ^ (z >> 31);
}

} /* anonymous namespace */

int
main(void)
{
	/*
	 * Phase 1: exhaustive cross product of the hand-picked field values.
	 * Every ordered pair of (high, low) combinations is driven through both
	 * entry points, so each comparison in the port is exercised with the
	 * operands equal, strictly below and strictly above the boundary.
	 */
	for (unsigned i = 0; i < kNumHighs; ++i) {
		for (unsigned j = 0; j < kNumLows; ++j) {
			for (unsigned k = 0; k < kNumHighs; ++k) {
				for (unsigned l = 0; l < kNumLows; ++l) {
					run_pair("edge", kHighs[i], kLows[j],
					    kHighs[k], kLows[l]);
				}
			}
		}
	}

	/*
	 * Phase 2: explicitly named cases whose expected answers pin down the
	 * special rules: -0 compares equal to +0 rather than below it, an
	 * infinity is not a NaN even though its exponent is all ones, and a
	 * NaN on either side alone raises the invalid flag.
	 */
	struct Named {
		const char *what;
		std::uint16_t ah;
		std::uint64_t al;
		std::uint16_t bh;
		std::uint64_t bl;
	};
	static const Named named[] = {
		{ "minus_zero_vs_plus_zero", 0x8000, 0x0000000000000000ULL,
		  0x0000, 0x0000000000000000ULL },
		{ "plus_zero_vs_minus_zero", 0x0000, 0x0000000000000000ULL,
		  0x8000, 0x0000000000000000ULL },
		{ "plus_zero_vs_plus_zero", 0x0000, 0x0000000000000000ULL,
		  0x0000, 0x0000000000000000ULL },
		{ "minus_zero_vs_minus_zero", 0x8000, 0x0000000000000000ULL,
		  0x8000, 0x0000000000000000ULL },
		{ "minus_one_vs_plus_one", 0xBFFF, 0x8000000000000000ULL,
		  0x3FFF, 0x8000000000000000ULL },
		{ "plus_one_vs_minus_one", 0x3FFF, 0x8000000000000000ULL,
		  0xBFFF, 0x8000000000000000ULL },
		{ "plus_one_vs_plus_one", 0x3FFF, 0x8000000000000000ULL,
		  0x3FFF, 0x8000000000000000ULL },
		{ "plus_one_vs_plus_two", 0x3FFF, 0x8000000000000000ULL,
		  0x4000, 0x8000000000000000ULL },
		{ "plus_two_vs_plus_one", 0x4000, 0x8000000000000000ULL,
		  0x3FFF, 0x8000000000000000ULL },
		{ "minus_one_vs_minus_two", 0xBFFF, 0x8000000000000000ULL,
		  0xC000, 0x8000000000000000ULL },
		{ "minus_two_vs_minus_one", 0xC000, 0x8000000000000000ULL,
		  0xBFFF, 0x8000000000000000ULL },
		{ "equal_exp_low_less", 0x3FFF, 0x8000000000000000ULL,
		  0x3FFF, 0x8000000000000001ULL },
		{ "equal_exp_low_greater", 0x3FFF, 0x8000000000000001ULL,
		  0x3FFF, 0x8000000000000000ULL },
		{ "neg_equal_exp_low_less", 0xBFFF, 0x8000000000000000ULL,
		  0xBFFF, 0x8000000000000001ULL },
		{ "neg_equal_exp_low_greater", 0xBFFF, 0x8000000000000001ULL,
		  0xBFFF, 0x8000000000000000ULL },
		{ "high_greater_low_less", 0x4000, 0x0000000000000001ULL,
		  0x3FFF, 0x0000000000000002ULL },
		{ "high_less_low_greater", 0x3FFF, 0x0000000000000002ULL,
		  0x4000, 0x0000000000000001ULL },
		{ "plus_inf_vs_plus_one", 0x7FFF, 0x8000000000000000ULL,
		  0x3FFF, 0x8000000000000000ULL },
		{ "plus_one_vs_plus_inf", 0x3FFF, 0x8000000000000000ULL,
		  0x7FFF, 0x8000000000000000ULL },
		{ "minus_inf_vs_plus_one", 0xFFFF, 0x8000000000000000ULL,
		  0x3FFF, 0x8000000000000000ULL },
		{ "plus_one_vs_minus_inf", 0x3FFF, 0x8000000000000000ULL,
		  0xFFFF, 0x8000000000000000ULL },
		{ "plus_inf_vs_minus_inf", 0x7FFF, 0x8000000000000000ULL,
		  0xFFFF, 0x8000000000000000ULL },
		{ "plus_inf_vs_plus_inf", 0x7FFF, 0x8000000000000000ULL,
		  0x7FFF, 0x8000000000000000ULL },
		{ "pseudo_inf_zero_frac", 0x7FFF, 0x0000000000000000ULL,
		  0x3FFF, 0x8000000000000000ULL },
		{ "nan_a_only", 0x7FFF, 0xC000000000000000ULL,
		  0x3FFF, 0x8000000000000000ULL },
		{ "nan_b_only", 0x3FFF, 0x8000000000000000ULL,
		  0x7FFF, 0xC000000000000000ULL },
		{ "nan_both", 0x7FFF, 0xC000000000000000ULL,
		  0x7FFF, 0xC000000000000000ULL },
		{ "snan_a_only", 0x7FFF, 0x8000000000000001ULL,
		  0x3FFF, 0x8000000000000000ULL },
		{ "snan_b_only", 0x3FFF, 0x8000000000000000ULL,
		  0x7FFF, 0x8000000000000001ULL },
		{ "nan_a_inf_b", 0xFFFF, 0xC000000000000000ULL,
		  0x7FFF, 0x8000000000000000ULL },
		{ "inf_a_nan_b", 0x7FFF, 0x8000000000000000ULL,
		  0xFFFF, 0xC000000000000000ULL },
		{ "max_exp_finite_vs_inf", 0x7FFE, 0xFFFFFFFFFFFFFFFFULL,
		  0x7FFF, 0x8000000000000000ULL },
		{ "inf_vs_max_exp_finite", 0x7FFF, 0x8000000000000000ULL,
		  0x7FFE, 0xFFFFFFFFFFFFFFFFULL },
		{ "denorm_a_vs_zero_b", 0x0000, 0x0000000000000001ULL,
		  0x0000, 0x0000000000000000ULL },
		{ "zero_a_vs_denorm_b", 0x0000, 0x0000000000000000ULL,
		  0x0000, 0x0000000000000001ULL },
		{ "neg_denorm_vs_neg_zero", 0x8000, 0x0000000000000001ULL,
		  0x8000, 0x0000000000000000ULL },
		{ "neg_zero_vs_pos_denorm", 0x8000, 0x0000000000000000ULL,
		  0x0000, 0x0000000000000001ULL },
		{ "pos_denorm_vs_neg_zero", 0x0000, 0x0000000000000001ULL,
		  0x8000, 0x0000000000000000ULL },
		{ "neg_zero_vs_neg_denorm", 0x8000, 0x0000000000000000ULL,
		  0x8000, 0x0000000000000001ULL },
		{ "sign_diff_low_only_set", 0x8000, 0x0000000000000001ULL,
		  0x0000, 0x0000000000000000ULL },
		{ "sign_diff_all_high_bits", 0xFFFE, 0x0000000000000000ULL,
		  0x7FFE, 0x0000000000000000ULL },
		{ "exp_boundary_7ffe_vs_7fff", 0x7FFE, 0x0000000000000000ULL,
		  0x7FFF, 0x0000000000000000ULL },
		{ "exp_boundary_7fff_vs_7ffe", 0x7FFF, 0x0000000000000000ULL,
		  0x7FFE, 0x0000000000000000ULL },
	};
	for (unsigned i = 0; i < sizeof(named) / sizeof(named[0]); ++i) {
		run_pair(named[i].what, named[i].ah, named[i].al,
		    named[i].bh, named[i].bl);
	}

	/*
	 * Phase 3: fixed-seed randomised sweep.  Modes bias the generator
	 * towards operands that sit exactly on the interesting boundaries --
	 * identical values, values differing only in the low word, values
	 * differing only in the sign bit, and all-ones exponents -- so that the
	 * random traffic keeps hitting both sides of every comparison rather
	 * than drowning them in unrelated bit patterns.
	 */
	const unsigned long iterations = 250000UL;
	for (unsigned long n = 0; n < iterations; ++n) {
		std::uint64_t r = next_rand();
		std::uint16_t ah = (std::uint16_t)(r & 0xFFFFULL);
		std::uint64_t al = next_rand();
		std::uint16_t bh;
		std::uint64_t bl;
		unsigned mode = (unsigned)((r >> 32) % 10U);

		switch (mode) {
		case 0:		/* wholly independent operands */
			bh = (std::uint16_t)(next_rand() & 0xFFFFULL);
			bl = next_rand();
			break;
		case 1:		/* identical operands */
			bh = ah;
			bl = al;
			break;
		case 2:		/* same high word, low word nudged */
			bh = ah;
			bl = al + (std::uint64_t)(std::int64_t)
			    ((std::int64_t)(next_rand() % 5ULL) - 2);
			break;
		case 3:		/* high word nudged, same low word */
			bh = (std::uint16_t)(ah + (unsigned)(next_rand() % 5ULL)
			    - 2U);
			bl = al;
			break;
		case 4:		/* same magnitude, opposite sign */
			bh = (std::uint16_t)(ah ^ 0x8000U);
			bl = al;
			break;
		case 5:		/* both drawn from the hand-picked tables */
			ah = kHighs[next_rand() % kNumHighs];
			al = kLows[next_rand() % kNumLows];
			bh = kHighs[next_rand() % kNumHighs];
			bl = kLows[next_rand() % kNumLows];
			break;
		case 6:		/* a has an all-ones exponent */
			ah = (std::uint16_t)((ah & 0x8000U) | 0x7FFFU);
			bh = (std::uint16_t)(next_rand() & 0xFFFFULL);
			bl = next_rand();
			break;
		case 7:		/* b has an all-ones exponent */
			bh = (std::uint16_t)((next_rand() & 0x8000ULL) |
			    0x7FFFULL);
			bl = next_rand();
			break;
		case 8:		/* both have all-ones exponents */
			ah = (std::uint16_t)((ah & 0x8000U) | 0x7FFFU);
			bh = (std::uint16_t)((next_rand() & 0x8000ULL) |
			    0x7FFFULL);
			bl = next_rand();
			/* half the time make one of them an exact infinity */
			if ((next_rand() & 1ULL) != 0)
				al = 0x8000000000000000ULL;
			if ((next_rand() & 1ULL) != 0)
				bl = 0x8000000000000000ULL;
			break;
		default:	/* sparse significands, so many ties in lt128 */
			al &= 0x8000000000000003ULL;
			bh = (std::uint16_t)(ah ^ (unsigned)(next_rand() &
			    0x8001ULL));
			bl = next_rand() & 0x8000000000000003ULL;
			break;
		}
		run_pair("random", ah, al, bh, bl);
	}

	std::printf("\n%-20s %12s %12s\n", "function", "cases", "failures");
	std::printf("%-20s %12s %12s\n", "--------------------",
	    "------------", "------------");
	std::printf("%-20s %12llu %12llu\n", st_gtxf2.name, st_gtxf2.cases,
	    st_gtxf2.failures);
	std::printf("%-20s %12llu %12llu\n", st_lt.name, st_lt.cases,
	    st_lt.failures);

	unsigned long long total_cases = st_gtxf2.cases + st_lt.cases;
	unsigned long long total_failures = st_gtxf2.failures + st_lt.failures;
	std::printf("%-20s %12llu %12llu\n", "TOTAL", total_cases,
	    total_failures);

	if (total_failures != 0) {
		std::printf("\nRESULT: FAIL (%llu of %llu cases diverged)\n",
		    total_failures, total_cases);
		return 1;
	}
	std::printf("\nRESULT: PASS (%llu cases, 0 divergences)\n",
	    total_cases);
	return 0;
}
