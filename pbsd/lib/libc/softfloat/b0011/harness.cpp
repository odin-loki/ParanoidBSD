/*
 * harness.cpp -- differential test for PBSD batch b0011.
 *
 * Every ported entry point is driven against the ref_ oracle in oracle.c
 * with hand-written edge cases and a fixed-seed randomised sweep.  Both
 * the return value and the softfloat exception-flag word raised by the
 * call are compared.  Flag-returning functions store the raw char result
 * into a guard buffer so width and truncation are observable.
 *
 * Operand staging uses two separate buffers pre-filled with the guard byte
 * 0x7f -- one fed to the oracle, one fed to the port -- and both buffers
 * are compared in their entirety after each pair of calls.
 */

#include <cinttypes>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>

import pbsd.lib.libc.softfloat.b0011;

namespace port = pbsd::lib_libc_softfloat::b0011;

extern "C" {
extern int ref_float_exception_flags;
char ref_eqdf2(std::uint64_t, std::uint64_t);
std::uint64_t ref_negdf2(std::uint64_t);
char ref_gesf2(std::uint32_t, std::uint32_t);
char ref_lesf2(std::uint32_t, std::uint32_t);
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

Stat st_eqdf2{"__eqdf2", 0, 0, 0};
Stat st_negdf2{"__negdf2", 0, 0, 0};
Stat st_gesf2{"__gesf2", 0, 0, 0};
Stat st_lesf2{"__lesf2", 0, 0, 0};

/* ------------------------------------------------------------------ */
/* guard buffers						      */
/* ------------------------------------------------------------------ */

constexpr std::size_t GUARD_SIZE = 64;
constexpr std::size_t GUARD_OFF = 16;
constexpr std::size_t RET_OFF = GUARD_OFF + 32;

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
		std::uint32_t sign = static_cast<std::uint32_t>((r >> 8) & 1) << 31;
		std::uint32_t frac = static_cast<std::uint32_t>(prng_next()) & 0x7FFFFF;
		if (((r >> 9) & 3) == 0)
			frac &= 0xF;
		return sign | 0x7F800000u | frac;
	}
	case 4: {
		std::uint32_t sign = static_cast<std::uint32_t>((r >> 8) & 1) << 31;
		std::uint32_t frac = static_cast<std::uint32_t>(prng_next()) & 0x7FFFFF;
		if (((r >> 9) & 3) == 0)
			frac = 0;
		else if (((r >> 9) & 3) == 1)
			frac &= 0xFF;
		return sign | frac;
	}
	case 5: {
		static const std::uint32_t exps[] = {0, 1, 2, 0x7D, 0x7E, 0x7F,
		    0x80, 0x81, 0xFD, 0xFE, 0xFF};
		std::uint64_t q = prng_next();
		std::uint32_t sign = static_cast<std::uint32_t>((r >> 8) & 1) << 31;
		std::uint32_t exp = exps[q % (sizeof(exps) / sizeof(exps[0]))];
		std::uint32_t frac = static_cast<std::uint32_t>(q >> 32) & 0x7FFFFF;
		return sign | (exp << 23) | frac;
	}
	case 6: {
		std::uint32_t v = 0;
		std::uint64_t q = prng_next();
		for (int i = 0; i < 4; i++) {
			std::uint32_t byte = 0x80u | ((q >> (i * 8)) & 0x7F);
			v |= byte << (i * 8);
		}
		return v;
	}
	default: {
		std::uint64_t q = prng_next();
		std::uint32_t sign = static_cast<std::uint32_t>((r >> 8) & 1) << 31;
		std::uint32_t exp = 0x7Fu + static_cast<std::uint32_t>(q % 5) - 2;
		return sign | (exp << 23);
	}
	}
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

std::vector<std::uint32_t> edge_f32()
{
	std::vector<std::uint32_t> v = {
	    0x00000000u, 0x80000000u,
	    0x00000001u, 0x80000001u,
	    0x007FFFFFu, 0x807FFFFFu,
	    0x00800000u, 0x80800000u,
	    0x00800001u, 0x80800001u,
	    0x3F800000u, 0xBF800000u,
	    0x40000000u, 0xC0000000u,
	    0x3F7FFFFFu, 0xBF7FFFFFu,
	    0x7F7FFFFFu, 0xFF7FFFFFu,
	    0x7F800000u, 0xFF800000u,
	    0x7F800001u, 0xFF800001u,
	    0x7FA00000u, 0xFFA00000u,
	    0x7FBFFFFFu, 0xFFBFFFFFu,
	    0x7FC00000u, 0xFFC00000u,
	    0x7FFFFFFFu, 0xFFFFFFFFu,
	    0x7FFFFFFEu, 0xFFFFFFFEu,
	    0x00000080u, 0x000000FFu,
	    0x0000FF00u, 0x00FF0000u, 0xFF000000u,
	    0x7F7F7F7Fu,
	    0x80000080u, 0x008000FFu,
	};
	for (unsigned b = 0x80u; b <= 0xFFu; b++)
		v.push_back(b * 0x01010101u);
	return v;
}

std::vector<std::uint64_t> edge_f64()
{
	std::vector<std::uint64_t> v = {
	    0x0000000000000000ULL, 0x8000000000000000ULL,
	    0x0000000000000001ULL, 0x8000000000000001ULL,
	    0x000FFFFFFFFFFFFFULL, 0x800FFFFFFFFFFFFFULL,
	    0x0010000000000000ULL, 0x8010000000000000ULL,
	    0x0010000000000001ULL, 0x8010000000000001ULL,
	    0x3FF0000000000000ULL, 0xBFF0000000000000ULL,
	    0x4000000000000000ULL, 0xC000000000000000ULL,
	    0x3FEFFFFFFFFFFFFFULL, 0xBFEFFFFFFFFFFFFFULL,
	    0x7FEFFFFFFFFFFFFFULL, 0xFFEFFFFFFFFFFFFFULL,
	    0x7FF0000000000000ULL, 0xFFF0000000000000ULL,
	    0x7FF0000000000001ULL, 0xFFF0000000000001ULL,
	    0x7FF4000000000000ULL, 0xFFF4000000000000ULL,
	    0x7FF7FFFFFFFFFFFFULL, 0xFFF7FFFFFFFFFFFFULL,
	    0x7FF8000000000000ULL, 0xFFF8000000000000ULL,
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

void note_fail(Stat &s) { s.fails++; }

bool want_report(Stat &s) { return s.reported++ < MAX_REPORT; }

void check_negdf2(std::uint64_t a, bool guarded)
{
	Stat &s = st_negdf2;
	s.cases++;

	GuardBuf ba, bb;
	std::uint64_t ra = a, rb = a;

	if (guarded) {
		ba.fill();
		bb.fill();
		ba.store(a, 0);
		bb.store(a, 0);
		ra = ba.load<std::uint64_t>(0);
		rb = bb.load<std::uint64_t>(0);
	}

	ref_float_exception_flags = 0;
	std::uint64_t rref = ref_negdf2(ra);
	int fref = ref_float_exception_flags;

	port::float_exception_flags = 0;
	std::uint64_t rprt = port::negdf2(rb);
	int fprt = port::float_exception_flags;

	bool bad = (rref != rprt) || (fref != fprt);
	if (guarded && std::memcmp(ba.b, bb.b, GUARD_SIZE) != 0)
		bad = true;

	if (bad) {
		note_fail(s);
		if (want_report(s))
			std::printf("  FAIL __negdf2(0x%016" PRIx64 "): "
				    "ref=0x%016" PRIx64 "/flags=%d "
				    "port=0x%016" PRIx64 "/flags=%d buf=%s\n",
			    a, rref, fref, rprt, fprt,
			    guarded ? (std::memcmp(ba.b, bb.b, GUARD_SIZE) == 0
				    ? "ok" : "DIFF") : "n/a");
	}
}

void check_flag64(Stat &s, const char *name,
    char (*fref_fn)(std::uint64_t, std::uint64_t),
    port::flag (*fprt_fn)(std::uint64_t, std::uint64_t),
    std::uint64_t a, std::uint64_t b, bool guarded)
{
	s.cases++;

	GuardBuf ba, bb;
	std::uint64_t a1 = a, b1 = b, a2 = a, b2 = b;
	char rref_buf[1], rprt_buf[1];

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
	char rref = fref_fn(a1, b1);
	int fref = ref_float_exception_flags;
	std::memcpy(rref_buf, &rref, 1);

	port::float_exception_flags = 0;
	port::flag rprt = fprt_fn(a2, b2);
	int fprt = port::float_exception_flags;
	std::memcpy(rprt_buf, &rprt, 1);

	bool bufdiff = guarded && std::memcmp(ba.b, bb.b, GUARD_SIZE) != 0;
	if (rref != rprt || std::memcmp(rref_buf, rprt_buf, 1) != 0
	    || fref != fprt || bufdiff) {
		note_fail(s);
		if (want_report(s))
			std::printf("  FAIL %s(0x%016" PRIx64 ", 0x%016" PRIx64
				    "): ref=%d/flags=%d port=%d/flags=%d%s\n",
			    name, a, b, static_cast<int>(rref), fref,
			    static_cast<int>(rprt), fprt,
			    bufdiff ? " GUARD BUFFER DIFF" : "");
	}
}

void check_flag32(Stat &s, const char *name,
    char (*fref_fn)(std::uint32_t, std::uint32_t),
    port::flag (*fprt_fn)(std::uint32_t, std::uint32_t),
    std::uint32_t a, std::uint32_t b, bool guarded)
{
	s.cases++;

	GuardBuf ba, bb;
	std::uint32_t a1 = a, b1 = b, a2 = a, b2 = b;
	char rref_buf[1], rprt_buf[1];

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

	ref_float_exception_flags = 0;
	char rref = fref_fn(a1, b1);
	int fref = ref_float_exception_flags;
	std::memcpy(rref_buf, &rref, 1);

	port::float_exception_flags = 0;
	port::flag rprt = fprt_fn(a2, b2);
	int fprt = port::float_exception_flags;
	std::memcpy(rprt_buf, &rprt, 1);

	bool bufdiff = guarded && std::memcmp(ba.b, bb.b, GUARD_SIZE) != 0;
	if (rref != rprt || std::memcmp(rref_buf, rprt_buf, 1) != 0
	    || fref != fprt || bufdiff) {
		note_fail(s);
		if (want_report(s))
			std::printf("  FAIL %s(0x%08" PRIx32 ", 0x%08" PRIx32
				    "): ref=%d/flags=%d port=%d/flags=%d%s\n",
			    name, a, b, static_cast<int>(rref), fref,
			    static_cast<int>(rprt), fprt,
			    bufdiff ? " GUARD BUFFER DIFF" : "");
	}
}

port::flag port_eqdf2(std::uint64_t a, std::uint64_t b)
{
	return port::eqdf2(a, b);
}

port::flag port_gesf2(std::uint32_t a, std::uint32_t b)
{
	return port::gesf2(a, b);
}

port::flag port_lesf2(std::uint32_t a, std::uint32_t b)
{
	return port::lesf2(a, b);
}

void run_eqdf2()
{
	std::vector<std::uint64_t> e = edge_f64();

	for (std::uint64_t a : e)
		for (std::uint64_t b : e)
			check_flag64(st_eqdf2, "__eqdf2", ref_eqdf2, port_eqdf2,
			    a, b, true);

	prng_seed(0x0011000100000001ULL);
	for (long i = 0; i < RANDOM_ITERATIONS; i++) {
		std::uint64_t a = rand_f64();
		std::uint64_t b;
		switch (prng_next() & 3) {
		case 0:
			b = a;
			break;
		case 1:
			b = a ^ 0x8000000000000000ULL;
			break;
		case 2:
			b = a + 1;
			break;
		default:
			b = rand_f64();
			break;
		}
		check_flag64(st_eqdf2, "__eqdf2", ref_eqdf2, port_eqdf2, a, b,
		    false);
	}
}

void run_negdf2()
{
	std::vector<std::uint64_t> e = edge_f64();

	for (std::uint64_t a : e)
		check_negdf2(a, true);
	for (unsigned i = 0; i < 256; i++) {
		check_negdf2(static_cast<std::uint64_t>(i), true);
		check_negdf2(static_cast<std::uint64_t>(i) << 8, true);
		check_negdf2(static_cast<std::uint64_t>(i) << 16, true);
		check_negdf2(static_cast<std::uint64_t>(i) << 24, true);
		check_negdf2(static_cast<std::uint64_t>(i) << 32, true);
		check_negdf2(static_cast<std::uint64_t>(i) << 40, true);
		check_negdf2(static_cast<std::uint64_t>(i) << 48, true);
		check_negdf2(static_cast<std::uint64_t>(i) << 56, true);
	}
	for (int i = 0; i < 64; i++) {
		check_negdf2(1ULL << i, true);
		check_negdf2(~(1ULL << i), true);
	}

	prng_seed(0x0011000200000002ULL);
	for (long i = 0; i < RANDOM_ITERATIONS; i++)
		check_negdf2(rand_f64(), false);
}

void run_gesf2()
{
	std::vector<std::uint32_t> e = edge_f32();

	for (std::uint32_t a : e)
		for (std::uint32_t b : e)
			check_flag32(st_gesf2, "__gesf2", ref_gesf2, port_gesf2,
			    a, b, true);

	prng_seed(0x0011000300000003ULL);
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
		default:
			b = rand_f32();
			break;
		}
		check_flag32(st_gesf2, "__gesf2", ref_gesf2, port_gesf2, a, b,
		    false);
	}
}

void run_lesf2()
{
	std::vector<std::uint32_t> e = edge_f32();

	for (std::uint32_t a : e)
		for (std::uint32_t b : e)
			check_flag32(st_lesf2, "__lesf2", ref_lesf2, port_lesf2,
			    a, b, true);

	prng_seed(0x0011000400000004ULL);
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
		default:
			b = rand_f32();
			break;
		}
		check_flag32(st_lesf2, "__lesf2", ref_lesf2, port_lesf2, a, b,
		    false);
	}
}

} /* namespace */

int main()
{
	std::printf("PBSD batch b0011 differential test\n");
	std::printf("(oracle.c is the specification; guard byte 0x7f)\n\n");

	run_eqdf2();
	run_negdf2();
	run_gesf2();
	run_lesf2();

	const Stat *all[] = {&st_eqdf2, &st_negdf2, &st_gesf2, &st_lesf2};

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

	return total_fails == 0 ? 0 : 1;
}
