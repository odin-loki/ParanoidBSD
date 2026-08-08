/*
 * Differential test for batch b0221 (lldiv, imaxdiv).
 */

#include <cstdarg>
#include <climits>
#include <cinttypes>
#include <cstdio>
#include <cstdlib>

import pbsd.lib.libc.stdlib.b0221;

namespace P = pbsd::lib_libc_stdlib::b0221;

extern "C" {
lldiv_t ref_lldiv(long long numer, long long denom);
imaxdiv_t ref_imaxdiv(intmax_t numer, intmax_t denom);
}

namespace {

enum Fn {
	F_LLDIV,
	F_IMAXDIV,
	F_COUNT
};

static const char *const fn_name[F_COUNT] = {
	"lldiv",
	"imaxdiv",
};

static unsigned long long n_cases[F_COUNT];
static unsigned long long n_fails[F_COUNT];
static unsigned long long reported[F_COUNT];

static const unsigned SWEEP_ITERS = 200000u;

static void
record_case(int fn, bool ok, const char *fmt, ...)
{
	n_cases[fn]++;
	if (ok)
		return;
	n_fails[fn]++;
	if (reported[fn] < 10) {
		reported[fn]++;
		va_list ap;
		va_start(ap, fmt);
		std::fprintf(stderr, "FAIL %s: ", fn_name[fn]);
		std::vfprintf(stderr, fmt, ap);
		va_end(ap);
		std::fputc('\n', stderr);
	} else if (reported[fn] == 10) {
		reported[fn]++;
		std::fprintf(stderr, "FAIL %s: (further failures suppressed)\n",
		    fn_name[fn]);
	}
}

static uint64_t rng_state = 0xb0221deadbeefULL;

static uint64_t
nextr(void)
{
	rng_state ^= rng_state << 13;
	rng_state ^= rng_state >> 7;
	rng_state ^= rng_state << 17;
	return rng_state;
}

static long long
rnd_ll(void)
{
	uint64_t v = nextr();
	return (long long)(v ^ (nextr() << 1));
}

static intmax_t
rnd_imax(void)
{
	return (intmax_t)rnd_ll();
}

static bool
lldiv_ub(long long numer, long long denom)
{
	return denom == 0 ||
	    (numer == LLONG_MIN && denom == -1);
}

static bool
imaxdiv_ub(intmax_t numer, intmax_t denom)
{
	return denom == 0 ||
	    (numer == INTMAX_MIN && denom == -1);
}

static void
case_lldiv(long long numer, long long denom)
{
	lldiv_t rp, rr;
	bool ok;

	if (lldiv_ub(numer, denom))
		return;

	rp = P::lldiv(numer, denom);
	rr = ref_lldiv(numer, denom);
	ok = rp.quot == rr.quot && rp.rem == rr.rem;
	record_case(F_LLDIV, ok,
	    "numer=%lld denom=%lld port=(%lld,%lld) ref=(%lld,%lld)",
	    numer, denom, rp.quot, rp.rem, rr.quot, rr.rem);
}

static void
case_imaxdiv(intmax_t numer, intmax_t denom)
{
	imaxdiv_t rp, rr;
	bool ok;

	if (imaxdiv_ub(numer, denom))
		return;

	rp = P::imaxdiv(numer, denom);
	rr = ref_imaxdiv(numer, denom);
	ok = rp.quot == rr.quot && rp.rem == rr.rem;
	record_case(F_IMAXDIV, ok,
	    "numer=%jd denom=%jd port=(%jd,%jd) ref=(%jd,%jd)",
	    (intmax_t)numer, (intmax_t)denom,
	    (intmax_t)rp.quot, (intmax_t)rp.rem,
	    (intmax_t)rr.quot, (intmax_t)rr.rem);
}

static void
test_lldiv_edges(void)
{
	static const long long nums[] = {
		0LL,
		1LL,
		-1LL,
		2LL,
		-2LL,
		7LL,
		-7LL,
		127LL,
		-127LL,
		128LL,
		-128LL,
		255LL,
		-255LL,
		256LL,
		-256LL,
		LLONG_MAX,
		LLONG_MIN,
		LLONG_MAX - 1,
		LLONG_MIN + 1,
	};
	static const long long dens[] = {
		1LL,
		-1LL,
		2LL,
		-2LL,
		3LL,
		-3LL,
		7LL,
		-7LL,
		127LL,
		-127LL,
		128LL,
		-128LL,
		255LL,
		-255LL,
		256LL,
		-256LL,
		LLONG_MAX,
		LLONG_MIN + 1,
	};
	unsigned i, j;

	for (i = 0; i < sizeof(nums) / sizeof(nums[0]); i++) {
		for (j = 0; j < sizeof(dens) / sizeof(dens[0]); j++)
			case_lldiv(nums[i], dens[j]);
	}

	case_lldiv(0LL, 1LL);
	case_lldiv(0LL, -1LL);
	case_lldiv(0LL, 42LL);
	case_lldiv(5LL, 5LL);
	case_lldiv(-5LL, 5LL);
	case_lldiv(5LL, -5LL);
	case_lldiv(-5LL, -5LL);
	case_lldiv(6LL, 4LL);
	case_lldiv(-6LL, 4LL);
	case_lldiv(6LL, -4LL);
	case_lldiv(-6LL, -4LL);
	case_lldiv(7LL, 3LL);
	case_lldiv(-7LL, 3LL);
	case_lldiv(7LL, -3LL);
	case_lldiv(-7LL, -3LL);
	case_lldiv(LLONG_MAX, 1LL);
	case_lldiv(LLONG_MIN, 1LL);
	case_lldiv(LLONG_MAX, -1LL);
	case_lldiv(LLONG_MIN + 1, -1LL);
	case_lldiv(LLONG_MAX, LLONG_MAX);
	case_lldiv(LLONG_MIN, LLONG_MIN + 1);
	case_lldiv(1LL, LLONG_MAX);
	case_lldiv(-1LL, LLONG_MAX);
	case_lldiv(1LL, LLONG_MIN + 1);
}

static void
test_imaxdiv_edges(void)
{
	static const intmax_t nums[] = {
		0,
		1,
		-1,
		2,
		-2,
		7,
		-7,
		127,
		-127,
		128,
		-128,
		255,
		-255,
		256,
		-256,
		INTMAX_MAX,
		INTMAX_MIN,
		INTMAX_MAX - 1,
		INTMAX_MIN + 1,
	};
	static const intmax_t dens[] = {
		1,
		-1,
		2,
		-2,
		3,
		-3,
		7,
		-7,
		127,
		-127,
		128,
		-128,
		255,
		-255,
		256,
		-256,
		INTMAX_MAX,
		INTMAX_MIN + 1,
	};
	unsigned i, j;

	for (i = 0; i < sizeof(nums) / sizeof(nums[0]); i++) {
		for (j = 0; j < sizeof(dens) / sizeof(dens[0]); j++)
			case_imaxdiv(nums[i], dens[j]);
	}

	case_imaxdiv(0, 1);
	case_imaxdiv(0, -1);
	case_imaxdiv(0, 42);
	case_imaxdiv(5, 5);
	case_imaxdiv(-5, 5);
	case_imaxdiv(5, -5);
	case_imaxdiv(-5, -5);
	case_imaxdiv(6, 4);
	case_imaxdiv(-6, 4);
	case_imaxdiv(6, -4);
	case_imaxdiv(-6, -4);
	case_imaxdiv(7, 3);
	case_imaxdiv(-7, 3);
	case_imaxdiv(7, -3);
	case_imaxdiv(-7, -3);
	case_imaxdiv(INTMAX_MAX, 1);
	case_imaxdiv(INTMAX_MIN, 1);
	case_imaxdiv(INTMAX_MAX, -1);
	case_imaxdiv(INTMAX_MIN + 1, -1);
	case_imaxdiv(INTMAX_MAX, INTMAX_MAX);
	case_imaxdiv(INTMAX_MIN, INTMAX_MIN + 1);
	case_imaxdiv(1, INTMAX_MAX);
	case_imaxdiv(-1, INTMAX_MAX);
	case_imaxdiv(1, INTMAX_MIN + 1);
}

static void
test_lldiv_random(unsigned iters)
{
	unsigned i;

	for (i = 0; i < iters; i++) {
		long long numer = rnd_ll();
		long long denom = rnd_ll();

		if (denom == 0)
			denom = (long long)((nextr() % 1000u) + 1u);
		if (numer == LLONG_MIN && denom == -1)
			denom = 1;
		case_lldiv(numer, denom);
	}
}

static void
test_imaxdiv_random(unsigned iters)
{
	unsigned i;

	for (i = 0; i < iters; i++) {
		intmax_t numer = rnd_imax();
		intmax_t denom = rnd_imax();

		if (denom == 0)
			denom = (intmax_t)((nextr() % 1000u) + 1u);
		if (numer == INTMAX_MIN && denom == -1)
			denom = 1;
		case_imaxdiv(numer, denom);
	}
}

} /* namespace */

int
main(void)
{
	unsigned fn;
	unsigned long long total_cases = 0, total_fails = 0;

	test_lldiv_edges();
	test_imaxdiv_edges();
	test_lldiv_random(SWEEP_ITERS);
	test_imaxdiv_random(SWEEP_ITERS);

	std::printf("\nbatch b0221 differential results\n");
	std::printf("%-14s %12s %10s %s\n", "function", "cases", "failures",
	    "status");
	for (fn = 0; fn < F_COUNT; fn++) {
		std::printf("%-14s %12llu %10llu %s\n", fn_name[fn],
		    n_cases[fn], n_fails[fn],
		    n_fails[fn] == 0u ? "ok" : "FAIL");
		total_cases += n_cases[fn];
		total_fails += n_fails[fn];
	}
	std::printf("%-14s %12llu %10llu %s\n", "TOTAL", total_cases,
	    total_fails, total_fails == 0u ? "ok" : "FAIL");

	return total_fails == 0u ? 0 : 1;
}
