/*
 * Differential test harness for PBSD batch b0095.
 *
 * Compares the C++23 port against the ref_ oracle for powl(), logl(),
 * log1pl(), log10l(), and log2l().  Every case exercises both sides; long
 * double results are compared bit-for-bit (10 bytes on ld80).
 */

#include <bit>
#include <cfloat>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>

import pbsd.lib.msun.ld80.b0095;

namespace P = pbsd::lib_msun_ld80::b0095;

extern "C" {
long double ref_powl(long double x, long double y);
long double ref_logl(long double x);
long double ref_log1pl(long double x);
long double ref_log10l(long double x);
long double ref_log2l(long double x);
}

#if (defined(__i386__) || defined(__x86_64__)) && LDBL_MANT_DIG == 64
#define PBSD_LD80 1
static const std::size_t LD_BYTES = 10;
#else
static const std::size_t LD_BYTES = sizeof(long double);
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

static Stat st_powl = { "powl", 0, 0, 0 };
static Stat st_logl = { "logl", 0, 0, 0 };
static Stat st_log1pl = { "log1pl", 0, 0, 0 };
static Stat st_log10l = { "log10l", 0, 0, 0 };
static Stat st_log2l = { "log2l", 0, 0, 0 };

static std::uint64_t rng_state = 0xB0095C0FFEEULL;

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
	return std::memcmp(ba, bb, LD_BYTES) == 0;
}

static void
print_ld(long double v)
{
	unsigned char b[sizeof(long double)];
	std::size_t i;

	std::memset(b, 0, sizeof(b));
	std::memcpy(b, &v, sizeof(v));
	for (i = LD_BYTES; i-- > 0;)
		std::printf("%02x", b[i]);
}

static long double
ld_bits(std::uint64_t mant, unsigned exp15, unsigned sign)
{
	long double v;
	unsigned char b[sizeof(long double)];
	std::uint16_t se;

	std::memset(b, 0, sizeof(b));
	std::memcpy(b, &mant, sizeof(mant));
	se = (std::uint16_t)(((sign & 1u) << 15) | (exp15 & 0x7fffu));
	std::memcpy(b + 8, &se, sizeof(se));
	std::memcpy(&v, b, sizeof(v));
	return v;
}

union GuardSlot {
	long double v;
	unsigned char b[sizeof(long double)];
};

static GuardSlot port_x[8], port_y[8], ref_x[8], ref_y[8];
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
report_fail(Stat &s, const char *tag, long double got, long double want)
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
check_powl(long double x, long double y)
{
	long double got, want;
	bool bad;

	st_powl.cases++;

	pad_fill(port_x, x);
	pad_fill(port_y, y);
	pad_fill(ref_x, x);
	pad_fill(ref_y, y);

	got = P::powl(port_x[SLOT].v, port_y[SLOT].v);
	want = ref_powl(ref_x[SLOT].v, ref_y[SLOT].v);

	bad = !ld_same(got, want);
	if (!pad_ok(port_x, x) || !pad_ok(port_y, y))
		bad = true;
	if (!pad_ok(ref_x, x) || !pad_ok(ref_y, y))
		bad = true;
	if (std::memcmp(port_x, ref_x, sizeof(port_x)) != 0)
		bad = true;
	if (std::memcmp(port_y, ref_y, sizeof(port_y)) != 0)
		bad = true;

	if (bad) {
		st_powl.failures++;
		report_fail(st_powl, "value", got, want);
	}
}

static void
check_unary(Stat &s, long double x,
    long double (*port_fn)(long double),
    long double (*ref_fn)(long double))
{
	long double got, want;
	bool bad;

	s.cases++;

	pad_fill(port_x, x);
	pad_fill(ref_x, x);

	got = port_fn(port_x[SLOT].v);
	want = ref_fn(ref_x[SLOT].v);

	bad = !ld_same(got, want);
	if (!pad_ok(port_x, x) || !pad_ok(ref_x, x))
		bad = true;
	if (std::memcmp(port_x, ref_x, sizeof(port_x)) != 0)
		bad = true;

	if (bad) {
		s.failures++;
		report_fail(s, "value", got, want);
	}
}

static void
check_logl(long double x)
{
	check_unary(st_logl, x, P::logl, ref_logl);
}

static void
check_log1pl(long double x)
{
	check_unary(st_log1pl, x, P::log1pl, ref_log1pl);
}

static void
check_log10l(long double x)
{
	check_unary(st_log10l, x, P::log10l, ref_log10l);
}

static void
check_log2l(long double x)
{
	check_unary(st_log2l, x, P::log2l, ref_log2l);
}

static void
check_all_unary(long double x)
{
	check_logl(x);
	check_log1pl(x);
	check_log10l(x);
	check_log2l(x);
}

/* ---------------------------------------------------------------- */
/* hand-written edge cases                                          */
/* ---------------------------------------------------------------- */

static void
run_scalar_edges(void)
{
	static const long double xs[] = {
		0.0L, -0.0L, 1.0L, -1.0L, 2.0L, -2.0L, 0.5L, -0.5L,
		1.5L, -1.5L, 3.0L, -3.0L, 10.0L, -10.0L, 1000.0L, -1000.0L,
		0.99L, 1.01L, 0.999L, 1.001L, 0.001L, 1000.0L,
		1e-30L, -1e-30L, 1e-300L, -1e-300L, 1e30L, -1e30L,
		1e4000L, -1e4000L, 1e10000L, -1e10000L,
		LDBL_MIN, -LDBL_MIN, LDBL_MAX, -LDBL_MAX,
		LDBL_TRUE_MIN, -LDBL_TRUE_MIN, LDBL_EPSILON, -LDBL_EPSILON,
		2.0L, 0x1.0p-65L, 0x1.0p65L, 0x1.0p-1L, 0x1.0p1L,
		0x1.0p-32L, 0x1.0p32L, 0x1.0p-16382L, 0x1.0p16382L,
	};
	static const long double ys[] = {
		0.0L, -0.0L, 1.0L, -1.0L, 2.0L, -2.0L, 3.0L, -3.0L,
		0.5L, -0.5L, 10.0L, -10.0L, 1000.0L, -1000.0L,
		32767.0L, -32767.0L, 32768.0L, -32768.0L,
		2.5L, -2.5L, 3.5L, -3.5L, 0.25L, -0.25L,
		8700.0L, -8700.0L, 1e30L, -1e30L, 1e4000L, -1e4000L,
		LDBL_MAX, -LDBL_MAX, LDBL_MIN, -LDBL_MIN,
	};
	std::size_t i, j;

	for (i = 0; i < sizeof(xs) / sizeof(xs[0]); i++) {
		check_all_unary(xs[i]);
		for (j = 0; j < sizeof(ys) / sizeof(ys[0]); j++)
			check_powl(xs[i], ys[j]);
	}

	check_all_unary((long double)HUGE_VAL);
	check_all_unary(-(long double)HUGE_VAL);
	check_all_unary(std::nanl(""));
	check_all_unary(-std::nanl(""));

	check_powl((long double)HUGE_VAL, 2.0L);
	check_powl(-(long double)HUGE_VAL, 2.0L);
	check_powl(2.0L, (long double)HUGE_VAL);
	check_powl(2.0L, -(long double)HUGE_VAL);
	check_powl(0.5L, (long double)HUGE_VAL);
	check_powl(2.0L, std::nanl(""));
	check_powl(std::nanl(""), 2.0L);
	check_powl(-1.0L, (long double)HUGE_VAL);
	check_powl(-1.0L, -(long double)HUGE_VAL);

	for (i = 1; i <= 20; i++) {
		check_powl((long double)i, 2.0L);
		check_powl(-(long double)i, 3.0L);
		check_powl((long double)i, (long double)i);
		check_powl(-(long double)i, (long double)i);
		check_powl(2.0L, (long double)i);
		check_powl(-2.0L, (long double)i);
	}

	for (i = 0; i <= 32; i++) {
		long double t = std::ldexpl(1.0L, -i);

		check_powl(t, 0.5L);
		check_powl(t * 1.0001L, 100.0L);
		check_all_unary(t);
		check_all_unary(2.0L - t);
	}
}

#ifdef PBSD_LD80
static void
run_ld80_bit_edges(void)
{
	static const unsigned exps[] = {
		0u, 1u, 2u, 0x3ffdu, 0x3ffeu, 0x3fffu, 0x4000u,
		0x7ffdu, 0x7ffeu, 0x7fffu,
	};
	static const std::uint64_t mants[] = {
		0x0000000000000000ULL, 0x0000000000000001ULL,
		0x7fffffffffffffffULL, 0x8000000000000000ULL,
		0x8000000000000001ULL, 0xc000000000000000ULL,
		0xffffffffffffffffULL,
	};
	static const long double ys[] = {
		0.0L, 1.0L, -1.0L, 2.0L, -2.0L, 0.5L, 3.0L, -3.0L,
		32767.0L, 100.0L, 8700.0L, LDBL_MAX, -LDBL_MAX,
	};
	std::size_t i, j, k, s, y;

	for (i = 0; i < sizeof(exps) / sizeof(exps[0]); i++)
		for (j = 0; j < sizeof(mants) / sizeof(mants[0]); j++)
			for (s = 0; s < 2; s++) {
				long double x = ld_bits(mants[j], exps[i],
				    (unsigned)s);
				check_all_unary(x);
				for (y = 0; y < sizeof(ys) / sizeof(ys[0]);
				    y++)
					check_powl(x, ys[y]);
			}

	for (i = 0; i < 129; i++) {
		long double c = 1.0L + (long double)i / 128.0L;
		long double e = (long double)i / 128.0L - 0.5L / 128.0L;

		check_all_unary(c);
		check_all_unary(c - 1.0L / 256.0L);
		check_log1pl(e);
		check_log1pl(-e);
		check_powl(c, 2.0L);
		check_powl(c, -2.0L);
	}
}
#else
static void
run_ld80_bit_edges(void)
{
}
#endif

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
		switch (i & 7u) {
		case 0:
			check_powl((long double)std::bit_cast<double>(r),
			    (long double)std::bit_cast<double>(rnd64()));
			break;
		case 1:
			check_powl((long double)((std::int64_t)r) / 17.0L,
			    (long double)((std::int64_t)(rnd64())) / 31.0L);
			break;
#ifdef PBSD_LD80
		case 2: {
			long double x = ld_bits(rnd64(),
			    (unsigned)(r >> 40) & 0x7fffu,
			    (unsigned)(r & 1u));
			long double y = ld_bits(rnd64(),
			    (unsigned)(rnd64() >> 40) & 0x7fffu,
			    (unsigned)(rnd64() & 1u));
			check_powl(x, y);
			check_all_unary(x);
			break;
		}
		case 3: {
			long double x = ld_bits(rnd64(),
			    0x3fffu + ((unsigned)(r >> 8) & 0x1fu) -
			    ((unsigned)(r >> 16) & 0x1fu),
			    (unsigned)(r & 1u));
			check_all_unary(x);
			check_powl(x, (long double)((int)(r & 0xffu) - 128) /
			    16.0L);
			break;
		}
#else
		case 2:
			check_powl((long double)(std::int32_t)(r >> 32) / 3.0L,
			    (long double)(std::int32_t)rnd64() / 7.0L);
			break;
		case 3:
			check_all_unary((long double)(std::int32_t)(r >> 32) /
			    5.0L);
			break;
#endif
		case 4:
			check_log1pl((long double)std::bit_cast<double>(r));
			break;
		case 5:
			check_log1pl((long double)((std::int64_t)r) /
			    8589934592.0L);
			break;
		case 6:
			check_powl(0.99L + (long double)(r & 0xffffu) /
			    6553600.0L,
			    (long double)((r >> 16) & 0xffffu));
			break;
		default:
			check_powl((long double)((r & 0x7ffu) + 1),
			    -0.5L + (long double)((rnd64() >> 20) & 0xfffu) /
			    256.0L);
			break;
		}
	}
}

/* ---------------------------------------------------------------- */

int
main(void)
{
	Stat *all[] = {
		&st_powl, &st_logl, &st_log1pl, &st_log10l, &st_log2l,
	};
	unsigned long long total_fail = 0;
	int i;

	run_scalar_edges();
	run_ld80_bit_edges();
	run_random();

	std::printf("\n%-10s %14s %12s  %s\n", "function", "cases", "failures",
	    "result");
	std::printf("---------------------------------------------------\n");
	for (i = 0; i < 5; i++) {
		std::printf("%-10s %14llu %12llu  %s\n", all[i]->name,
		    all[i]->cases, all[i]->failures,
		    all[i]->failures == 0 ? "PASS" : "FAIL");
		total_fail += all[i]->failures;
	}
	std::printf("---------------------------------------------------\n");
	std::printf("%-10s %14llu %12llu  %s\n", "TOTAL",
	    st_powl.cases + st_logl.cases + st_log1pl.cases +
	    st_log10l.cases + st_log2l.cases,
	    total_fail, total_fail == 0 ? "PASS" : "FAIL");

	return total_fail == 0 ? 0 : 1;
}
