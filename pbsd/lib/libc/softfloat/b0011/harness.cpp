/*
 * PBSD migration batch b0011 -- differential test.
 *
 * Every case is run through both the C++23 port and the ref_ oracle in
 * oracle.c and the results are compared bit for bit.  None of the batch
 * functions writes through a pointer or returns a pointer, so the whole
 * observable result of a call is (return value, softfloat exception flags);
 * both are captured and compared for every case.
 *
 * The "edge case" analogue of NUL-heavy / high-bit-byte strings for these
 * floating point entry points is the set of IEEE-754 special encodings:
 * both zeroes, both infinities, the smallest and largest subnormals, the
 * smallest and largest normals, the values adjacent to 1.0, and quiet and
 * signalling NaNs of both signs at both ends of their payload ranges.
 * Every pair of those is tried for the two-argument functions.
 */

import pbsd.lib.libc.softfloat.b0011;

#include <cstdint>
#include <cstdio>

namespace port = pbsd::lib_libc_softfloat::b0011;

extern "C" {
char ref___eqdf2(std::uint64_t, std::uint64_t);
std::uint64_t ref___negdf2(std::uint64_t);
char ref___gesf2(std::uint32_t, std::uint32_t);
char ref___lesf2(std::uint32_t, std::uint32_t);
extern signed char float_exception_flags;
}

namespace {

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
	0x00000000u, 0x80000000u,	/* +0, -0 */
	0x00000001u, 0x80000001u,	/* smallest subnormal */
	0x004C0000u, 0x804C0000u,	/* mid subnormal */
	0x007FFFFFu, 0x807FFFFFu,	/* largest subnormal */
	0x00800000u, 0x80800000u,	/* smallest normal */
	0x00800001u, 0x80800001u,
	0x3F7FFFFFu, 0xBF7FFFFFu,	/* just below 1.0 */
	0x3F800000u, 0xBF800000u,	/* 1.0 */
	0x3F800001u, 0xBF800001u,	/* just above 1.0 */
	0x40000000u, 0xC0000000u,	/* 2.0 */
	0x7F7FFFFFu, 0xFF7FFFFFu,	/* largest finite */
	0x7F800000u, 0xFF800000u,	/* infinities */
	0x7F800001u, 0xFF800001u,	/* smallest signalling NaN */
	0x7FBFFFFFu, 0xFFBFFFFFu,	/* largest signalling NaN */
	0x7FC00000u, 0xFFC00000u,	/* smallest quiet NaN */
	0x7FFFFFFFu, 0xFFFFFFFFu	/* largest quiet NaN */
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

/*
 * Draw a float32 pattern biased towards the encodings where the softfloat
 * comparison changes behaviour: zeroes, subnormals, both infinities and both
 * NaN classes, as well as plain uniform bit patterns.
 */
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
		return sign;				/* signed zero */
	case 2:
		return sign | frac;			/* subnormal */
	case 3:
		return sign | 0x00800000u | frac;	/* smallest normals */
	case 4:
		return sign | 0x7F000000u | frac;	/* huge normals */
	case 5:
		return sign | 0x7F800000u;		/* infinity */
	case 6:
		return sign | 0x7F800000u | (frac | 1u);	/* NaN */
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

/* Relate the second operand to the first so the equal / adjacent / opposite
 * sign branches of the comparisons are hit often, not just by chance. */
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
check_eqdf2(Stats &st, std::uint64_t a, std::uint64_t b)
{
	float_exception_flags = 0;
	char got = port::__eqdf2(a, b);
	int got_flags = float_exception_flags;

	float_exception_flags = 0;
	char want = ref___eqdf2(a, b);
	int want_flags = float_exception_flags;

	st.cases++;
	st.note(static_cast<unsigned long long>(static_cast<unsigned char>(want)));
	if (got != want || got_flags != want_flags) {
		st.failures++;
		if (st.reported++ < 8)
			std::printf("  __eqdf2(0x%016llx, 0x%016llx): port %d "
			    "flags %d, oracle %d flags %d\n",
			    static_cast<unsigned long long>(a),
			    static_cast<unsigned long long>(b),
			    got, got_flags, want, want_flags);
	}
}

void
check_negdf2(Stats &st, std::uint64_t a)
{
	std::uint64_t got = port::__negdf2(a);
	std::uint64_t want = ref___negdf2(a);

	st.cases++;
	st.note(want ^ (want >> 56));
	if (got != want) {
		st.failures++;
		if (st.reported++ < 8)
			std::printf("  __negdf2(0x%016llx): port 0x%016llx, "
			    "oracle 0x%016llx\n",
			    static_cast<unsigned long long>(a),
			    static_cast<unsigned long long>(got),
			    static_cast<unsigned long long>(want));
	}
}

void
check_gesf2(Stats &st, std::uint32_t a, std::uint32_t b)
{
	float_exception_flags = 0;
	char got = port::__gesf2(a, b);
	int got_flags = float_exception_flags;

	float_exception_flags = 0;
	char want = ref___gesf2(a, b);
	int want_flags = float_exception_flags;

	st.cases++;
	st.note(static_cast<unsigned long long>(static_cast<unsigned char>(want)));
	if (got != want || got_flags != want_flags) {
		st.failures++;
		if (st.reported++ < 8)
			std::printf("  __gesf2(0x%08lx, 0x%08lx): port %d "
			    "flags %d, oracle %d flags %d\n",
			    static_cast<unsigned long>(a),
			    static_cast<unsigned long>(b),
			    got, got_flags, want, want_flags);
	}
}

void
check_lesf2(Stats &st, std::uint32_t a, std::uint32_t b)
{
	float_exception_flags = 0;
	char got = port::__lesf2(a, b);
	int got_flags = float_exception_flags;

	float_exception_flags = 0;
	char want = ref___lesf2(a, b);
	int want_flags = float_exception_flags;

	st.cases++;
	st.note(static_cast<unsigned long long>(static_cast<unsigned char>(want)));
	if (got != want || got_flags != want_flags) {
		st.failures++;
		if (st.reported++ < 8)
			std::printf("  __lesf2(0x%08lx, 0x%08lx): port %d "
			    "flags %d, oracle %d flags %d\n",
			    static_cast<unsigned long>(a),
			    static_cast<unsigned long>(b),
			    got, got_flags, want, want_flags);
	}
}

const unsigned long long SWEEP = 250000;

} /* namespace */

int
main()
{
	Stats eq("__eqdf2"), neg("__negdf2"), ge("__gesf2"), le("__lesf2");

	/* Hand written edge cases: every pair of special encodings. */
	for (std::uint64_t a : f64_edges) {
		check_negdf2(neg, a);
		for (std::uint64_t b : f64_edges)
			check_eqdf2(eq, a, b);
	}
	for (std::uint32_t a : f32_edges)
		for (std::uint32_t b : f32_edges) {
			check_gesf2(ge, a, b);
			check_lesf2(le, a, b);
		}

	/* Walk the exponent field with a fixed fraction, both signs, so the
	 * ordering comparisons are exercised on every exponent boundary. */
	for (unsigned e = 0; e <= 0xFF; e++)
		for (unsigned s = 0; s < 2; s++) {
			std::uint32_t a = (static_cast<std::uint32_t>(s) << 31)
			    | (static_cast<std::uint32_t>(e) << 23) | 0x12345u;
			for (std::uint32_t b : f32_edges) {
				check_gesf2(ge, a, b);
				check_gesf2(ge, b, a);
				check_lesf2(le, a, b);
				check_lesf2(le, b, a);
			}
		}
	for (unsigned e = 0; e <= 0x7FF; e++)
		for (unsigned s = 0; s < 2; s++) {
			std::uint64_t a =
			    (static_cast<std::uint64_t>(s) << 63)
			    | (static_cast<std::uint64_t>(e) << 52)
			    | 0x123456789ABull;
			check_negdf2(neg, a);
			for (std::uint64_t b : f64_edges) {
				check_eqdf2(eq, a, b);
				check_eqdf2(eq, b, a);
			}
			check_eqdf2(eq, a, a);
			check_eqdf2(eq, a, a ^ 0x8000000000000000ull);
		}

	/* Fixed seed randomised sweeps. */
	rng_seed(0xB0011ull);
	for (unsigned long long i = 0; i < SWEEP; i++) {
		std::uint64_t a = gen64();
		check_eqdf2(eq, a, mate64(a));
	}
	rng_seed(0xB0012ull);
	for (unsigned long long i = 0; i < SWEEP; i++)
		check_negdf2(neg, gen64());
	rng_seed(0xB0013ull);
	for (unsigned long long i = 0; i < SWEEP; i++) {
		std::uint32_t a = gen32();
		check_gesf2(ge, a, mate32(a));
	}
	rng_seed(0xB0014ull);
	for (unsigned long long i = 0; i < SWEEP; i++) {
		std::uint32_t a = gen32();
		check_lesf2(le, a, mate32(a));
	}

	const Stats *all[] = { &eq, &neg, &ge, &le };
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
	    "b0011: all cases matched the oracle" :
	    "b0011: DIVERGENCE from oracle");
	return total_failures == 0 ? 0 : 1;
}
