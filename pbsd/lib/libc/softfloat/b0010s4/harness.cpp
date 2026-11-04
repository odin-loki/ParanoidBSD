/*
 * harness.cpp -- differential test for PBSD batch b0010s4.
 *
 * Every ported entry point is driven against the ref_ oracle in oracle.c
 * with hand-written edge cases and a fixed-seed randomised sweep.  Both
 * the return value and the softfloat exception-flag word raised by the
 * call are compared; the flag word is the only side channel these
 * functions have (none of them writes through a pointer).
 *
 * The batch takes its operands by value, so there is no write window to
 * inspect.  To catch a port that has grown a hidden aliasing bug the
 * edge-case operands are staged in two separate buffers pre-filled with
 * the guard byte 0x7f -- one fed to the oracle, one fed to the port --
 * and both buffers are compared in their entirety after each pair of
 * calls.
 */

#include <cinttypes>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>

import pbsd.lib.libc.softfloat.b0010s4;

namespace port = pbsd::lib_libc_softfloat::b0010s4;

extern "C" {
extern int float_exception_flags;
int ref_eqsf2(std::uint32_t, std::uint32_t);
int ref_float32_eq(std::uint32_t, std::uint32_t);
}

namespace {

constexpr int MAX_REPORT = 12;
constexpr long RANDOM_ITERATIONS = 250000;

struct Stat {
	const char *name;
	long cases;
	long fails;
	int reported;
};

Stat st_eqsf2{"__eqsf2", 0, 0, 0};
Stat st_eq{"float32_eq", 0, 0, 0};

/* ------------------------------------------------------------------ */
/* guard buffers						      */
/* ------------------------------------------------------------------ */

constexpr std::size_t GUARD_SIZE = 64;
constexpr std::size_t GUARD_OFF = 16;

struct GuardBuf {
	unsigned char b[GUARD_SIZE];

	template <typename T>
	void store(const T &v, std::size_t slot)
	{
		std::memcpy(b + GUARD_OFF + slot * sizeof(T), &v, sizeof(T));
	}

	template <typename T>
	T load(std::size_t slot) const
	{
		T v;
		std::memcpy(&v, b + GUARD_OFF + slot * sizeof(T), sizeof(T));
		return v;
	}

	void fill() { std::memset(b, 0x7f, GUARD_SIZE); }
};

/* ------------------------------------------------------------------ */
/* fixed-seed PRNG (splitmix64)					      */
/* ------------------------------------------------------------------ */

std::uint64_t prng_state;

void prng_seed(std::uint64_t seed) { prng_state = seed; }

std::uint64_t prng_next()
{
	std::uint64_t z = (prng_state += 0x9E3779B97F4A7C15ULL);
	z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
	z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
	return z ^ (z >> 31);
}

std::uint32_t rand_f32()
{
	std::uint64_t r = prng_next();

	switch (r & 7) {
	case 0:
	case 1:
	case 2:
		return static_cast<std::uint32_t>(r >> 32);
	case 3: {
		/* infinity or NaN: exponent forced to all ones */
		std::uint32_t sign = static_cast<std::uint32_t>((r >> 8) & 1) << 31;
		std::uint32_t frac = static_cast<std::uint32_t>(prng_next()) & 0x7FFFFF;
		if (((r >> 9) & 3) == 0)
			frac &= 0xF;
		return sign | 0x7F800000u | frac;
	}
	case 4: {
		/* zero or subnormal */
		std::uint32_t sign = static_cast<std::uint32_t>((r >> 8) & 1) << 31;
		std::uint32_t frac = static_cast<std::uint32_t>(prng_next()) & 0x7FFFFF;
		if (((r >> 9) & 3) == 0)
			frac = 0;
		else if (((r >> 9) & 3) == 1)
			frac &= 0xFF;
		return sign | frac;
	}
	case 5: {
		/* exponent picked from the interesting boundaries */
		static const std::uint32_t exps[] = {0, 1, 2, 0x7D, 0x7E, 0x7F,
		    0x80, 0x81, 0xFD, 0xFE, 0xFF};
		std::uint64_t q = prng_next();
		std::uint32_t sign = static_cast<std::uint32_t>((r >> 8) & 1) << 31;
		std::uint32_t exp = exps[q % (sizeof(exps) / sizeof(exps[0]))];
		std::uint32_t frac = static_cast<std::uint32_t>(q >> 32) & 0x7FFFFF;
		return sign | (exp << 23) | frac;
	}
	case 6: {
		/* byte patterns drawn from 0x80..0xFF */
		std::uint32_t v = 0;
		std::uint64_t q = prng_next();
		for (int i = 0; i < 4; i++) {
			std::uint32_t byte = 0x80u | ((q >> (i * 8)) & 0x7F);
			v |= byte << (i * 8);
		}
		return v;
	}
	default: {
		/* small magnitudes and exact powers of two */
		std::uint64_t q = prng_next();
		std::uint32_t sign = static_cast<std::uint32_t>((r >> 8) & 1) << 31;
		std::uint32_t exp = 0x7Fu + static_cast<std::uint32_t>(q % 5) - 2;
		return sign | (exp << 23);
	}
	}
}

/* ------------------------------------------------------------------ */
/* edge-case operand pools					      */
/* ------------------------------------------------------------------ */

std::vector<std::uint32_t> edge_f32()
{
	std::vector<std::uint32_t> v = {
	    0x00000000u, 0x80000000u, /* +0, -0 */
	    0x00000001u, 0x80000001u, /* smallest subnormal */
	    0x007FFFFFu, 0x807FFFFFu, /* largest subnormal */
	    0x00800000u, 0x80800000u, /* smallest normal */
	    0x00800001u, 0x80800001u,
	    0x3F800000u, 0xBF800000u, /* +-1.0 */
	    0x40000000u, 0xC0000000u, /* +-2.0 */
	    0x3F7FFFFFu, 0xBF7FFFFFu,
	    0x7F7FFFFFu, 0xFF7FFFFFu, /* largest normal */
	    0x7F800000u, 0xFF800000u, /* +-inf */
	    0x7F800001u, 0xFF800001u, /* smallest signalling NaN */
	    0x7FA00000u, 0xFFA00000u, /* signalling NaN */
	    0x7FBFFFFFu, 0xFFBFFFFFu, /* largest signalling NaN */
	    0x7FC00000u, 0xFFC00000u, /* smallest quiet NaN */
	    0x7FFFFFFFu, 0xFFFFFFFFu, /* largest quiet NaN */
	    0x7FFFFFFEu, 0xFFFFFFFEu,
	    0x00000080u, 0x000000FFu, /* low-byte only */
	    0x0000FF00u, 0x00FF0000u, 0xFF000000u,
	    0x7F7F7F7Fu,		 /* guard-byte pattern */
	    0x80000080u, 0x008000FFu,
	};
	for (unsigned b = 0x80u; b <= 0xFFu; b++)
		v.push_back(b * 0x01010101u);
	return v;
}

/* ------------------------------------------------------------------ */
/* per-function checks						      */
/* ------------------------------------------------------------------ */

void note_fail(Stat &s)
{
	s.fails++;
}

bool want_report(Stat &s)
{
	return s.reported++ < MAX_REPORT;
}

void check_cmp32(Stat &s, const char *name,
    int (*fref_fn)(std::uint32_t, std::uint32_t),
    int (*fprt_fn)(std::uint32_t, std::uint32_t),
    std::uint32_t a, std::uint32_t b, bool guarded)
{
	s.cases++;

	GuardBuf ba, bb;
	std::uint32_t a1 = a, b1 = b, a2 = a, b2 = b;

	if (guarded) {
		ba.fill();
		bb.fill();
		ba.store(a, 0);
		ba.store(b, 1);
		bb.store(a, 0);
		bb.store(b, 1);
		a1 = ba.load<std::uint32_t>(0);
		b1 = ba.load<std::uint32_t>(1);
		a2 = bb.load<std::uint32_t>(0);
		b2 = bb.load<std::uint32_t>(1);
	}

	float_exception_flags = 0;
	int rref = fref_fn(a1, b1);
	int eref = float_exception_flags;

	port::float_exception_flags = 0;
	int rprt = fprt_fn(a2, b2);
	int eprt = port::float_exception_flags;

	bool bufdiff = guarded && std::memcmp(ba.b, bb.b, GUARD_SIZE) != 0;
	if (rref != rprt || eref != eprt || bufdiff) {
		note_fail(s);
		if (want_report(s))
			std::printf("  FAIL %s(0x%08" PRIx32 ", 0x%08" PRIx32
				    "): ref=%d/flags=%d port=%d/flags=%d%s\n",
			    name, a, b, rref, eref, rprt, eprt,
			    bufdiff ? " GUARD BUFFER DIFF" : "");
	}
}

int port_eqsf2(std::uint32_t a, std::uint32_t b)
{
	return port::eqsf2(a, b);
}

int port_float32_eq(std::uint32_t a, std::uint32_t b)
{
	return port::float32_eq(a, b);
}

void run_pair(std::uint32_t a, std::uint32_t b, bool guarded)
{
	check_cmp32(st_eqsf2, "__eqsf2", ref_eqsf2, port_eqsf2, a, b, guarded);
	check_cmp32(st_eq, "float32_eq", ref_float32_eq, port_float32_eq, a, b,
	    guarded);
}

void run_eqsf2()
{
	std::vector<std::uint32_t> e = edge_f32();

	for (std::uint32_t a : e)
		for (std::uint32_t b : e)
			run_pair(a, b, true);

	/* single-operand degenerate cases: both operands identical */
	for (std::uint32_t a : e)
		run_pair(a, a, true);

	/* every single-byte value in each byte position */
	for (unsigned i = 0; i < 256; i++) {
		std::uint32_t patterns[] = {
		    static_cast<std::uint32_t>(i),
		    static_cast<std::uint32_t>(i) << 8,
		    static_cast<std::uint32_t>(i) << 16,
		    static_cast<std::uint32_t>(i) << 24,
		    static_cast<std::uint32_t>(0x80u | i),
		    static_cast<std::uint32_t>((0x80u | i) << 8),
		    static_cast<std::uint32_t>((0x80u | i) << 16),
		    static_cast<std::uint32_t>((0x80u | i) << 24),
		};
		for (std::uint32_t a : patterns) {
			run_pair(a, a, true);
			run_pair(a, a ^ 0x80000000u, true);
			run_pair(a, a + 1, true);
			run_pair(a, a - 1, true);
		}
	}

	/* every single-bit and single-bit-cleared pattern */
	for (int i = 0; i < 32; i++) {
		std::uint32_t bit = 1u << i;
		run_pair(bit, bit, true);
		run_pair(bit, bit ^ 0x80000000u, true);
		run_pair(bit, ~bit, true);
		run_pair(~(1u << i), 1u << i, true);
	}

	/* named cases pinning down the special rules */
	struct Named {
		const char *what;
		std::uint32_t a;
		std::uint32_t b;
	};
	static const Named named[] = {
		{ "plus_zero_vs_minus_zero", 0x00000000u, 0x80000000u },
		{ "minus_zero_vs_plus_zero", 0x80000000u, 0x00000000u },
		{ "plus_zero_vs_plus_zero", 0x00000000u, 0x00000000u },
		{ "minus_zero_vs_minus_zero", 0x80000000u, 0x80000000u },
		{ "plus_one_vs_plus_one", 0x3F800000u, 0x3F800000u },
		{ "plus_one_vs_minus_one", 0x3F800000u, 0xBF800000u },
		{ "plus_one_vs_plus_two", 0x3F800000u, 0x40000000u },
		{ "plus_two_vs_plus_one", 0x40000000u, 0x3F800000u },
		{ "plus_inf_vs_plus_one", 0x7F800000u, 0x3F800000u },
		{ "plus_one_vs_plus_inf", 0x3F800000u, 0x7F800000u },
		{ "plus_inf_vs_plus_inf", 0x7F800000u, 0x7F800000u },
		{ "minus_inf_vs_plus_inf", 0xFF800000u, 0x7F800000u },
		{ "pseudo_inf_zero_frac", 0x7F800000u, 0x00000000u },
		{ "nan_a_only", 0x7FC00000u, 0x3F800000u },
		{ "nan_b_only", 0x3F800000u, 0x7FC00000u },
		{ "nan_both", 0x7FC00000u, 0x7FC00000u },
		{ "snan_a_only", 0x7F800001u, 0x3F800000u },
		{ "snan_b_only", 0x3F800000u, 0x7F800001u },
		{ "snan_both", 0x7F800001u, 0x7F800001u },
		{ "qnan_a_snan_b", 0x7FC00000u, 0x7F800001u },
		{ "denorm_vs_zero", 0x00000001u, 0x00000000u },
		{ "zero_vs_denorm", 0x00000000u, 0x00000001u },
		{ "neg_denorm_vs_neg_zero", 0x80000001u, 0x80000000u },
		{ "max_normal_vs_inf", 0x7F7FFFFFu, 0x7F800000u },
		{ "inf_vs_max_normal", 0x7F800000u, 0x7F7FFFFFu },
		{ "exp_boundary_7e_vs_7f", 0x3F000000u, 0x3F800000u },
		{ "exp_boundary_7f_vs_7e", 0x3F800000u, 0x3F000000u },
		{ "frac_boundary", 0x3F7FFFFFu, 0x3F800000u },
		{ "sign_diff_only", 0x40000000u, 0xC0000000u },
		{ "high_byte_0x80", 0x80000000u, 0x00000000u },
		{ "high_byte_0xff", 0xFF000000u, 0x7F000000u },
	};
	for (unsigned i = 0; i < sizeof(named) / sizeof(named[0]); ++i)
		run_pair(named[i].a, named[i].b, true);

	prng_seed(0x0010s40000000001ULL);
	for (long i = 0; i < RANDOM_ITERATIONS; i++) {
		std::uint32_t a = rand_f32();
		std::uint32_t b;
		switch (prng_next() & 7) {
		case 0:
			b = a;
			break;
		case 1:
			b = a ^ 0x80000000u;
			break;
		case 2:
			b = a + 1;
			break;
		case 3:
			b = a - 1;
			break;
		case 4:
			b = a ^ 1u;
			break;
		case 5:
			/* force all-ones exponent on one side */
			a = (a & 0x80000000u) | 0x7F800000u |
			    (a & 0x007FFFFFu);
			b = rand_f32();
			break;
		case 6:
			b = (rand_f32() & 0x80000000u) | 0x7F800000u |
			    (rand_f32() & 0x007FFFFFu);
			break;
		default:
			b = rand_f32();
			break;
		}
		run_pair(a, b, false);
	}
}

} /* namespace */

int main()
{
	std::printf("PBSD batch b0010s4 differential test\n");
	std::printf("(oracle.c is the specification; guard byte 0x7f)\n\n");

	run_eqsf2();

	const Stat *all[] = {&st_eqsf2, &st_eq};

	std::printf("\n%-16s %12s %12s %10s\n", "function", "cases",
	    "failures", "result");
	std::printf("%-16s %12s %12s %10s\n", "----------------",
	    "------------", "------------", "----------");

	long total_cases = 0;
	long total_fails = 0;
	for (const Stat *s : all) {
		total_cases += s->cases;
		total_fails += s->fails;
		std::printf("%-16s %12ld %12ld %10s\n", s->name, s->cases,
		    s->fails, s->fails == 0 ? "PASS" : "FAIL");
	}
	std::printf("%-16s %12s %12s %10s\n", "----------------",
	    "------------", "------------", "----------");
	std::printf("%-16s %12ld %12ld %10s\n", "TOTAL", total_cases,
	    total_fails, total_fails == 0 ? "PASS" : "FAIL");

	if (total_fails != 0) {
		std::printf("\nRESULT: FAIL (%ld of %ld cases diverged)\n",
		    total_fails, total_cases);
		return 1;
	}
	std::printf("\nRESULT: PASS (%ld cases, 0 divergences)\n", total_cases);
	return 0;
}
