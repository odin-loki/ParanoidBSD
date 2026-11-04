/*
 * harness.cpp -- differential test for PBSD batch b0010s3.
 *
 * The ported __ltdf2 entry point is driven against the ref_ oracle in
 * oracle.c with hand-written edge cases and a fixed-seed randomised sweep.
 * Both the return value and the softfloat exception-flag word raised by the
 * call are compared; the flag word is the only side channel this function
 * has (it does not write through a pointer).
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

import pbsd.lib.libc.softfloat.b0010s3;

namespace port = pbsd::lib_libc_softfloat::b0010s3;

extern "C" {
extern int ref_float_exception_flags;
int ref_ltdf2(std::uint64_t, std::uint64_t);
}

namespace {

constexpr int MAX_REPORT = 10;
constexpr long RANDOM_ITERATIONS = 250000;

struct Stat {
	const char *name;
	long cases;
	long fails;
	int reported;
};

Stat st_ltdf2{"__ltdf2", 0, 0, 0};

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

std::uint64_t rand_f64()
{
	std::uint64_t r = prng_next();

	switch (r & 7) {
	case 0:
	case 1:
	case 2:
		return prng_next();
	case 3: {
		std::uint64_t sign = ((r >> 8) & 1) << 63;
		std::uint64_t frac = prng_next() & 0x000FFFFFFFFFFFFFULL;
		if (((r >> 9) & 3) == 0)
			frac &= 0xF;
		return sign | 0x7FF0000000000000ULL | frac;
	}
	case 4: {
		std::uint64_t sign = ((r >> 8) & 1) << 63;
		std::uint64_t frac = prng_next() & 0x000FFFFFFFFFFFFFULL;
		if (((r >> 9) & 3) == 0)
			frac = 0;
		else if (((r >> 9) & 3) == 1)
			frac &= 0xFF;
		return sign | frac;
	}
	case 5: {
		static const std::uint64_t exps[] = {0, 1, 2, 0x3FD, 0x3FE,
		    0x3FF, 0x400, 0x401, 0x7FD, 0x7FE, 0x7FF};
		std::uint64_t q = prng_next();
		std::uint64_t sign = ((r >> 8) & 1) << 63;
		std::uint64_t exp = exps[q % (sizeof(exps) / sizeof(exps[0]))];
		std::uint64_t frac = prng_next() & 0x000FFFFFFFFFFFFFULL;
		return sign | (exp << 52) | frac;
	}
	case 6: {
		std::uint64_t v = 0;
		std::uint64_t q = prng_next();
		for (int i = 0; i < 8; i++) {
			std::uint64_t byte = 0x80ULL | ((q >> (i * 8)) & 0x7F);
			v |= byte << (i * 8);
		}
		return v;
	}
	default: {
		std::uint64_t q = prng_next();
		std::uint64_t sign = ((r >> 8) & 1) << 63;
		std::uint64_t exp = 0x3FFULL + (q % 5) - 2;
		return sign | (exp << 52);
	}
	}
}

/* ------------------------------------------------------------------ */
/* edge-case operand pools					      */
/* ------------------------------------------------------------------ */

std::vector<std::uint64_t> edge_f64()
{
	std::vector<std::uint64_t> v = {
	    0x0000000000000000ULL, 0x8000000000000000ULL, /* +0, -0 */
	    0x0000000000000001ULL, 0x8000000000000001ULL, /* subnormal */
	    0x000FFFFFFFFFFFFFULL, 0x800FFFFFFFFFFFFFULL,
	    0x0010000000000000ULL, 0x8010000000000000ULL, /* smallest normal */
	    0x0010000000000001ULL, 0x8010000000000001ULL,
	    0x3FF0000000000000ULL, 0xBFF0000000000000ULL, /* +-1.0 */
	    0x4000000000000000ULL, 0xC000000000000000ULL, /* +-2.0 */
	    0x3FEFFFFFFFFFFFFFULL, 0xBFEFFFFFFFFFFFFFULL,
	    0x7FEFFFFFFFFFFFFFULL, 0xFFEFFFFFFFFFFFFFULL, /* largest normal */
	    0x7FF0000000000000ULL, 0xFFF0000000000000ULL, /* +-inf */
	    0x7FF0000000000001ULL, 0xFFF0000000000001ULL, /* signalling NaN */
	    0x7FF4000000000000ULL, 0xFFF4000000000000ULL,
	    0x7FF7FFFFFFFFFFFFULL, 0xFFF7FFFFFFFFFFFFULL, /* largest sNaN */
	    0x7FF8000000000000ULL, 0xFFF8000000000000ULL, /* smallest qNaN */
	    0x7FFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL,
	    0x7FFFFFFFFFFFFFFEULL, 0xFFFFFFFFFFFFFFFEULL,
	    0x0000000000000080ULL, 0x00000000000000FFULL,
	    0x00000000FF000000ULL, 0xFF00000000000000ULL,
	    0x7F7F7F7F7F7F7F7FULL,
	    0x8000000000000080ULL,
	};
	for (unsigned b = 0x80u; b <= 0xFFu; b++)
		v.push_back(b * 0x0101010101010101ULL);
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

void check_ltdf2(std::uint64_t a, std::uint64_t b, bool guarded)
{
	Stat &s = st_ltdf2;
	s.cases++;

	GuardBuf ba, bb;
	std::uint64_t a1 = a, b1 = b, a2 = a, b2 = b;

	if (guarded) {
		ba.fill();
		bb.fill();
		ba.store(a, 0);
		ba.store(b, 1);
		bb.store(a, 0);
		bb.store(b, 1);
		a1 = ba.load<std::uint64_t>(0);
		b1 = ba.load<std::uint64_t>(1);
		a2 = bb.load<std::uint64_t>(0);
		b2 = bb.load<std::uint64_t>(1);
	}

	ref_float_exception_flags = 0;
	int rref = ref_ltdf2(a1, b1);
	int eref = ref_float_exception_flags;

	port::float_exception_flags = 0;
	int rprt = port::ltdf2(a2, b2);
	int eprt = port::float_exception_flags;

	bool bufdiff = guarded && std::memcmp(ba.b, bb.b, GUARD_SIZE) != 0;
	if (rref != rprt || eref != eprt || bufdiff) {
		note_fail(s);
		if (want_report(s))
			std::printf("  FAIL __ltdf2(0x%016" PRIx64 ", 0x%016" PRIx64
				    "): ref=%d/flags=%d port=%d/flags=%d%s\n",
			    a, b, rref, eref, rprt, eprt,
			    bufdiff ? " GUARD BUFFER DIFF" : "");
	}
}

void run_ltdf2()
{
	std::vector<std::uint64_t> e = edge_f64();

	for (std::uint64_t a : e)
		for (std::uint64_t b : e)
			check_ltdf2(a, b, true);

	/* ordered pairs that exercise each branch of float64_lt */
	check_ltdf2(0x3FF0000000000000ULL, 0x4000000000000000ULL, true); /* a < b */
	check_ltdf2(0x4000000000000000ULL, 0x3FF0000000000000ULL, true); /* a > b */
	check_ltdf2(0x3FF0000000000000ULL, 0x3FF0000000000000ULL, true); /* equal */
	check_ltdf2(0x0000000000000000ULL, 0x8000000000000000ULL, true); /* +0, -0 */
	check_ltdf2(0x8000000000000000ULL, 0x0000000000000000ULL, true); /* -0, +0 */
	check_ltdf2(0xBFF0000000000000ULL, 0x3FF0000000000000ULL, true); /* neg < pos */
	check_ltdf2(0x3FF0000000000000ULL, 0xBFF0000000000000ULL, true); /* pos > neg */
	check_ltdf2(0xFFF0000000000000ULL, 0x7FF0000000000000ULL, true); /* -inf, +inf */
	check_ltdf2(0x7FF8000000000000ULL, 0x3FF0000000000000ULL, true); /* qNaN */
	check_ltdf2(0x7FF0000000000001ULL, 0x3FF0000000000000ULL, true); /* sNaN */

	prng_seed(0xB001003000000001ULL);
	for (long i = 0; i < RANDOM_ITERATIONS; i++) {
		std::uint64_t a = rand_f64();
		std::uint64_t b;
		switch (prng_next() & 7) {
		case 0:
			b = a;
			break;
		case 1:
			b = a ^ 0x8000000000000000ULL;
			break;
		case 2:
			b = a + 1;
			break;
		case 3:
			b = a - 1;
			break;
		case 4:
			b = a ^ 1;
			break;
		case 5:
			b = a ^ 0x0000000000000001ULL;
			break;
		default:
			b = rand_f64();
			break;
		}
		check_ltdf2(a, b, false);
	}
}

} /* namespace */

int main()
{
	std::printf("PBSD batch b0010s3 differential test\n");
	std::printf("(oracle.c is the specification; guard byte 0x7f)\n\n");

	run_ltdf2();

	std::printf("\n%-16s %12s %12s %10s\n", "function", "cases",
	    "failures", "result");
	std::printf("%-16s %12s %12s %10s\n", "----------------",
	    "------------", "------------", "----------");

	std::printf("%-16s %12ld %12ld %10s\n", st_ltdf2.name, st_ltdf2.cases,
	    st_ltdf2.fails, st_ltdf2.fails == 0 ? "PASS" : "FAIL");

	return st_ltdf2.fails == 0 ? 0 : 1;
}
