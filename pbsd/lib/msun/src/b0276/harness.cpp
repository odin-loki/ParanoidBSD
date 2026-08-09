// Differential test for PBSD batch b0276 (tan, tanh, nextafter).

import pbsd.lib.msun.src.b0276;

#include <cstdint>
#include <cstdio>
#include <cstring>

namespace P = pbsd::lib_msun_src::b0276;

extern "C" {
double ref_tan(double x);
double ref_tanh(double x);
double ref_nextafter(double x, double y);
}

static const int MAX_REPORT = 12;

struct Stat {
	const char *name;
	unsigned long long cases;
	unsigned long long failures;
	unsigned reported;
};

static Stat st_tan = { "tan", 0, 0, 0 };
static Stat st_tanh = { "tanh", 0, 0, 0 };
static Stat st_nextafter = { "nextafter", 0, 0, 0 };

static std::uint64_t rng_state = 0xb0276a5a5a5a5a5aull;

static std::uint64_t next_u64(void)
{
	std::uint64_t z = (rng_state += 0x9e3779b97f4a7c15ull);
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ull;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebull;
	return z ^ (z >> 31);
}

static std::uint32_t next_mod(std::uint32_t m)
{
	return (std::uint32_t)(next_u64() % m);
}

static std::uint64_t dbits(double d)
{
	std::uint64_t u;
	std::memcpy(&u, &d, sizeof(u));
	return u;
}

static double fromdbits(std::uint64_t u)
{
	double d;
	std::memcpy(&d, &u, sizeof(d));
	return d;
}

static bool same_double(double a, double b)
{
	return dbits(a) == dbits(b);
}

static void report_fail(Stat &st, const char *tag, std::uint64_t xb,
    std::uint64_t yb, std::uint64_t pb, std::uint64_t ob)
{
	if (st.reported++ >= MAX_REPORT)
		return;
	if (st.name == st_nextafter.name)
		std::printf("  FAIL %s [%s] x=%#018llx y=%#018llx port=%#018llx ref=%#018llx\n",
		    st.name, tag, (unsigned long long)xb, (unsigned long long)yb,
		    (unsigned long long)pb, (unsigned long long)ob);
	else
		std::printf("  FAIL %s [%s] x=%#018llx port=%#018llx ref=%#018llx\n",
		    st.name, tag, (unsigned long long)xb,
		    (unsigned long long)pb, (unsigned long long)ob);
}

static void check_tan(std::uint64_t xb, const char *tag)
{
	const double x = fromdbits(xb);
	const double got = P::tan(x);
	const double want = ref_tan(x);

	st_tan.cases++;
	if (same_double(got, want))
		return;
	st_tan.failures++;
	report_fail(st_tan, tag, xb, 0, dbits(got), dbits(want));
}

static void check_tanh(std::uint64_t xb, const char *tag)
{
	const double x = fromdbits(xb);
	const double got = P::tanh(x);
	const double want = ref_tanh(x);

	st_tanh.cases++;
	if (same_double(got, want))
		return;
	st_tanh.failures++;
	report_fail(st_tanh, tag, xb, 0, dbits(got), dbits(want));
}

static void check_nextafter(std::uint64_t xb, std::uint64_t yb, const char *tag)
{
	const double x = fromdbits(xb);
	const double y = fromdbits(yb);
	const double got = P::nextafter(x, y);
	const double want = ref_nextafter(x, y);

	st_nextafter.cases++;
	if (same_double(got, want))
		return;
	st_nextafter.failures++;
	report_fail(st_nextafter, tag, xb, yb, dbits(got), dbits(want));
}

static void check_both_signs_tan(std::uint64_t xb, const char *tag)
{
	check_tan(xb, tag);
	check_tan(xb ^ 0x8000000000000000ull, tag);
}

static void check_both_signs_tanh(std::uint64_t xb, const char *tag)
{
	check_tanh(xb, tag);
	check_tanh(xb ^ 0x8000000000000000ull, tag);
}

/* tan thresholds: 0x3e400000 (2**-27), 0x3fe921fb (pi/4), 0x7ff00000 */
static const std::uint64_t tan_boundaries[] = {
	0x0000000000000000ull,
	0x0000000000000001ull,
	0x000fffffffffffffull,
	0x0010000000000000ull,
	0x0010000000000001ull,
	0x3e3ffffffe000000ull,
	0x3e3fffffffffffffull,
	0x3e40000000000000ull,
	0x3e40000000000001ull,
	0x3e40000000000002ull,
	0x3fe921fb00000000ull,
	0x3fe921fb54442d18ull,
	0x3fe921fb54442d19ull,
	0x3fe921fb54442d1aull,
	0x3fe921fbffffffffull,
	0x3fe921fc00000000ull,
	0x3ff0000000000000ull,
	0x400921fb54442d18ull,
	0x401921fb54442d18ull,
	0x403e000000000000ull,
	0x7fefffffffffffffull,
	0x7ff0000000000000ull,
	0x7ff0000000000001ull,
	0x7ff8000000000000ull,
	0x7fffffffffffffffull,
};

/* tanh thresholds: 0x3e300000 (2**-28), 0x3ff00000 (1), 0x40360000 (22) */
static const std::uint64_t tanh_boundaries[] = {
	0x0000000000000000ull,
	0x0000000000000001ull,
	0x000fffffffffffffull,
	0x0010000000000000ull,
	0x3e2ffffffe000000ull,
	0x3e2fffffffffffffull,
	0x3e30000000000000ull,
	0x3e30000000000001ull,
	0x3fefffffffffffffull,
	0x3ff0000000000000ull,
	0x3ff0000000000001ull,
	0x4035fffffffffffffULL,
	0x4036000000000000ull,
	0x4036000000000001ull,
	0x7fefffffffffffffull,
	0x7ff0000000000000ull,
	0x7ff8000000000000ull,
	0x7fffffffffffffffull,
};

static const std::uint64_t dvec[] = {
	0x0000000000000000ull, 0x8000000000000000ull,
	0x0000000000000001ull, 0x8000000000000001ull,
	0x0000000000000080ull, 0x8000000000000080ull,
	0x000fffffffffffffull, 0x800fffffffffffffull,
	0x0010000000000000ull, 0x8010000000000000ull,
	0x0010000000000001ull, 0x8010000000000001ull,
	0x3e40000000000000ull, 0xbe40000000000000ull,
	0x3e3fffffffffffffull, 0xbe3fffffffffffffull,
	0x3e30000000000000ull, 0xbe30000000000000ull,
	0x3e2fffffffffffffull, 0xbe2fffffffffffffull,
	0x3fe921fb54442d18ull, 0xbfe921fb54442d18ull,
	0x3fe921fb54442d19ull, 0xbfe921fb54442d19ull,
	0x3fe921fbffffffffull, 0xbfe921fbffffffffull,
	0x3fe921fc00000000ull, 0xbfe921fc00000000ull,
	0x3ff0000000000000ull, 0xbff0000000000000ull,
	0x3ff0000000000001ull, 0xbff0000000000001ull,
	0x4035fffffffffffffULL, 0xc035fffffffffffffULL,
	0x4036000000000000ull, 0xc036000000000000ull,
	0x4036000000000001ull, 0xc036000000000001ull,
	0x400921fb54442d18ull, 0xc00921fb54442d18ull,
	0x401921fb54442d18ull, 0xc01921fb54442d18ull,
	0x4024000000000000ull, 0xc024000000000000ull,
	0x40490fdb00000000ull, 0xc0490fdb00000000ull,
	0x4080000000000000ull, 0xc080000000000000ull,
	0x40e0000000000000ull, 0xc0e0000000000000ull,
	0x41e0000000000000ull, 0xc1e0000000000000ull,
	0x7fefffffffffffffull, 0xffefffffffffffffull,
	0x7ff0000000000000ull, 0xfff0000000000000ull,
	0x7ff0000000000001ull, 0xfff0000000000001ull,
	0x7ff8000000000000ull, 0xfff8000000000000ull,
	0x7ff8000000000001ull, 0xfff8000000000001ull,
	0x7fffffffffffffffull, 0xffffffffffffffffull,
	0x8080808080808080ull, 0x7f7f7f7f7f7f7f7full,
	0x00ff00ff00ff00ffull, 0xff00ff00ff00ff00ull,
};

static std::uint64_t rand_dbits(void)
{
	std::uint64_t r = next_u64();
	switch (next_mod(14)) {
	case 0:
		return r;
	case 1:
		return r & 0x800fffffffffffffull;
	case 2:
		return (r & 0x800fffffffffffffull) | 0x7ff0000000000000ull;
	case 3: {
		std::uint64_t d = next_mod(11) - 5u;
		return (r & 0x8000000000000000ull) |
		    ((0x7ff0000000000000ull + d) & 0x7fffffffffffffffull);
	}
	case 4: {
		std::uint64_t d = next_mod(11) - 5u;
		return (r & 0x8000000000000000ull) |
		    ((0x0010000000000000ull + d) & 0x7fffffffffffffffull);
	}
	case 5: {
		std::uint64_t e = 970ull + next_mod(61);
		return (r & 0x8000000000000000ull) | (e << 52) |
		    (r & 0x000fffffffffffffull);
	}
	case 6:
		return 0x3e40000000000000ull ^ (r & 0x000fffffffffffffull);
	case 7:
		return 0x3fe921fb54442d18ull ^ (r & 0x000fffffffffffffull);
	case 8:
		return 0x3e30000000000000ull ^ (r & 0x000fffffffffffffull);
	case 9:
		return 0x4036000000000000ull ^ (r & 0x000fffffffffffffull);
	case 10: {
		std::uint64_t e = (next_u64() & 1u) ? next_mod(8u)
						    : 2040ull + next_mod(8u);
		return (r & 0x8000000000000000ull) | (e << 52) |
		    (r & 0x000fffffffffffffull);
	}
	case 11:
		return 0x3ff0000000000000ull ^ (r & 0x000fffffffffffffull);
	case 12:
		return 0x7ff8000000000000ull | (r & 0x000fffffffffffffull);
	default:
		return r ^ next_u64();
	}
}

static void hand_written_cases(void)
{
	unsigned i, j, b;

	for (i = 0; i < sizeof dvec / sizeof dvec[0]; i++) {
		check_both_signs_tan(dvec[i] & 0x7fffffffffffffffull, "dvec");
		check_both_signs_tanh(dvec[i] & 0x7fffffffffffffffull, "dvec");
	}

	for (b = 0; b < sizeof tan_boundaries / sizeof tan_boundaries[0]; b++) {
		const std::uint64_t base = tan_boundaries[b];
		for (int d = -1024; d <= 1024; d++) {
			const std::int64_t v = (std::int64_t)base + d;
			if (v < 0 || v > 0x7fffffffffffffffll)
				continue;
			check_both_signs_tan((std::uint64_t)v, "tan-walk");
		}
	}

	for (b = 0; b < sizeof tanh_boundaries / sizeof tanh_boundaries[0]; b++) {
		const std::uint64_t base = tanh_boundaries[b];
		for (int d = -1024; d <= 1024; d++) {
			const std::int64_t v = (std::int64_t)base + d;
			if (v < 0 || v > 0x7fffffffffffffffll)
				continue;
			check_both_signs_tanh((std::uint64_t)v, "tanh-walk");
		}
	}

	for (unsigned expv = 0; expv < 2048; expv++) {
		for (unsigned s = 0; s < 2; s++) {
			std::uint64_t m = ((std::uint64_t)(s & 1u) << 63) |
			    ((std::uint64_t)expv << 52) |
			    (0x80808080ull << 20) | 0x80808080ull;
			check_both_signs_tan(m & 0x7fffffffffffffffull, "exp-sweep");
			check_both_signs_tanh(m & 0x7fffffffffffffffull, "exp-sweep");
		}
	}

	for (unsigned bit = 0; bit < 64; bit++) {
		std::uint64_t m = 1ull << bit;
		check_both_signs_tan(m, "bit");
		check_both_signs_tanh(m, "bit");
		check_tan(~m, "bit-not");
		check_tanh(~m, "bit-not");
	}

	for (i = 0; i < sizeof dvec / sizeof dvec[0]; i++) {
		for (j = 0; j < sizeof dvec / sizeof dvec[0]; j++)
			check_nextafter(dvec[i], dvec[j], "cross");
	}

	static const std::uint64_t na_pairs[][2] = {
		{ 0x7ff8000000000000ull, 0x0000000000000000ull },
		{ 0xfff8000000000000ull, 0x3ff0000000000000ull },
		{ 0x3ff0000000000000ull, 0x7ff8000000000000ull },
		{ 0x7ff0000000000000ull, 0x7ff8000000000000ull },
	};
	for (i = 0; i < sizeof na_pairs / sizeof na_pairs[0]; i++)
		check_nextafter(na_pairs[i][0], na_pairs[i][1], "nan");

	for (i = 0; i < sizeof dvec / sizeof dvec[0]; i++) {
		check_nextafter(dvec[i], dvec[i], "x-eq-y");
		check_nextafter(0x0000000000000000ull, dvec[i], "from-zero");
		check_nextafter(0x8000000000000000ull, dvec[i], "from-negzero");
		check_nextafter(dvec[i], 0x0000000000000000ull, "toward-zero");
		check_nextafter(dvec[i], 0x7ff0000000000000ull, "toward-inf");
		check_nextafter(dvec[i], 0xfff0000000000000ull, "toward-neginf");
	}

	for (std::uint64_t e = 0; e < 2048; e++) {
		std::uint64_t xb = (e << 52) | 0x00123456789abcull;
		std::uint64_t yb = ((e + 1) << 52) | 0x00fedcba987654ull;
		check_nextafter(xb, yb, "exp-pair");
		check_nextafter(xb ^ 0x8000000000000000ull, yb, "exp-pair-negx");
		check_nextafter(xb, yb ^ 0x8000000000000000ull, "exp-pair-negy");
	}
}

static void random_sweep(void)
{
	long long n;

	for (n = 0; n < 220000; n++) {
		const std::uint64_t xb = rand_dbits();
		check_tan(xb, "rand");
		check_tanh(xb, "rand");
	}

	for (n = 0; n < 220000; n++) {
		const std::uint64_t xb = rand_dbits();
		const std::uint64_t yb = rand_dbits();
		check_nextafter(xb, yb, "rand");
	}

	for (n = 0; n < 40000; n++) {
		const std::uint64_t base =
		    tan_boundaries[next_mod((std::uint32_t)(sizeof tan_boundaries /
			sizeof tan_boundaries[0]))];
		const int d = (int)next_mod(4097) - 2048;
		const std::int64_t v = (std::int64_t)base + d;
		if (v >= 0 && v <= 0x7fffffffffffffffll)
			check_both_signs_tan((std::uint64_t)v, "rand-tan-bd");
	}

	for (n = 0; n < 40000; n++) {
		const std::uint64_t base =
		    tanh_boundaries[next_mod((std::uint32_t)(sizeof tanh_boundaries /
			sizeof tanh_boundaries[0]))];
		const int d = (int)next_mod(4097) - 2048;
		const std::int64_t v = (std::int64_t)base + d;
		if (v >= 0 && v <= 0x7fffffffffffffffll)
			check_both_signs_tanh((std::uint64_t)v, "rand-tanh-bd");
	}
}

int main(void)
{
	hand_written_cases();
	random_sweep();

	Stat *all[] = { &st_tan, &st_tanh, &st_nextafter };
	const unsigned nall = (unsigned)(sizeof all / sizeof all[0]);
	unsigned long long total_cases = 0;
	unsigned long long total_fails = 0;

	std::printf("\n%-12s %14s %14s\n", "function", "cases", "failures");
	std::printf("%-12s %14s %14s\n", "------------", "--------------",
	    "--------------");
	for (unsigned k = 0; k < nall; k++) {
		std::printf("%-12s %14llu %14llu\n", all[k]->name,
		    all[k]->cases, all[k]->failures);
		total_cases += all[k]->cases;
		total_fails += all[k]->failures;
	}
	std::printf("%-12s %14s %14s\n", "------------", "--------------",
	    "--------------");
	std::printf("%-12s %14llu %14llu\n", "TOTAL", total_cases,
	    total_fails);

	int bad = 0;
	for (unsigned k = 0; k < nall; k++) {
		if (all[k]->cases == 0) {
			std::printf("%s: no cases were run\n", all[k]->name);
			bad = 1;
		}
		if (all[k]->failures != 0)
			bad = 1;
	}

	std::printf("%s\n", bad ? "FAIL" : "PASS");
	return bad ? 1 : 0;
}
