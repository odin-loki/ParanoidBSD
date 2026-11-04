/*
 * harness.cpp -- differential test for PBSD batch b0339.
 *
 * fpsetsticky is a thin accessor over the float_exception_flags softfloat
 * global.  Every case sets identical initial state on both the C++ port and
 * the ref_ oracle, invokes the matching entry point on each side, and compares
 * the return value plus the full flags state afterwards so a port that returns
 * the right value but writes the wrong global is caught.
 */

import pbsd.lib.libc.softfloat.b0339;

#include <cstdint>
#include <cstdio>
#include <cstdlib>

namespace port = pbsd::lib_libc_softfloat::b0339;

extern "C" {
int ref_fpsetsticky(int except);
extern int float_exception_flags;
}

namespace {

constexpr int MAX_REPORT = 12;
constexpr long RANDOM_CASES = 10000;

struct Stat {
	const char *name;
	long cases;
	long fails;
	int reported;
};

Stat st_sticky{"fpsetsticky", 0, 0, 0};

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
set_ref(int flags)
{

	float_exception_flags = flags;
}

int
get_ref()
{

	return float_exception_flags;
}

void
set_port(int flags)
{

	port::float_exception_flags = flags;
}

int
get_port()
{

	return port::float_exception_flags;
}

void
report(Stat &st, const char *kind, int old_flags, int new_flags, int got,
    int want, int ref_flags, int port_flags)
{

	if (st.reported >= MAX_REPORT)
		return;
	st.reported++;
	std::fprintf(stderr,
	    "%s %s old=%d new=%d ret %d vs %d  ref_flags=%d port_flags=%d\n",
	    st.name, kind, old_flags, new_flags, got, want, ref_flags,
	    port_flags);
}

void
check_fpsetsticky(int old_flags, int new_flags)
{

	st_sticky.cases++;
	set_ref(old_flags);
	set_port(old_flags);

	int got = port::fpsetsticky(new_flags);
	int want = ref_fpsetsticky(new_flags);
	int ref_flags = get_ref();
	int port_flags = get_port();

	if (got != want || ref_flags != port_flags || ref_flags != new_flags) {
		st_sticky.fails++;
		if (got != want)
			report(st_sticky, "ret", old_flags, new_flags, got, want,
			    ref_flags, port_flags);
		else
			report(st_sticky, "state", old_flags, new_flags, got,
			    want, ref_flags, port_flags);
	}
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
	static_cast<int>(0x80000000u),
	-1,
	-2,
	-128,
	-129,
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
edge_fpsetsticky()
{
	for (int old_flags : edge_vals) {
		for (int new_flags : edge_vals)
			check_fpsetsticky(old_flags, new_flags);
	}
}

void
random_fpsetsticky()
{
	for (long i = 0; i < RANDOM_CASES; i++)
		check_fpsetsticky(rand_int(), rand_int());
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

	prng_seed(0xB03390026ULL);

	edge_fpsetsticky();
	random_fpsetsticky();

	std::printf("function        cases  failures\n");
	print_stat(st_sticky);
	std::printf("random sweep   %8ld (>= %ld required)\n", RANDOM_CASES,
	    10000L);

	return st_sticky.fails == 0 ? 0 : 1;
}
