/*
 * PBSD migration batch b0049 -- differential test.
 *
 * Every case is run through both the C++23 port and the ref_ oracle in
 * oracle.c and the results are compared bit for bit.  The unord* helpers also
 * raise invalid-operation flags on signalling NaNs via the shared float32_eq /
 * float64_eq primitives, so both the return value and float_exception_flags
 * are captured and compared for those functions.
 */

import pbsd.lib.libc.softfloat.b0049;

#include <cstddef>
#include <cstdint>
#include <cstdio>

namespace port = pbsd::lib_libc_softfloat::b0049;

extern "C" {
struct c_float128 {
	std::uint64_t low;
	std::uint64_t high;
};

c_float128 ref___negtf2(c_float128);
int ref___unorddf2(unsigned long long, unsigned long long);
int ref___unordsf2(unsigned int, unsigned int);
int ref_fpgetmask(void);
extern int float_exception_flags;
extern int float_exception_mask;
}

namespace {

static_assert(sizeof(c_float128) == sizeof(port::float128));
static_assert(offsetof(c_float128, low) == offsetof(port::float128, low));
static_assert(offsetof(c_float128, high) == offsetof(port::float128, high));

struct Stats {
	const char *name;
	unsigned long long cases = 0;
	unsigned long long failures = 0;
	unsigned long long reported = 0;
	bool seen[256] = {};

	explicit Stats(const char *n) : name(n) {}

	void note(unsigned long long r) { seen[r & 0xFFu] = true; }

	unsigned distinct() const
	{
		unsigned n = 0;
		for (bool b : seen)
			if (b)
				n++;
		return n;
	}
};

const std::uint32_t f32_edges[] = {
	0x00000000u, 0x80000000u,
	0x00000001u, 0x80000001u,
	0x004C0000u, 0x804C0000u,
	0x007FFFFFu, 0x807FFFFFu,
	0x00800000u, 0x80800000u,
	0x00800001u, 0x80800001u,
	0x3F7FFFFFu, 0xBF7FFFFFu,
	0x3F800000u, 0xBF800000u,
	0x3F800001u, 0xBF800001u,
	0x40000000u, 0xC0000000u,
	0x7F7FFFFFu, 0xFF7FFFFFu,
	0x7F800000u, 0xFF800000u,
	0x7F800001u, 0xFF800001u,
	0x7FBFFFFFu, 0xFFBFFFFFu,
	0x7FC00000u, 0xFFC00000u,
	0x7FFFFFFFu, 0xFFFFFFFFu
};

const std::uint64_t f64_edges[] = {
	0x0000000000000000ull, 0x8000000000000000ull,
	0x0000000000000001ull, 0x8000000000000001ull,
	0x0008000000000000ull, 0x8008000000000000ull,
	0x000FFFFFFFFFFFFFull, 0x800FFFFFFFFFFFFFull,
	0x0010000000000000ull, 0x8010000000000000ull,
	0x0010000000000001ull, 0x8010000000000001ull,
	0x3FEFFFFFFFFFFFFFull, 0xBFEFFFFFFFFFFFFFull,
	0x3FF0000000000000ull, 0xBFF0000000000000ull,
	0x3FF0000000000001ull, 0xBFF0000000000001ull,
	0x4000000000000000ull, 0xC000000000000000ull,
	0x7FEFFFFFFFFFFFFFull, 0xFFEFFFFFFFFFFFFFull,
	0x7FF0000000000000ull, 0xFFF0000000000000ull,
	0x7FF0000000000001ull, 0xFFF0000000000001ull,
	0x7FF7FFFFFFFFFFFFull, 0xFFF7FFFFFFFFFFFFull,
	0x7FF8000000000000ull, 0xFFF8000000000000ull,
	0x7FFFFFFFFFFFFFFFull, 0xFFFFFFFFFFFFFFFFull
};

const std::uint64_t f128_hi_edges[] = {
	0x0000000000000000ull, 0x8000000000000000ull,
	0x0000000000000001ull, 0x8000000000000001ull,
	0x7FFF000000000000ull, 0xFFFF000000000000ull,
	0x7FFF000000000001ull, 0xFFFF000000000001ull,
	0x7FFF7FFFFFFFFFFFULL, 0xFFFF7FFFFFFFFFFFULL,
	0x7FFF800000000000ull, 0xFFFF800000000000ull,
	0x7FFFFFFFFFFFFFFFull, 0xFFFFFFFFFFFFFFFFull
};

const std::uint64_t f128_lo_edges[] = {
	0x0000000000000000ull, 0x8000000000000000ull,
	0x0000000000000001ull, 0x8000000000000001ull,
	0x7FFFFFFFFFFFFFFFull, 0xFFFFFFFFFFFFFFFFull
};

const int mask_edges[] = {
	0, 1, 2, 4, 8, 16, 32, 64,
	0x7F, 0x80, 0xFF, 0x100, 0x7FFFFFFF, -1, -2, 0x40000000
};

std::uint64_t rng_state;

void
rng_seed(std::uint64_t s)
{

	rng_state = s;
}

std::uint64_t
rng_next()
{
	std::uint64_t z = (rng_state += 0x9E3779B97F4A7C15ull);

	z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
	z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
	return z ^ (z >> 31);
}

std::uint32_t
gen32()
{
	std::uint64_t r = rng_next();
	std::uint32_t sign = static_cast<std::uint32_t>(r & 1) << 31;
	std::uint32_t frac = static_cast<std::uint32_t>(r >> 32) & 0x007FFFFFu;

	switch ((r >> 1) & 7) {
	case 0:
		return static_cast<std::uint32_t>(rng_next());
	case 1:
		return sign;
	case 2:
		return sign | frac;
	case 3:
		return sign | 0x00800000u | frac;
	case 4:
		return sign | 0x7F000000u | frac;
	case 5:
		return sign | 0x7F800000u;
	case 6:
		return sign | 0x7F800000u | (frac | 1u);
	default:
		return sign | (static_cast<std::uint32_t>(r >> 8) &
		    0x7FFFFFFFu);
	}
}

std::uint64_t
gen64()
{
	std::uint64_t r = rng_next();
	std::uint64_t sign = (r & 1) << 63;
	std::uint64_t frac = rng_next() & 0x000FFFFFFFFFFFFFull;

	switch ((r >> 1) & 7) {
	case 0:
		return rng_next();
	case 1:
		return sign;
	case 2:
		return sign | frac;
	case 3:
		return sign | 0x0010000000000000ull | frac;
	case 4:
		return sign | 0x7FE0000000000000ull | frac;
	case 5:
		return sign | 0x7FF0000000000000ull;
	case 6:
		return sign | 0x7FF0000000000000ull | (frac | 1ull);
	default:
		return sign | (rng_next() & 0x7FFFFFFFFFFFFFFFull);
	}
}

std::uint32_t
mate32(std::uint32_t a)
{

	switch (rng_next() & 7) {
	case 0:
		return a;
	case 1:
		return a ^ 0x80000000u;
	case 2:
		return a + 1;
	case 3:
		return a - 1;
	case 4:
		return a ^ 1u;
	default:
		return gen32();
	}
}

std::uint64_t
mate64(std::uint64_t a)
{

	switch (rng_next() & 7) {
	case 0:
		return a;
	case 1:
		return a ^ 0x8000000000000000ull;
	case 2:
		return a + 1;
	case 3:
		return a - 1;
	case 4:
		return a ^ 1ull;
	default:
		return gen64();
	}
}

void
check_negtf2(Stats &st, std::uint64_t lo, std::uint64_t hi)
{
	port::float128 pin{ lo, hi };
	c_float128 rin{ lo, hi };

	port::float128 got = port::__negtf2(pin);
	c_float128 want = ref___negtf2(rin);

	st.cases++;
	st.note(want.high ^ (want.high >> 56));
	if (got.low != want.low || got.high != want.high) {
		st.failures++;
		if (st.reported++ < 8)
			std::printf("  __negtf2(lo=0x%016llx hi=0x%016llx): "
			    "port lo=0x%016llx hi=0x%016llx, "
			    "oracle lo=0x%016llx hi=0x%016llx\n",
			    static_cast<unsigned long long>(lo),
			    static_cast<unsigned long long>(hi),
			    static_cast<unsigned long long>(got.low),
			    static_cast<unsigned long long>(got.high),
			    static_cast<unsigned long long>(want.low),
			    static_cast<unsigned long long>(want.high));
	}
}

void
check_unorddf2(Stats &st, std::uint64_t a, std::uint64_t b)
{
	float_exception_flags = 0;
	int got = port::__unorddf2(a, b);
	int got_flags = float_exception_flags;

	float_exception_flags = 0;
	int want = ref___unorddf2(a, b);
	int want_flags = float_exception_flags;

	st.cases++;
	st.note(static_cast<unsigned long long>(static_cast<unsigned char>(want)));
	if (got != want || got_flags != want_flags) {
		st.failures++;
		if (st.reported++ < 8)
			std::printf("  __unorddf2(0x%016llx, 0x%016llx): port %d "
			    "flags %d, oracle %d flags %d\n",
			    static_cast<unsigned long long>(a),
			    static_cast<unsigned long long>(b),
			    got, got_flags, want, want_flags);
	}
}

void
check_unordsf2(Stats &st, std::uint32_t a, std::uint32_t b)
{
	float_exception_flags = 0;
	int got = port::__unordsf2(a, b);
	int got_flags = float_exception_flags;

	float_exception_flags = 0;
	int want = ref___unordsf2(a, b);
	int want_flags = float_exception_flags;

	st.cases++;
	st.note(static_cast<unsigned long long>(static_cast<unsigned char>(want)));
	if (got != want || got_flags != want_flags) {
		st.failures++;
		if (st.reported++ < 8)
			std::printf("  __unordsf2(0x%08lx, 0x%08lx): port %d "
			    "flags %d, oracle %d flags %d\n",
			    static_cast<unsigned long>(a),
			    static_cast<unsigned long>(b),
			    got, got_flags, want, want_flags);
	}
}

void
check_fpgetmask(Stats &st, int mask)
{
	float_exception_mask = mask;

	int got = port::fpgetmask();
	int want = ref_fpgetmask();

	st.cases++;
	st.note(static_cast<unsigned long long>(static_cast<unsigned char>(want)));
	if (got != want) {
		st.failures++;
		if (st.reported++ < 8)
			std::printf("  fpgetmask(mask=%d): port %d, oracle %d\n",
			    mask, got, want);
	}
}

const unsigned long long SWEEP = 200000;

} /* namespace */

int
main()
{
	Stats neg("__negtf2"), udf("__unorddf2"), usf("__unordsf2"),
	    mask("fpgetmask");

	/* __negtf2: every pair of special high/low encodings. */
	for (std::uint64_t hi : f128_hi_edges)
		for (std::uint64_t lo : f128_lo_edges)
			check_negtf2(neg, lo, hi);

	/* Walk the float128 exponent field with a fixed fraction. */
	for (unsigned e = 0; e <= 0x7FFF; e += (e < 0x100 ? 1 : 0x100)) {
		for (unsigned s = 0; s < 2; s++) {
			std::uint64_t hi =
			    (static_cast<std::uint64_t>(s) << 63)
			    | (static_cast<std::uint64_t>(e) << 48)
			    | 0x123456789ABCull;
			check_negtf2(neg, 0, hi);
			check_negtf2(neg, 0xFEDCBA9876543210ull, hi);
			check_negtf2(neg, hi, hi);
		}
	}

	/* __unorddf2 / __unordsf2: every pair of IEEE special encodings. */
	for (std::uint64_t a : f64_edges)
		for (std::uint64_t b : f64_edges)
			check_unorddf2(udf, a, b);
	for (std::uint32_t a : f32_edges)
		for (std::uint32_t b : f32_edges)
			check_unordsf2(usf, a, b);

	/* Exponent-boundary sweeps so ordered (0) and unordered (1) both fire. */
	for (unsigned e = 0; e <= 0x7FF; e++) {
		for (unsigned s = 0; s < 2; s++) {
			std::uint64_t a =
			    (static_cast<std::uint64_t>(s) << 63)
			    | (static_cast<std::uint64_t>(e) << 52)
			    | 0x123456789ABull;
			for (std::uint64_t b : f64_edges) {
				check_unorddf2(udf, a, b);
				check_unorddf2(udf, b, a);
			}
			check_unorddf2(udf, a, a);
		}
	}
	for (unsigned e = 0; e <= 0xFF; e++) {
		for (unsigned s = 0; s < 2; s++) {
			std::uint32_t a = (static_cast<std::uint32_t>(s) << 31)
			    | (static_cast<std::uint32_t>(e) << 23) | 0x12345u;
			for (std::uint32_t b : f32_edges) {
				check_unordsf2(usf, a, b);
				check_unordsf2(usf, b, a);
			}
			check_unordsf2(usf, a, a);
		}
	}

	/* Mixed ordered / NaN pairs: one finite, one quiet or signalling NaN. */
	for (std::uint64_t ord : f64_edges) {
		check_unorddf2(udf, ord, 0x7FF8000000000000ull);
		check_unorddf2(udf, ord, 0xFFF8000000000000ull);
		check_unorddf2(udf, ord, 0x7FF0000000000001ull);
		check_unorddf2(udf, ord, 0xFFF0000000000001ull);
		check_unorddf2(udf, 0x7FF8000000000000ull, ord);
		check_unorddf2(udf, 0x7FF0000000000001ull, ord);
	}
	for (std::uint32_t ord : f32_edges) {
		check_unordsf2(usf, ord, 0x7FC00000u);
		check_unordsf2(usf, ord, 0xFFC00000u);
		check_unordsf2(usf, ord, 0x7F800001u);
		check_unordsf2(usf, ord, 0xFF800001u);
		check_unordsf2(usf, 0x7FC00000u, ord);
		check_unordsf2(usf, 0x7F800001u, ord);
	}

	/* fpgetmask: boundary mask values. */
	for (int m : mask_edges)
		check_fpgetmask(mask, m);

	/* Fixed seed randomised sweeps (>= 200000 each). */
	rng_seed(0xB0049Aull);
	for (unsigned long long i = 0; i < SWEEP; i++) {
		std::uint64_t lo = rng_next();
		std::uint64_t hi = rng_next();
		check_negtf2(neg, lo, hi);
	}
	rng_seed(0xB0049Bull);
	for (unsigned long long i = 0; i < SWEEP; i++) {
		std::uint64_t a = gen64();
		check_unorddf2(udf, a, mate64(a));
	}
	rng_seed(0xB0049Cull);
	for (unsigned long long i = 0; i < SWEEP; i++) {
		std::uint32_t a = gen32();
		check_unordsf2(usf, a, mate32(a));
	}
	rng_seed(0xB0049Dull);
	for (unsigned long long i = 0; i < SWEEP; i++)
		check_fpgetmask(mask, static_cast<int>(rng_next() & 0xFFFFFFFFu));

	const Stats *all[] = { &neg, &udf, &usf, &mask };
	unsigned long long total_failures = 0;

	std::printf("\n%-12s %12s %12s %10s %8s\n", "function", "cases",
	    "failures", "distinct", "result");
	for (const Stats *s : all) {
		bool ok = s->failures == 0 && s->distinct() >= 2;

		if (!ok)
			total_failures += s->failures ? s->failures : 1;
		std::printf("%-12s %12llu %12llu %10u %8s\n", s->name,
		    s->cases, s->failures, s->distinct(),
		    ok ? "PASS" : "FAIL");
	}
	std::printf("\n%s\n", total_failures == 0 ?
	    "b0049: all cases matched the oracle" :
	    "b0049: DIVERGENCE from oracle");
	return total_failures == 0 ? 0 : 1;
}
