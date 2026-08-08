/*
 * harness.cpp -- differential test for PBSD batch b0052.
 *
 * fpgetsticky, fpgetround, fpsetmask and fpsetround are thin accessors over
 * three softfloat globals.  Every case sets identical initial state on both
 * the C++ port and the ref_ oracle, invokes the matching entry point on each
 * side, and compares the return value plus the full (rounding, flags, mask)
 * state afterwards so a port that returns the right value but writes the
 * wrong global is caught.
 */

import pbsd.lib.libc.softfloat.b0052;

#include <cstdint>
#include <cstdio>
#include <cstdlib>

namespace port = pbsd::lib_libc_softfloat::b0052;

extern "C" {
int ref_fpgetsticky(void);
int ref_fpgetround(void);
int ref_fpsetmask(int mask);
int ref_fpsetround(int rnd_dir);
extern int float_rounding_mode;
extern int float_exception_flags;
extern int float_exception_mask;
}

namespace {

constexpr int MAX_REPORT = 12;
constexpr long RANDOM_PER_FN = 50000;
constexpr long RANDOM_TOTAL = RANDOM_PER_FN * 4;

struct State {
	int rounding;
	int flags;
	int mask;
};

struct Stat {
	const char *name;
	long cases;
	long fails;
	int reported;
};

Stat st_sticky{"fpgetsticky", 0, 0, 0};
Stat st_round_get{"fpgetround", 0, 0, 0};
Stat st_mask{"fpsetmask", 0, 0, 0};
Stat st_round_set{"fpsetround", 0, 0, 0};

std::uint64_t prng_state;

void
prng_seed(std::uint64_t seed)
{

	prng_state = seed;
}

std::uint64_t
prng_next()
{
	std::uint64_t z = (prng_state += 0x9E3779B97F4A7C15ULL);

	z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
	z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
	return z ^ (z >> 31);
}

void
set_ref(const State &s)
{

	float_rounding_mode = s.rounding;
	float_exception_flags = s.flags;
	float_exception_mask = s.mask;
}

State
get_ref()
{

	return {float_rounding_mode, float_exception_flags, float_exception_mask};
}

void
set_port(const State &s)
{

	port::float_rounding_mode = s.rounding;
	port::float_exception_flags = s.flags;
	port::float_exception_mask = s.mask;
}

State
get_port()
{

	return {port::float_rounding_mode, port::float_exception_flags,
	    port::float_exception_mask};
}

bool
states_equal(const State &a, const State &b)
{

	return a.rounding == b.rounding && a.flags == b.flags && a.mask == b.mask;
}

void
report(Stat &st, const char *kind, int arg, int got, int want,
    const State &gs, const State &ws, const State &gp, const State &wp)
{

	if (st.reported >= MAX_REPORT)
		return;
	st.reported++;
	std::fprintf(stderr,
	    "%s %s arg=%d ret %d vs %d  ref{%d,%d,%d} port{%d,%d,%d} want{%d,%d,%d}\n",
	    st.name, kind, arg, got, want, gs.rounding, gs.flags, gs.mask,
	    gp.rounding, gp.flags, gp.mask, ws.rounding, ws.flags, ws.mask);
}

void
check_get(Stat &st, int (port_fn)(), int (*ref_fn)(), const State &init)
{

	st.cases++;
	set_ref(init);
	set_port(init);

	int got = port_fn();
	int want = ref_fn();
	State gs = get_ref();
	State gp = get_port();

	if (got != want || !states_equal(gs, gp) || !states_equal(gs, init))
		st.fails++;
	else if (got != want)
		report(st, "ret", 0, got, want, gs, init, gp, init);
	else if (!states_equal(gs, gp))
		report(st, "state", 0, got, want, gs, init, gp, init);
}

void
check_set(Stat &st, int (port_fn)(int), int (*ref_fn)(int), const State &init,
    int arg)
{

	st.cases++;
	set_ref(init);
	set_port(init);

	int got = port_fn(arg);
	int want = ref_fn(arg);
	State gs = get_ref();
	State gp = get_port();
	State want_state = init;

	if (st.name == st_mask.name)
		want_state.mask = arg;
	else
		want_state.rounding = arg;

	if (got != want || !states_equal(gs, gp) || !states_equal(gs, want_state))
		st.fails++;
	else if (got != want)
		report(st, "ret", arg, got, want, gs, want_state, gp, want_state);
	else if (!states_equal(gs, gp))
		report(st, "state", arg, got, want, gs, want_state, gp,
		    want_state);
}

const int edge_vals[] = {
	0,
	1,
	2,
	3,
	4,
	8,
	16,
	31,
	32,
	63,
	127,
	128,
	255,
	256,
	0x7fff,
	0x8000,
	0xffff,
	0x7fffffff,
	0x80000000,
	-1,
	-2,
	-128,
	-129,
	0x7fffffff,
	0x80000000,
	port::float_round_nearest_even,
	port::float_round_to_zero,
	port::float_round_down,
	port::float_round_up,
	port::float_flag_inexact,
	port::float_flag_underflow,
	port::float_flag_overflow,
	port::float_flag_divbyzero,
	port::float_flag_invalid,
	port::float_flag_inexact | port::float_flag_underflow,
	port::float_flag_overflow | port::float_flag_divbyzero |
	    port::float_flag_invalid,
	0x55,
	0xaa,
	0x80,
	0xff,
	0x7f,
};

int
rand_int()
{
	std::uint64_t r = prng_next();
	unsigned u = static_cast<unsigned>(r);
	int sign = (r & 1) ? -1 : 1;

	switch ((r >> 1) & 15) {
	case 0:
		return 0;
	case 1:
		return static_cast<int>(u & 0xffu);
	case 2:
		return static_cast<int>(0x80u | (u & 0x7fu));
	case 3:
		return static_cast<int>(u & 0xffffu);
	case 4:
		return static_cast<int>(0x8000u | (u & 0x7fffu));
	case 5:
		return static_cast<int>(u & 3u);
	case 6:
		return static_cast<int>((u & 31u) | 1u);
	case 7:
		return sign * static_cast<int>(u & 0x7fu);
	case 8:
		return sign * static_cast<int>(0x80u | (u & 0x7fu));
	case 9:
		return static_cast<int>(r >> 32);
	case 10:
		return static_cast<int>(0x7fffffff);
	case 11:
		return static_cast<int>(0x80000000u);
	case 12:
		return -1;
	default:
		return static_cast<int>(u ^ static_cast<unsigned>(r >> 32));
	}
}

void
edge_fpgetsticky()
{
	for (int flags : edge_vals) {
		for (int rounding : edge_vals) {
			for (int mask : edge_vals) {
				State init{rounding, flags, mask};
				check_get(st_sticky, port::fpgetsticky,
				    ref_fpgetsticky, init);
			}
		}
	}
}

void
edge_fpgetround()
{
	for (int flags : edge_vals) {
		for (int rounding : edge_vals) {
			for (int mask : edge_vals) {
				State init{rounding, flags, mask};
				check_get(st_round_get, port::fpgetround,
				    ref_fpgetround, init);
			}
		}
	}
}

void
edge_fpsetmask()
{
	for (int flags : edge_vals) {
		for (int rounding : edge_vals) {
			for (int old_mask : edge_vals) {
				for (int new_mask : edge_vals) {
					State init{rounding, flags, old_mask};
					check_set(st_mask, port::fpsetmask,
					    ref_fpsetmask, init, new_mask);
				}
			}
		}
	}
}

void
edge_fpsetround()
{
	for (int flags : edge_vals) {
		for (int old_round : edge_vals) {
			for (int mask : edge_vals) {
				for (int new_round : edge_vals) {
					State init{old_round, flags, mask};
					check_set(st_round_set,
					    port::fpsetround, ref_fpsetround,
					    init, new_round);
				}
			}
		}
	}
}

void
random_fpgetsticky()
{
	for (long i = 0; i < RANDOM_PER_FN; i++) {
		State init{rand_int(), rand_int(), rand_int()};
		check_get(st_sticky, port::fpgetsticky, ref_fpgetsticky, init);
	}
}

void
random_fpgetround()
{
	for (long i = 0; i < RANDOM_PER_FN; i++) {
		State init{rand_int(), rand_int(), rand_int()};
		check_get(st_round_get, port::fpgetround, ref_fpgetround, init);
	}
}

void
random_fpsetmask()
{
	for (long i = 0; i < RANDOM_PER_FN; i++) {
		State init{rand_int(), rand_int(), rand_int()};
		check_set(st_mask, port::fpsetmask, ref_fpsetmask, init,
		    rand_int());
	}
}

void
random_fpsetround()
{
	for (long i = 0; i < RANDOM_PER_FN; i++) {
		State init{rand_int(), rand_int(), rand_int()};
		check_set(st_round_set, port::fpsetround, ref_fpsetround, init,
		    rand_int());
	}
}

void
print_stat(const Stat &st)
{

	std::printf("%-12s %8ld %8ld\n", st.name, st.cases, st.fails);
}

} /* namespace */

int
main()
{

	prng_seed(0xB00520026ULL);

	edge_fpgetsticky();
	edge_fpgetround();
	edge_fpsetmask();
	edge_fpsetround();

	random_fpgetsticky();
	random_fpgetround();
	random_fpsetmask();
	random_fpsetround();

	std::printf("function        cases  failures\n");
	print_stat(st_sticky);
	print_stat(st_round_get);
	print_stat(st_mask);
	print_stat(st_round_set);
	std::printf("total          %8ld\n",
	    st_sticky.cases + st_round_get.cases + st_mask.cases +
	    st_round_set.cases);
	std::printf("random sweep   %8ld (>= %ld required)\n", RANDOM_TOTAL,
	    200000L);

	long total_fails = st_sticky.fails + st_round_get.fails + st_mask.fails +
	    st_round_set.fails;

	return total_fails == 0 ? 0 : 1;
}
