/*
 * harness.cpp -- differential test for PBSD batch b0012s3.
 *
 * Compares the C++23 port in port.cppm against the unmodified C reference in
 * oracle.c for every function in the batch and the SoftFloat primitives it
 * is built out of:
 *
 *   negxf2      (== __negxf2 of negxf2.c)
 *   mulxf3      (== __mulxf3 / floatx80_mul)
 *   floatsixf   (== __floatsixf / int32_to_floatx80)
 *
 * Operands and results are materialised inside separate 0x7f-filled buffers
 * (one for the port, one for the oracle) so any write past a nominal window
 * is caught.  Return values and the softfloat exception-flag word are
 * compared on every case.
 */

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

import pbsd.lib.libc.softfloat.b0012s3;

namespace P = pbsd::lib_libc_softfloat::b0012s3;

extern "C" {

struct ref_floatx80 {
	std::uint16_t high;
	std::uint64_t low;
};

ref_floatx80 ref___negxf2(ref_floatx80 a);
ref_floatx80 floatx80_mul(ref_floatx80 a, ref_floatx80 b);
ref_floatx80 int32_to_floatx80(std::int32_t a);
extern int float_exception_flags;

} /* extern "C" */

namespace {

enum { GUARD = 0x7f };
enum { BUFSZ = 64 };
enum { OFF_IN = 8 };
enum { OFF_OUT = 32 };
enum { OFF_A = OFF_IN };
enum { OFF_B = OFF_IN + 16 };

static_assert(sizeof(P::floatx80) == sizeof(ref_floatx80), "floatx80 layout");
static_assert(sizeof(P::floatx80) == 16, "floatx80 size");

struct Val {
	std::uint16_t high;
	std::uint64_t low;
};

struct Stats {
	const char *name;
	unsigned long long cases;
	unsigned long long failures;
	unsigned long long printed;
};

Stats st_neg{ "negxf2 (__negxf2)", 0, 0, 0 };
Stats st_mul{ "mulxf3 (__mulxf3)", 0, 0, 0 };
Stats st_six{ "floatsixf", 0, 0, 0 };

const unsigned long long kMaxPrint = 12;

void
report(Stats &s, const char *phase, const char *field,
    std::uint16_t ah, std::uint64_t al, std::uint16_t bh, std::uint64_t bl,
    std::int32_t si, std::uint16_t rh, std::uint64_t rl, int pf, int rf)
{
	if (s.printed >= kMaxPrint)
		return;
	++s.printed;
	std::printf("FAIL %-16s [%s] %s a={0x%04X,0x%016llX} b={0x%04X,0x%016llX} "
	    "si=%d port={0x%04X,0x%016llX,flags=%d} ref={0x%04X,0x%016llX,flags=%d}\n",
	    s.name, phase, field, (unsigned)ah, (unsigned long long)al,
	    (unsigned)bh, (unsigned long long)bl, (int)si,
	    (unsigned)rh, (unsigned long long)rl, pf, (unsigned)rh,
	    (unsigned long long)rl, rf);
}

void
plant_x80(unsigned char *buf, std::size_t off, Val v)
{
	std::memcpy(buf + off, &v, sizeof v);
}

Val
load_x80(const unsigned char *buf, std::size_t off)
{
	Val v;
	std::memcpy(&v, buf + off, sizeof v);
	return v;
}

bool
bufs_equal(const unsigned char *a, const unsigned char *b)
{
	return std::memcmp(a, b, BUFSZ) == 0;
}

void
check_neg(const char *phase, Val a)
{
	unsigned char bufRef[BUFSZ], bufPort[BUFSZ], pristine[BUFSZ];

	std::memset(bufRef, GUARD, BUFSZ);
	std::memset(bufPort, GUARD, BUFSZ);
	plant_x80(bufRef, OFF_IN, a);
	plant_x80(bufPort, OFF_IN, a);
	std::memcpy(pristine, bufRef, BUFSZ);

	ref_floatx80 ra;
	P::floatx80 pa;
	std::memcpy(&ra, bufRef + OFF_IN, sizeof ra);
	std::memcpy(&pa, bufPort + OFF_IN, sizeof pa);

	float_exception_flags = 0;
	ref_floatx80 rr = ref___negxf2(ra);
	int rfl = float_exception_flags;

	P::float_exception_flags = 0;
	P::floatx80 pr = P::negxf2(pa);
	int pfl = P::float_exception_flags;

	std::memcpy(bufRef + OFF_OUT, &rr, sizeof rr);
	std::memcpy(bufPort + OFF_OUT, &pr, sizeof pr);

	++st_neg.cases;
	Val rv{ rr.high, rr.low };
	Val pv{ pr.high, pr.low };
	bool bad = rv.high != pv.high || rv.low != pv.low || rfl != pfl ||
	    !bufs_equal(bufRef, bufPort) ||
	    std::memcmp(bufRef, pristine, OFF_OUT) != 0 ||
	    std::memcmp(bufPort, pristine, OFF_OUT) != 0;
	if (bad) {
		report(st_neg, phase, "result", a.high, a.low, 0, 0, 0,
		    pv.high, pv.low, pfl, rfl);
		++st_neg.failures;
	}
}

void
check_mul(const char *phase, Val a, Val b)
{
	unsigned char bufRef[BUFSZ], bufPort[BUFSZ], pristine[BUFSZ];

	std::memset(bufRef, GUARD, BUFSZ);
	std::memset(bufPort, GUARD, BUFSZ);
	plant_x80(bufRef, OFF_A, a);
	plant_x80(bufRef, OFF_B, b);
	plant_x80(bufPort, OFF_A, a);
	plant_x80(bufPort, OFF_B, b);
	std::memcpy(pristine, bufRef, BUFSZ);

	ref_floatx80 ra, rb;
	P::floatx80 pa, pb;
	std::memcpy(&ra, bufRef + OFF_A, sizeof ra);
	std::memcpy(&rb, bufRef + OFF_B, sizeof rb);
	std::memcpy(&pa, bufPort + OFF_A, sizeof pa);
	std::memcpy(&pb, bufPort + OFF_B, sizeof pb);

	float_exception_flags = 0;
	ref_floatx80 rr = floatx80_mul(ra, rb);
	int rfl = float_exception_flags;

	P::float_exception_flags = 0;
	P::floatx80 pr = P::mulxf3(pa, pb);
	int pfl = P::float_exception_flags;

	std::memcpy(bufRef + OFF_OUT, &rr, sizeof rr);
	std::memcpy(bufPort + OFF_OUT, &pr, sizeof pr);

	++st_mul.cases;
	Val rv{ rr.high, rr.low };
	Val pv{ pr.high, pr.low };
	if (rv.high != pv.high || rv.low != pv.low || rfl != pfl ||
	    !bufs_equal(bufRef, bufPort) ||
	    std::memcmp(bufRef, pristine, OFF_B + sizeof(ref_floatx80)) != 0 ||
	    std::memcmp(bufPort, pristine, OFF_B + sizeof(ref_floatx80)) != 0)
		++st_mul.failures;
}

void
check_six(const char *phase, std::int32_t si)
{
	unsigned char bufRef[BUFSZ], bufPort[BUFSZ], pristine[BUFSZ];

	std::memset(bufRef, GUARD, BUFSZ);
	std::memset(bufPort, GUARD, BUFSZ);
	std::memcpy(bufRef + OFF_IN, &si, sizeof si);
	std::memcpy(bufPort + OFF_IN, &si, sizeof si);
	std::memcpy(pristine, bufRef, BUFSZ);

	float_exception_flags = 0;
	ref_floatx80 rr = int32_to_floatx80(si);
	int rfl = float_exception_flags;

	P::float_exception_flags = 0;
	P::floatx80 pr = P::floatsixf(si);
	int pfl = P::float_exception_flags;

	std::memcpy(bufRef + OFF_OUT, &rr, sizeof rr);
	std::memcpy(bufPort + OFF_OUT, &pr, sizeof pr);

	++st_six.cases;
	Val rv{ rr.high, rr.low };
	Val pv{ pr.high, pr.low };
	if (rv.high != pv.high || rv.low != pv.low || rfl != pfl ||
	    !bufs_equal(bufRef, bufPort) ||
	    std::memcmp(bufRef, pristine, OFF_IN + sizeof si) != 0 ||
	    std::memcmp(bufPort, pristine, OFF_IN + sizeof si) != 0)
		++st_six.failures;
}

static const Val pool[] = {
	{ 0x0000, 0x0000000000000000ULL },
	{ 0x8000, 0x0000000000000000ULL },
	{ 0x0000, 0x0000000000000001ULL },
	{ 0x8000, 0x0000000000000001ULL },
	{ 0x0000, 0x8000000000000000ULL },
	{ 0x8000, 0x8000000000000000ULL },
	{ 0x0001, 0x8000000000000000ULL },
	{ 0x8001, 0x8000000000000000ULL },
	{ 0x3FFE, 0x8000000000000000ULL },
	{ 0x3FFF, 0x8000000000000000ULL },
	{ 0x3FFF, 0x8000000000000001ULL },
	{ 0xBFFF, 0x8000000000000000ULL },
	{ 0xBFFF, 0x8000000000000001ULL },
	{ 0x4000, 0x8000000000000000ULL },
	{ 0xC000, 0x8000000000000000ULL },
	{ 0x7FFE, 0xFFFFFFFFFFFFFFFFULL },
	{ 0xFFFE, 0xFFFFFFFFFFFFFFFFULL },
	{ 0x7FFF, 0x8000000000000000ULL },
	{ 0xFFFF, 0x8000000000000000ULL },
	{ 0x7FFF, 0xC000000000000000ULL },
	{ 0xFFFF, 0xC000000000000000ULL },
	{ 0x7FFF, 0x8000000000000001ULL },
	{ 0x7FFF, 0x0000000000000000ULL },
	{ 0x7FFF, 0xFFFFFFFFFFFFFFFFULL },
	{ 0x7F7F, 0x7F7F7F7F7F7F7F7FULL },
	{ 0x8080, 0x8080808080808080ULL },
	{ 0xFFFF, 0xFFFFFFFFFFFFFFFFULL },
};
static const unsigned kPool = sizeof pool / sizeof pool[0];

static const std::int32_t int_pool[] = {
	0, 1, -1, 2, -2, 127, -127, 128, -128,
	255, -255, 256, -256, 32767, -32767, 32768, -32768,
	65535, -65535, 65536, -65536,
	0x7fffffff, (std::int32_t)0x80000000u, (std::int32_t)0x80000001u,
	(std::int32_t)0xffffff00u, (std::int32_t)0x00000080u,
};
static const unsigned kIntPool = sizeof int_pool / sizeof int_pool[0];

std::uint64_t rng_state = 0xB0012A3C0FFEEULL;

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

Val
rand_val(unsigned mode)
{
	Val v;
	switch (mode % 10) {
	case 0:
		v = pool[next_rand() % kPool];
		break;
	case 1:
		v.high = (std::uint16_t)(next_rand() & 0xFFFFULL);
		v.low = next_rand();
		break;
	case 2:
		v.high = (std::uint16_t)((next_rand() & 0x8000ULL) | 0x7FFFULL);
		v.low = next_rand();
		if ((next_rand() & 1) == 0)
			v.low = 0x8000000000000000ULL;
		break;
	case 3:
		v.high = (std::uint16_t)(next_rand() & 0x0003ULL);
		v.low = next_rand() & 0x0000000000000FFFULL;
		break;
	case 4:
		v.high = (std::uint16_t)((next_rand() & 1ULL) << 15);
		v.low = 0;
		break;
	case 5: {
		unsigned b = (unsigned)(next_rand() & 0xFFULL);
		v.high = (std::uint16_t)((b << 8) | b);
		std::uint64_t x = 0;
		for (int i = 0; i < 8; ++i)
			x = (x << 8) | b;
		v.low = x;
		break;
	}
	case 6:
		v = pool[next_rand() % kPool];
		v.low ^= 1ULL << (next_rand() % 64);
		break;
	case 7:
		v = pool[next_rand() % kPool];
		v.high ^= (std::uint16_t)(1U << (next_rand() % 16));
		break;
	case 8:
		v.high = (std::uint16_t)(0x3FFF + (next_rand() % 3ULL) - 1ULL);
		v.low = 0x8000000000000000ULL | (next_rand() & 0x7FFFFFFFFFFFFFFFULL);
		break;
	default:
		v.high = (std::uint16_t)(next_rand() & 0xFFFFULL);
		v.low = next_rand() | 0x8000000000000000ULL;
		break;
	}
	return v;
}

} /* anonymous namespace */

int
main(void)
{
	/* Named edge cases for negation / multiply-by-minus-one. */
	check_neg("plus_one", { 0x3FFF, 0x8000000000000000ULL });
	check_neg("minus_one", { 0xBFFF, 0x8000000000000000ULL });
	check_neg("plus_zero", { 0x0000, 0x0000000000000000ULL });
	check_neg("minus_zero", { 0x8000, 0x0000000000000000ULL });
	check_neg("plus_inf", { 0x7FFF, 0x8000000000000000ULL });
	check_neg("minus_inf", { 0xFFFF, 0x8000000000000000ULL });
	check_neg("plus_nan", { 0x7FFF, 0xC000000000000000ULL });
	check_neg("minus_nan", { 0xFFFF, 0xC000000000000000ULL });
	check_neg("snan", { 0x7FFF, 0x8000000000000001ULL });
	check_neg("denorm", { 0x0000, 0x0000000000000001ULL });
	check_neg("neg_denorm", { 0x8000, 0x0000000000000001ULL });
	check_neg("max_finite", { 0x7FFE, 0xFFFFFFFFFFFFFFFFULL });

	check_six("zero", 0);
	check_six("one", 1);
	check_six("minus_one", -1);
	check_six("int_min", (std::int32_t)0x80000000u);
	check_six("int_max", 0x7fffffff);

	check_mul("one_times_minus_one",
	    { 0x3FFF, 0x8000000000000000ULL },
	    { 0xBFFF, 0x8000000000000000ULL });
	check_mul("zero_times_one",
	    { 0x0000, 0x0000000000000000ULL },
	    { 0x3FFF, 0x8000000000000000ULL });
	check_mul("inf_times_zero",
	    { 0x7FFF, 0x8000000000000000ULL },
	    { 0x0000, 0x0000000000000000ULL });
	check_mul("inf_times_finite",
	    { 0x7FFF, 0x8000000000000000ULL },
	    { 0x3FFF, 0x8000000000000000ULL });
	check_mul("nan_times_one",
	    { 0x7FFF, 0xC000000000000000ULL },
	    { 0x3FFF, 0x8000000000000000ULL });
	check_mul("snan_times_inf",
	    { 0x7FFF, 0x8000000000000001ULL },
	    { 0x7FFF, 0x8000000000000000ULL });

	for (unsigned i = 0; i < kPool; ++i)
		check_neg("pool", pool[i]);

	for (unsigned i = 0; i < kPool; ++i)
		for (unsigned j = 0; j < kPool; ++j)
			check_mul("pool", pool[i], pool[j]);

	for (unsigned i = 0; i < kIntPool; ++i)
		check_six("int_pool", int_pool[i]);

	for (unsigned i = 0; i < kPool; ++i)
		check_mul("neg_path", pool[i], { 0xBFFF, 0x8000000000000000ULL });

	const unsigned long iterations = 220000UL;
	for (unsigned long n = 0; n < iterations; ++n) {
		Val a = rand_val((unsigned)(n % 10));
		Val b = rand_val((unsigned)((n >> 4) % 10));
		check_neg("random", a);
		check_mul("random", a, b);
		if ((n & 0x3f) == 0) {
			std::int32_t si = (std::int32_t)next_rand();
			check_six("random", si);
		}
	}

	Stats *all[] = { &st_neg, &st_mul, &st_six };
	std::printf("\n%-20s %12s %12s\n", "function", "cases", "failures");
	std::printf("%-20s %12s %12s\n", "--------------------",
	    "------------", "------------");
	unsigned long long total_cases = 0, total_failures = 0;
	for (auto *s : all) {
		std::printf("%-20s %12llu %12llu\n", s->name, s->cases,
		    s->failures);
		total_cases += s->cases;
		total_failures += s->failures;
	}
	std::printf("\n%s: %llu failure(s)\n",
	    total_failures == 0 ? "PASS" : "FAIL", total_failures);
	return total_failures == 0 ? 0 : 1;
}
