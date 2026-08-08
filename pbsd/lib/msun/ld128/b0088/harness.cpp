/*
 * Differential test harness for PBSD batch b0088.
 *
 * Compares cospil(), sinpil(), tanpil(), and cexpl() against the ref_
 * oracle.  Long double and complex results are compared bit-for-bit.
 */

#include <bit>
#include <ccomplex>
#include <cfloat>
#include <climits>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

import pbsd.lib.msun.ld128.b0088;

namespace P = pbsd::lib_msun_ld128::b0088;

extern "C" {
long double ref_cospil(long double x);
long double ref_sinpil(long double x);
long double ref_tanpil(long double x);
long double complex ref_cexpl(long double complex z);
}

#if LDBL_MANT_DIG == 113
static const std::size_t LD_SIG = sizeof(long double);
#else
static const std::size_t LD_SIG = sizeof(long double);
#endif

static const unsigned long RANDOM_ITERS = 200000UL;
static const int MAX_REPORT = 8;
static const unsigned char GUARD = 0x7f;

struct Stat {
	const char *name;
	unsigned long long cases;
	unsigned long long failures;
	unsigned reported;
};

static Stat st_cospil = { "cospil", 0, 0, 0 };
static Stat st_sinpil = { "sinpil", 0, 0, 0 };
static Stat st_tanpil = { "tanpil", 0, 0, 0 };
static Stat st_cexpl = { "cexpl", 0, 0, 0 };

static std::uint64_t rng_state = 0xB0088ULL;

static std::uint64_t
rnd64(void)
{
	std::uint64_t z = (rng_state += 0x9E3779B97F4A7C15ULL);
	z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
	z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
	return z ^ (z >> 31);
}

static bool
ld_same(long double a, long double b)
{
	unsigned char ba[sizeof(long double)], bb[sizeof(long double)];

	std::memset(ba, 0, sizeof(ba));
	std::memset(bb, 0, sizeof(bb));
	std::memcpy(ba, &a, sizeof(a));
	std::memcpy(bb, &b, sizeof(b));
	return std::memcmp(ba, bb, LD_SIG) == 0;
}

static bool
cx_same(long double complex a, long double complex b)
{
	return ld_same(creall(a), creall(b)) && ld_same(cimagl(a), cimagl(b));
}

static void
print_ld(long double v)
{
	unsigned char b[sizeof(long double)];
	std::size_t i;

	std::memset(b, 0, sizeof(b));
	std::memcpy(b, &v, sizeof(v));
	for (i = LD_SIG; i-- > 0;)
		std::printf("%02x", b[i]);
}

static long double
mkld(std::uint64_t manh, std::uint64_t manl, std::uint16_t expsign)
{
	unsigned char b[sizeof(long double)];
	long double x;

	std::memset(b, 0, sizeof(b));
	std::memcpy(b, &manl, sizeof(manl));
	std::memcpy(b + 8, &manh, sizeof(manh));
	std::memcpy(b + 14, &expsign, sizeof(expsign));
	std::memcpy(&x, b, sizeof(x));
	return x;
}

union GuardSlot {
	long double v;
	unsigned char b[sizeof(long double)];
};

static GuardSlot port_x[8], ref_x[8];
static const int SLOT = 3;

static void
pad_fill(GuardSlot *pad, long double x)
{
	int i;

	for (i = 0; i < 8; i++)
		std::memset(pad[i].b, GUARD, sizeof(pad[i].b));
	pad[SLOT].v = x;
}

static bool
pad_ok(const GuardSlot *pad, long double x)
{
	unsigned char g[sizeof(long double)];
	int i;

	std::memset(g, GUARD, sizeof(g));
	for (i = 0; i < 8; i++) {
		if (i == SLOT) {
			if (!ld_same(pad[i].v, x))
				return false;
			continue;
		}
		if (std::memcmp(pad[i].b, g, sizeof(g)) != 0)
			return false;
	}
	return true;
}

static void
report_ld_fail(Stat &s, const char *tag, long double got, long double want)
{
	if (s.reported >= MAX_REPORT)
		return;
	s.reported++;
	std::printf("  %s FAIL [%s] port=", s.name, tag);
	print_ld(got);
	std::printf(" ref=");
	print_ld(want);
	std::printf("\n");
}

static void
report_cx_fail(Stat &s, const char *tag,
    long double complex got, long double complex want)
{
	if (s.reported >= MAX_REPORT)
		return;
	s.reported++;
	std::printf("  %s FAIL [%s] port=(", s.name, tag);
	print_ld(creall(got));
	std::printf(",");
	print_ld(cimagl(got));
	std::printf(") ref=(");
	print_ld(creall(want));
	std::printf(",");
	print_ld(cimagl(want));
	std::printf(")\n");
}

static void
check_cospil(long double x)
{
	long double got, want;
	bool bad;

	st_cospil.cases++;

	pad_fill(port_x, x);
	pad_fill(ref_x, x);

	got = P::cospil(port_x[SLOT].v);
	want = ref_cospil(ref_x[SLOT].v);

	bad = !ld_same(got, want);
	if (!pad_ok(port_x, x) || !pad_ok(ref_x, x))
		bad = true;
	if (std::memcmp(port_x, ref_x, sizeof(port_x)) != 0)
		bad = true;

	if (bad) {
		st_cospil.failures++;
		report_ld_fail(st_cospil, "value", got, want);
	}
}

static void
check_sinpil(long double x)
{
	long double got, want;
	bool bad;

	st_sinpil.cases++;

	pad_fill(port_x, x);
	pad_fill(ref_x, x);

	got = P::sinpil(port_x[SLOT].v);
	want = ref_sinpil(ref_x[SLOT].v);

	bad = !ld_same(got, want);
	if (!pad_ok(port_x, x) || !pad_ok(ref_x, x))
		bad = true;
	if (std::memcmp(port_x, ref_x, sizeof(port_x)) != 0)
		bad = true;

	if (bad) {
		st_sinpil.failures++;
		report_ld_fail(st_sinpil, "value", got, want);
	}
}

static void
check_tanpil(long double x)
{
	long double got, want;
	bool bad;

	st_tanpil.cases++;

	pad_fill(port_x, x);
	pad_fill(ref_x, x);

	got = P::tanpil(port_x[SLOT].v);
	want = ref_tanpil(ref_x[SLOT].v);

	bad = !ld_same(got, want);
	if (!pad_ok(port_x, x) || !pad_ok(ref_x, x))
		bad = true;
	if (std::memcmp(port_x, ref_x, sizeof(port_x)) != 0)
		bad = true;

	if (bad) {
		st_tanpil.failures++;
		report_ld_fail(st_tanpil, "value", got, want);
	}
}

static void
check_cexpl(long double complex z)
{
	long double complex got, want;
	bool bad;

	st_cexpl.cases++;

	got = P::cexpl(z);
	want = ref_cexpl(z);

	bad = !cx_same(got, want);

	if (bad) {
		st_cexpl.failures++;
		report_cx_fail(st_cexpl, "value", got, want);
	}
}

static void
check_all_real(long double x)
{
	check_cospil(x);
	check_sinpil(x);
	check_tanpil(x);
}

static void
check_all_complex(long double x, long double y)
{
	check_cexpl(x + y * 1.0iL);
}

/* ---------------------------------------------------------------- */
/* hand-written edge cases                                          */
/* ---------------------------------------------------------------- */

static void
run_scalar_edges(void)
{
	static const long double xs[] = {
		0.0L, -0.0L,
		0x1p-120L, -0x1p-120L,
		0x1p-61L, -0x1p-61L,
		0x1p-60L, -0x1p-60L,
		0x1p-59L, -0x1p-59L,
		0x1p-113L, -0x1p-113L,
		0.25L - 0x1p-60L, 0.25L, 0.25L + 0x1p-60L,
		0.5L - 0x1p-60L, 0.5L, 0.5L + 0x1p-60L,
		0.75L - 0x1p-60L, 0.75L, 0.75L + 0x1p-60L,
		1.0L - 0x1p-60L, 1.0L, 1.0L + 0x1p-60L,
		-1.0L, 2.0L, -2.0L, 3.0L, -3.0L,
		0.125L, -0.125L, 0.375L, -0.375L, 0.625L, -0.625L,
		0.875L, -0.875L, 1.5L, -1.5L,
		0x1p111L, -0x1p111L,
		0x1p112L, -0x1p112L,
		0x1p112L + 0.5L, -0x1p112L - 0.5L,
		0x1p112L + 1.0L, -0x1p112L - 1.0L,
		0x1p113L, -0x1p113L,
		0x1p113L + 1.0L, -0x1p113L - 1.0L,
		0x1p114L, -0x1p114L,
		1.13565234062941439494919310779707649e+04L,
		1.13565234062941439494919310779707650e+04L,
		2.27892930024498818830197576893019292e+04L,
		2.27892930024498818830197576893019293e+04L,
		LDBL_MIN, -LDBL_MIN, LDBL_MAX, -LDBL_MAX,
		LDBL_EPSILON, -LDBL_EPSILON,
	};
	std::size_t i;

	for (i = 0; i < sizeof(xs) / sizeof(xs[0]); i++)
		check_all_real(xs[i]);

	check_all_real((long double)HUGE_VAL);
	check_all_real(-(long double)HUGE_VAL);
	check_all_real(std::nanl(""));
	check_all_real(-std::nanl(""));

	for (i = 1; i <= 20; i++) {
		check_all_real((long double)i);
		check_all_real(-(long double)i);
		check_all_real((long double)i + 0.5L);
		check_all_real(-(long double)i - 0.5L);
		check_all_real((long double)i + 0.25L);
		check_all_real((long double)i + 0.75L);
	}

	for (i = 0; i <= 64; i++) {
		long double t = std::ldexpl(1.0L, -i);

		check_all_real(t);
		check_all_real(-t);
		check_all_real(0.25L - t);
		check_all_real(0.25L + t);
		check_all_real(0.5L - t);
		check_all_real(0.5L + t);
		check_all_real(0.75L - t);
		check_all_real(0.75L + t);
		check_all_real(1.0L - t);
		check_all_real(1.0L + t);
	}
}

static void
run_complex_edges(void)
{
	static const long double vals[] = {
		0.0L, -0.0L, 1.0L, -1.0L, 0.5L, -0.5L,
		0x1p-60L, 0x1p60L, 0x1p112L, 0x1p113L,
		1.13565234062941439494919310779707649e+04L,
		1.13565234062941439494919310779707650e+04L,
		2.27892930024498818830197576893019292e+04L,
		2.27892930024498818830197576893019293e+04L,
		LDBL_MAX, -LDBL_MAX,
	};
	std::size_t i, j;

	check_cexpl(0.0L + 0.0iL);
	check_cexpl(1.0L + 0.0iL);
	check_cexpl(0.0L + 1.0iL);
	check_cexpl(-0.0L + 0.0iL);
	check_cexpl(0.0L - 0.0iL);

	for (i = 0; i < sizeof(vals) / sizeof(vals[0]); i++) {
		for (j = 0; j < sizeof(vals) / sizeof(vals[0]); j++)
			check_all_complex(vals[i], vals[j]);
	}

	check_cexpl((long double)HUGE_VAL + 0.0iL);
	check_cexpl(-(long double)HUGE_VAL + 0.0iL);
	check_cexpl(std::nanl("") + 0.0iL);
	check_cexpl(0.0L + (long double)HUGE_VAL * 1.0iL);
	check_cexpl(0.0L - (long double)HUGE_VAL * 1.0iL);
	check_cexpl(0.0L + std::nanl("") * 1.0iL);
	check_cexpl((long double)HUGE_VAL + (long double)HUGE_VAL * 1.0iL);
	check_cexpl(-(long double)HUGE_VAL + (long double)HUGE_VAL * 1.0iL);
	check_cexpl((long double)HUGE_VAL + std::nanl("") * 1.0iL);
	check_cexpl(std::nanl("") + (long double)HUGE_VAL * 1.0iL);
	check_cexpl(std::nanl("") + std::nanl("") * 1.0iL);
}

static void
run_ld128_bit_edges(void)
{
	static const std::uint16_t exps[] = {
		0x0000u, 0x0001u, 0x3ffeu, 0x3fffu, 0x4000u, 0x7ffeu, 0x7fffu,
	};
	static const std::uint64_t manhs[] = {
		0x000000000000ULL, 0x000000000001ULL,
		0x0000800000000000ULL, 0xffffffffffffULL,
	};
	static const std::uint64_t manls[] = {
		0x0000000000000000ULL, 0x0000000000000001ULL,
		0x8000000000000000ULL, 0xffffffffffffffffULL,
	};
	std::size_t e, mh, ml, s;

	for (e = 0; e < sizeof(exps) / sizeof(exps[0]); e++)
		for (mh = 0; mh < sizeof(manhs) / sizeof(manhs[0]); mh++)
			for (ml = 0; ml < sizeof(manls) / sizeof(manls[0]); ml++)
				for (s = 0; s < 2; s++) {
					long double x = mkld(
					    manhs[mh], manls[ml],
					    (std::uint16_t)(exps[e] |
					    (s ? 0x8000u : 0u)));
					check_all_real(x);
					check_cexpl(x + 0.0iL);
					check_cexpl(x + x * 1.0iL);
				}
}

/* ---------------------------------------------------------------- */
/* randomised sweeps                                                */
/* ---------------------------------------------------------------- */

static void
run_random(void)
{
	unsigned long i;
	std::uint64_t r;

	for (i = 0; i < RANDOM_ITERS; i++) {
		r = rnd64();
		switch (i & 15u) {
		case 0:
			check_all_real((long double)std::bit_cast<double>(r));
			break;
		case 1:
			check_all_real((long double)((std::int64_t)r) / 17.0L);
			break;
		case 2: {
			long double x = mkld(rnd64() & 0x0000ffffffffffffULL,
			    rnd64(),
			    (std::uint16_t)((r >> 40) & 0x7fffu |
			    ((r & 1u) ? 0x8000u : 0u)));
			check_all_real(x);
			check_cexpl(x + (long double)std::bit_cast<double>(
			    rnd64()) * 1.0iL);
			break;
		}
		case 3: {
			long double x = 0.25L + (long double)((r & 0xffffu) -
			    32768) / 65536.0L;
			check_all_real(x);
			break;
		}
		case 4:
			check_all_real(0.5L + (long double)((r & 0xffffu) -
			    32768) / 131072.0L);
			break;
		case 5:
			check_all_real(0.75L + (long double)((r & 0xffffu) -
			    32768) / 131072.0L);
			break;
		case 6:
			check_all_real(1.0L + (long double)((r & 0xffffu) -
			    32768) / 65536.0L);
			break;
		case 7:
			check_all_real(std::ldexpl(1.0L,
			    (int)((r >> 16) & 0x7fu) - 64));
			break;
		case 8:
			check_all_real(std::ldexpl(1.0L,
			    (int)((r >> 16) & 0xffu) + 100));
			break;
		case 9:
			check_cexpl((long double)std::bit_cast<double>(r) +
			    (long double)std::bit_cast<double>(rnd64()) *
			    1.0iL);
			break;
		case 10:
			check_cexpl(11356.0L + (long double)((r & 0xfffu)) /
			    16.0L + (long double)((rnd64() & 0xfffu)) *
			    1.0iL);
			break;
		case 11:
			check_cexpl(22789.0L + (long double)((r & 0xfffu)) /
			    16.0L + (long double)((rnd64() & 0xfffu)) *
			    1.0iL);
			break;
		case 12:
			check_all_real((long double)((r & 0x7ffu) + 1) *
			    0.25L);
			break;
		case 13:
			check_all_real(fmodl((long double)((std::int64_t)r),
			    2.0L));
			break;
		case 14:
			check_cexpl((long double)((std::int64_t)r) / 3.0L +
			    (long double)((std::int64_t)(rnd64())) / 7.0L *
			    1.0iL);
			break;
		default:
			check_all_real((long double)(std::int32_t)(r >> 32) /
			    5.0L);
			break;
		}
	}
}

/* ---------------------------------------------------------------- */

int
main(void)
{
	Stat *all[] = { &st_cospil, &st_sinpil, &st_tanpil, &st_cexpl };
	unsigned long long total_fail = 0;
	int i;

	std::printf("LDBL_MANT_DIG=%d, comparing %zu bytes of long double\n\n",
	    (int)LDBL_MANT_DIG, LD_SIG);

	run_scalar_edges();
	run_complex_edges();
	run_ld128_bit_edges();
	run_random();

	std::printf("\n%-10s %14s %12s  %s\n", "function", "cases", "failures",
	    "result");
	std::printf("---------------------------------------------------\n");
	for (i = 0; i < 4; i++) {
		std::printf("%-10s %14llu %12llu  %s\n", all[i]->name,
		    all[i]->cases, all[i]->failures,
		    all[i]->failures == 0 ? "PASS" : "FAIL");
		total_fail += all[i]->failures;
	}
	std::printf("---------------------------------------------------\n");
	std::printf("%-10s %14llu %12llu  %s\n", "TOTAL",
	    st_cospil.cases + st_sinpil.cases + st_tanpil.cases +
	    st_cexpl.cases,
	    total_fail, total_fail == 0 ? "PASS" : "FAIL");

	return total_fail == 0 ? 0 : 1;
}
