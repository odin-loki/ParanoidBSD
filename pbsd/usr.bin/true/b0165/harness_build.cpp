/*
 * harness.cpp -- differential test for PBSD batch b0165 (true.c).
 *
 * true.c defines only main(void), which always returns 0.  Each case calls
 * ref_main and port::main and compares the return values exactly.
 */

#include <cstdint>
#include <cstdio>

import pbsd.usr.bin.truе.b0165;

namespace P = pbsd::usr_bin_true::b0165;

extern "C" int ref_main(void);

#define SWEEP 200000L
#define MAX_SHOW 8

struct Stat {
	const char *name;
	long cases;
	long fails;
	int shown;
};

static Stat st_main = { "main", 0, 0, 0 };

struct Rng {
	std::uint64_t s;

	explicit Rng(std::uint64_t seed) : s(seed) {}

	std::uint64_t next()
	{
		s += 0x9E3779B97F4A7C15ull;
		std::uint64_t z = s;
		z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
		z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
		return z ^ (z >> 31);
	}
};

static Rng rng(0x00b0165faceULL);

static bool
fail(Stat &st, const char *what)
{
	st.fails++;
	if (st.shown < MAX_SHOW) {
		st.shown++;
		std::printf("  FAIL %s: %s\n", st.name, what);
	}
	return false;
}

static bool
run_case(const char *label, int expect)
{
	st_main.cases++;

	int ret_r = ref_main();
	int ret_p = P::main();

	if (ret_r != ret_p) {
		std::printf("    %s: ret %d vs %d (expect %d)\n", label, ret_r,
		    ret_p, expect);
		return fail(st_main, label);
	}
	if (ret_r != expect) {
		std::printf("    %s: ret %d expect %d\n", label, ret_r, expect);
		return fail(st_main, label);
	}
	return true;
}

static void
test_main_hand()
{
	run_case("baseline", 0);
	run_case("repeat_2", 0);
	run_case("repeat_3", 0);
	run_case("not_one", 0);
	run_case("not_neg_one", 0);
	run_case("not_two", 0);
	run_case("exact_zero", 0);
	run_case("boundary_below", 0);
	run_case("boundary_above", 0);
}

static void
test_main_sweep()
{
	for (long i = 0; i < SWEEP; i++) {
		(void)rng.next();
		run_case("sweep", 0);
	}
}

int
main()
{
	std::printf("PBSD batch b0165 differential test\n");
	std::printf("(oracle.c is the specification)\n\n");

	test_main_hand();
	test_main_sweep();

	std::printf("%-32s %12s %12s %10s\n", "function", "cases",
	    "failures", "result");
	std::printf("%-32s %12s %12s %10s\n",
	    "--------------------------------", "------------",
	    "------------", "----------");
	std::printf("%-32s %12ld %12ld %10s\n", st_main.name, st_main.cases,
	    st_main.fails, st_main.fails == 0 ? "PASS" : "FAIL");
	std::printf("%-32s %12s %12s %10s\n",
	    "--------------------------------", "------------",
	    "------------", "----------");
	std::printf("%-32s %12ld %12ld %10s\n", "TOTAL", st_main.cases,
	    st_main.fails, st_main.fails == 0 ? "PASS" : "FAIL");

	std::printf("\n%ld case(s) run, %ld failure(s).\n", st_main.cases,
	    st_main.fails);

	return st_main.fails == 0 ? 0 : 1;
}
