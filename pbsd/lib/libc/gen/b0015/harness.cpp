/*
 * Differential test for batch b0015: pbsd::lib_libc_gen::b0015::{lrand48,
 * drand48} against ref_lrand48/ref_drand48 in oracle.c.
 *
 * Neither function takes or writes a caller-supplied buffer, so the
 * "two buffers plus guard bytes" rule is applied to the thing these
 * functions actually mutate: the rand48 state triple.  The port and the
 * oracle own separate copies of (_rand48_seed, _rand48_mult, _rand48_add);
 * every case seeds both copies identically and then compares ALL THREE
 * words after EVERY call.  Comparing the return value alone would not be
 * enough: lrand48() only exposes bits 17..47 of the 64-bit seed, so a port
 * that corrupted the other 33 bits, or that clobbered the multiplier or the
 * addend, would still return the right number for the first call.  The
 * multiplier and addend are the guard words - nothing in either function may
 * touch them, and the checks below fail if anything does.
 */

import pbsd.lib.libc.gen.b0015;

#include <cstdint>
#include <cstdio>
#include <cstring>

namespace port = pbsd::lib_libc_gen::b0015;

extern "C" {
extern std::uint64_t _rand48_seed;
extern std::uint64_t _rand48_mult;
extern std::uint64_t _rand48_add;
long ref_lrand48(void);
double ref_drand48(void);
}

/* ------------------------------------------------------------------ */

struct Stats {
	const char	*name;
	unsigned long long cases;
	unsigned long long fails;
	unsigned long long reported;
};

static Stats st_l = { "lrand48", 0, 0, 0 };
static Stats st_d = { "drand48", 0, 0, 0 };
static Stats st_mix = { "lrand48/drand48 interleaved", 0, 0, 0 };

static const unsigned long long REPORT_LIMIT = 10;

static std::uint64_t
dbits(double d)
{
	std::uint64_t u;

	std::memcpy(&u, &d, sizeof(u));
	return u;
}

static void
set_state(std::uint64_t seed, std::uint64_t mult, std::uint64_t add)
{
	port::_rand48_seed = seed;
	port::_rand48_mult = mult;
	port::_rand48_add = add;
	_rand48_seed = seed;
	_rand48_mult = mult;
	_rand48_add = add;
}

/*
 * Compare the whole state triple of the port against the whole state triple
 * of the oracle, and check that neither side disturbed the guard words.
 */
static bool
state_matches(std::uint64_t mult, std::uint64_t add)
{
	return port::_rand48_seed == _rand48_seed &&
	    port::_rand48_mult == _rand48_mult &&
	    port::_rand48_add == _rand48_add &&
	    port::_rand48_mult == mult && port::_rand48_add == add &&
	    _rand48_mult == mult && _rand48_add == add;
}

static void
fail_l(Stats &st, const char *what, int call, std::uint64_t seed,
    std::uint64_t mult, std::uint64_t add, long got, long want)
{
	st.fails++;
	if (st.reported++ >= REPORT_LIMIT)
		return;
	std::printf("FAIL %s [%s] call %d seed=%016llx mult=%016llx "
	    "add=%016llx\n", st.name, what, call,
	    (unsigned long long)seed, (unsigned long long)mult,
	    (unsigned long long)add);
	std::printf("     ret port=%ld ref=%ld\n", got, want);
	std::printf("     seed port=%016llx ref=%016llx\n",
	    (unsigned long long)port::_rand48_seed,
	    (unsigned long long)_rand48_seed);
	std::printf("     mult port=%016llx ref=%016llx  "
	    "add port=%016llx ref=%016llx\n",
	    (unsigned long long)port::_rand48_mult,
	    (unsigned long long)_rand48_mult,
	    (unsigned long long)port::_rand48_add,
	    (unsigned long long)_rand48_add);
}

static void
fail_d(Stats &st, const char *what, int call, std::uint64_t seed,
    std::uint64_t mult, std::uint64_t add, double got, double want)
{
	st.fails++;
	if (st.reported++ >= REPORT_LIMIT)
		return;
	std::printf("FAIL %s [%s] call %d seed=%016llx mult=%016llx "
	    "add=%016llx\n", st.name, what, call,
	    (unsigned long long)seed, (unsigned long long)mult,
	    (unsigned long long)add);
	std::printf("     ret port=%a (%016llx) ref=%a (%016llx)\n",
	    got, (unsigned long long)dbits(got),
	    want, (unsigned long long)dbits(want));
	std::printf("     seed port=%016llx ref=%016llx\n",
	    (unsigned long long)port::_rand48_seed,
	    (unsigned long long)_rand48_seed);
	std::printf("     mult port=%016llx ref=%016llx  "
	    "add port=%016llx ref=%016llx\n",
	    (unsigned long long)port::_rand48_mult,
	    (unsigned long long)_rand48_mult,
	    (unsigned long long)port::_rand48_add,
	    (unsigned long long)_rand48_add);
}

/* ------------------------------------------------------------------ */

/* One case = one fresh state plus ncalls consecutive calls. */
static void
case_l(const char *what, std::uint64_t seed, std::uint64_t mult,
    std::uint64_t add, int ncalls)
{
	st_l.cases++;
	set_state(seed, mult, add);
	for (int i = 0; i < ncalls; i++) {
		long a = port::lrand48();
		long b = ref_lrand48();

		if (a != b || !state_matches(mult, add)) {
			fail_l(st_l, what, i, seed, mult, add, a, b);
			return;
		}
	}
}

static void
case_d(const char *what, std::uint64_t seed, std::uint64_t mult,
    std::uint64_t add, int ncalls)
{
	st_d.cases++;
	set_state(seed, mult, add);
	for (int i = 0; i < ncalls; i++) {
		double a = port::drand48();
		double b = ref_drand48();

		if (dbits(a) != dbits(b) || !state_matches(mult, add)) {
			fail_d(st_d, what, i, seed, mult, add, a, b);
			return;
		}
	}
}

/*
 * Drive the shared state through an interleaved sequence of both functions,
 * checking after every single step.  This is the stateful-iterator style
 * check: the state is carried forward and any divergence in one function's
 * update is visible to the other.
 */
static void
case_mix(const char *what, std::uint64_t seed, std::uint64_t mult,
    std::uint64_t add, std::uint64_t pattern, int nsteps)
{
	st_mix.cases++;
	set_state(seed, mult, add);
	for (int i = 0; i < nsteps; i++) {
		if ((pattern >> (i % 64)) & 1) {
			long a = port::lrand48();
			long b = ref_lrand48();

			if (a != b || !state_matches(mult, add)) {
				fail_l(st_mix, what, i, seed, mult, add, a, b);
				return;
			}
		} else {
			double a = port::drand48();
			double b = ref_drand48();

			if (dbits(a) != dbits(b) ||
			    !state_matches(mult, add)) {
				fail_d(st_mix, what, i, seed, mult, add, a, b);
				return;
			}
		}
	}
}

/* ------------------------------------------------------------------ */

/*
 * Inverse of an odd 64-bit multiplier, so that a case can demand an exact
 * post-update seed: seed = (target - add) * mult^-1 makes the single
 * multiply-add inside the function land on `target' in all 64 bits.  That is
 * what lets the hand-written cases below place the top set bit of the low 48
 * bits at every position 0..47, and land exactly on zero.
 */
static std::uint64_t
inv_odd(std::uint64_t m)
{
	std::uint64_t inv = m;			/* exact mod 2^3 */

	for (int i = 0; i < 5; i++)		/* 3 -> 6 -> ... -> 96 bits */
		inv *= 2 - m * inv;
	return inv;
}

static std::uint64_t
seed_for_post(std::uint64_t target, std::uint64_t mult, std::uint64_t add)
{
	return (target - add) * inv_odd(mult);
}

/* ------------------------------------------------------------------ */

static std::uint64_t rng_state;

static std::uint64_t
rnd(void)
{
	std::uint64_t z = (rng_state += 0x9e3779b97f4a7c15ULL);

	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return z ^ (z >> 31);
}

struct Params {
	std::uint64_t	seed;
	std::uint64_t	mult;
	std::uint64_t	add;
};

/*
 * Ten generators, chosen so the sweep does not just sample uniform 64-bit
 * seeds.  A uniform seed puts bit 47 of the masked value on half the time,
 * so drand48()'s normalising loop would almost never run more than a couple
 * of iterations; the targeted post-state modes below spread the loop trip
 * count uniformly over 0..47 and also hit the low-48-bits-are-zero path.
 */
static Params
gen_params(void)
{
	Params p;
	std::uint64_t r = rnd();
	std::uint64_t t;
	unsigned k;

	switch (r % 10) {
	case 0:
		p.seed = rnd();
		p.mult = port::RAND48_MULT;
		p.add = port::RAND48_ADD;
		break;
	case 1:
		p.seed = rnd() & 0xffffffffffffULL;
		p.mult = port::RAND48_MULT;
		p.add = port::RAND48_ADD;
		break;
	case 2:
		p.seed = rnd();
		p.mult = rnd() | 1;
		p.add = rnd();
		break;
	case 3:
		p.seed = rnd();
		p.mult = rnd();		/* may be even */
		p.add = rnd();
		break;
	case 4:
		p.seed = rnd();
		p.mult = 1;
		p.add = rnd() >> (rnd() % 64);
		break;
	case 5:
		p.seed = rnd();
		p.mult = rnd() | 1;
		p.add = 0;
		break;
	case 6:
		/* Exact post-state, geometric spread of magnitudes. */
		p.mult = port::RAND48_MULT;
		p.add = port::RAND48_ADD;
		t = rnd() >> (rnd() % 64);
		p.seed = seed_for_post(t, p.mult, p.add);
		break;
	case 7:
		/* Exact post-state with the top set bit of low 48 at k. */
		p.mult = rnd() | 1;
		p.add = rnd();
		k = (unsigned)(rnd() % 48);
		t = (1ULL << k) | (rnd() & ((1ULL << k) - 1));
		p.seed = seed_for_post(t, p.mult, p.add);
		break;
	case 8:
		/* Exact post-state whose low 48 bits are zero. */
		p.mult = rnd() | 1;
		p.add = rnd();
		t = rnd() << 48;
		p.seed = seed_for_post(t, p.mult, p.add);
		break;
	default:
		p.seed = rnd() % 8;
		p.mult = rnd() % 8;
		p.add = rnd() % 8;
		break;
	}
	return p;
}

/* ------------------------------------------------------------------ */

static const std::uint64_t DEF_MULT = port::RAND48_MULT;
static const std::uint64_t DEF_ADD = port::RAND48_ADD;

/*
 * Hand-written cases.  Every one is applied to both functions so that each
 * gets the empty/degenerate state, the single-bit states, the NUL-heavy
 * (all-zero) states, the high-bit-set states and both sides of every
 * boundary the two bodies contain: the >> 17 window, the 0x7fffffff mask,
 * the 0xffffffffffff mask, the == 0 early return and the top of the
 * normalising loop.
 */
static void
edge_cases(void)
{
	struct Case {
		const char	*what;
		std::uint64_t	seed;
		std::uint64_t	mult;
		std::uint64_t	add;
		int		ncalls;
	};

	static const Case cases[] = {
		/* The library's own starting state, run for a long stretch. */
		{ "default state", port::RAND48_SEED, DEF_MULT, DEF_ADD, 64 },

		/* Degenerate seeds. */
		{ "seed 0", 0, DEF_MULT, DEF_ADD, 8 },
		{ "seed 1", 1, DEF_MULT, DEF_ADD, 8 },
		{ "seed all ones", ~0ULL, DEF_MULT, DEF_ADD, 8 },
		{ "seed low48 ones", 0xffffffffffffULL, DEF_MULT, DEF_ADD, 8 },
		{ "seed high16 only", 0xffff000000000000ULL, DEF_MULT,
		  DEF_ADD, 8 },
		{ "seed bit47", 1ULL << 47, DEF_MULT, DEF_ADD, 8 },
		{ "seed bit48", 1ULL << 48, DEF_MULT, DEF_ADD, 8 },
		{ "seed bit63", 1ULL << 63, DEF_MULT, DEF_ADD, 8 },

		/* Degenerate parameters. */
		{ "mult 0 add 0", 0x123456789abcULL, 0, 0, 4 },
		{ "mult 0 add 1", 0x123456789abcULL, 0, 1, 4 },
		{ "mult 0 add max", 0x123456789abcULL, 0, ~0ULL, 4 },
		{ "mult 1 add 0", 0x123456789abcULL, 1, 0, 4 },
		{ "mult 1 add 1", 0, 1, 1, 8 },
		{ "mult 2 add 0", 0x123456789abcULL, 2, 0, 8 },
		{ "mult max add max", 0x123456789abcULL, ~0ULL, ~0ULL, 8 },
		{ "mult default add 0", port::RAND48_SEED, DEF_MULT, 0, 8 },

		/*
		 * mult 1 / add 0 is the identity update, so the seed below IS
		 * the value both bodies then dissect.  These walk the >> 17
		 * window and the 0x7fffffff mask from both sides.
		 */
		{ "identity: 0", 0, 1, 0, 1 },
		{ "identity: bit16 (below window)", 1ULL << 16, 1, 0, 1 },
		{ "identity: bit17 (window lsb)", 1ULL << 17, 1, 0, 1 },
		{ "identity: window-1", (1ULL << 17) - 1, 1, 0, 1 },
		{ "identity: bit46", 1ULL << 46, 1, 0, 1 },
		{ "identity: bit47 (window msb)", 1ULL << 47, 1, 0, 1 },
		{ "identity: bit48 (above window)", 1ULL << 48, 1, 0, 1 },
		{ "identity: window all ones", 0x0000fffffffe0000ULL, 1, 0,
		  1 },
		{ "identity: window ones minus msb", 0x00007ffffffe0000ULL, 1,
		  0, 1 },
		{ "identity: low17 all ones", 0x000000000001ffffULL, 1, 0, 1 },
		{ "identity: all ones", ~0ULL, 1, 0, 1 },
		{ "identity: low48 all ones", 0xffffffffffffULL, 1, 0, 1 },
		{ "identity: high16 only", 0xffff000000000000ULL, 1, 0, 1 },
		{ "identity: 0x7fffffff in window", 0x0000fffffffe0000ULL, 1,
		  0, 1 },
	};

	for (unsigned i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
		const Case &c = cases[i];

		case_l(c.what, c.seed, c.mult, c.add, c.ncalls);
		case_d(c.what, c.seed, c.mult, c.add, c.ncalls);
		case_mix(c.what, c.seed, c.mult, c.add, 0xa5a5a5a5a5a5a5a5ULL,
		    c.ncalls < 16 ? c.ncalls * 2 : c.ncalls);
	}

	/*
	 * Exact post-update states.  seed_for_post() makes the single
	 * multiply-add land on the requested 64-bit value, so these drive
	 * drand48()'s normalising loop to every possible trip count and
	 * both branches of its `u.u64 == 0' test, and drive lrand48()'s
	 * shift-and-mask onto exact boundary values.
	 */
	static const std::uint64_t posts[] = {
		0,
		1,
		2,
		3,
		0xffffULL,
		0x10000ULL,
		0x1ffffULL,
		0x20000ULL,
		0x7fffffffffffULL,
		0x800000000000ULL,
		0x800000000001ULL,
		0xffffffffffffULL,
		0xfffffffffffeULL,
		0x0000fffffffe0000ULL,
		~0ULL,
		0xffff000000000000ULL,	/* low 48 zero, high bits set */
		0x0001000000000000ULL,	/* low 48 zero, one high bit */
		0x8000000000000000ULL,	/* low 48 zero, sign bit only */
	};

	for (unsigned i = 0; i < sizeof(posts) / sizeof(posts[0]); i++) {
		std::uint64_t t = posts[i];
		std::uint64_t s = seed_for_post(t, DEF_MULT, DEF_ADD);

		case_l("post-state", s, DEF_MULT, DEF_ADD, 1);
		case_d("post-state", s, DEF_MULT, DEF_ADD, 1);
	}

	/*
	 * Top set bit of the masked value at every position 0..47: loop trip
	 * counts 47 down to 0, i.e. exponents 975 up to 1022.  Each k is
	 * tested with an empty mantissa, a full mantissa and a mixed one, and
	 * with high garbage above bit 48 that the mask must discard.
	 */
	for (unsigned k = 0; k < 48; k++) {
		std::uint64_t bit = 1ULL << k;
		std::uint64_t low = bit - 1;
		std::uint64_t variants[4];

		variants[0] = bit;
		variants[1] = bit | low;
		variants[2] = bit | (low & 0xa5a5a5a5a5a5ULL);
		variants[3] = bit | (low & 0x5a5a5a5a5a5aULL) |
		    0xffff000000000000ULL;

		for (unsigned v = 0; v < 4; v++) {
			std::uint64_t s = seed_for_post(variants[v], DEF_MULT,
			    DEF_ADD);

			case_d("top bit k", s, DEF_MULT, DEF_ADD, 1);
			case_l("top bit k", s, DEF_MULT, DEF_ADD, 1);
		}
	}

	/* The same sweep with a non-default odd multiplier and addend. */
	for (unsigned k = 0; k < 48; k++) {
		std::uint64_t mult = 0x0123456789abcdefULL;
		std::uint64_t add = 0xfedcba9876543210ULL;
		std::uint64_t t = (1ULL << k) | ((1ULL << k) - 1);
		std::uint64_t s = seed_for_post(t, mult, add);

		case_d("top bit k, alt params", s, mult, add, 1);
		case_l("top bit k, alt params", s, mult, add, 1);
	}
}

/* ------------------------------------------------------------------ */

static const unsigned long long SWEEP = 250000;

static void
sweep(void)
{
	rng_state = 0x0f1e2d3c4b5a6978ULL;
	for (unsigned long long i = 0; i < SWEEP; i++) {
		Params p = gen_params();

		case_l("sweep", p.seed, p.mult, p.add, 1 + (int)(rnd() % 4));
	}

	rng_state = 0x123456789abcdef0ULL;
	for (unsigned long long i = 0; i < SWEEP; i++) {
		Params p = gen_params();

		case_d("sweep", p.seed, p.mult, p.add, 1 + (int)(rnd() % 4));
	}

	rng_state = 0xfeedfacecafebeefULL;
	for (unsigned long long i = 0; i < SWEEP; i++) {
		Params p = gen_params();
		std::uint64_t pattern = rnd();

		case_mix("sweep", p.seed, p.mult, p.add, pattern,
		    1 + (int)(rnd() % 8));
	}
}

/* ------------------------------------------------------------------ */

int
main(void)
{
	edge_cases();
	sweep();

	const Stats *all[] = { &st_l, &st_d, &st_mix };
	unsigned long long total_fails = 0;

	std::printf("\n%-32s %12s %12s  %s\n", "function", "cases",
	    "failures", "result");
	std::printf("-------------------------------------------------"
	    "-----------------------\n");
	for (unsigned i = 0; i < 3; i++) {
		const Stats *s = all[i];

		total_fails += s->fails;
		std::printf("%-32s %12llu %12llu  %s\n", s->name, s->cases,
		    s->fails, s->fails == 0 ? "ok" : "FAILED");
	}
	std::printf("-------------------------------------------------"
	    "-----------------------\n");
	std::printf("%-32s %12s %12llu  %s\n", "TOTAL", "",
	    total_fails, total_fails == 0 ? "ok" : "FAILED");

	return total_fails == 0 ? 0 : 1;
}
