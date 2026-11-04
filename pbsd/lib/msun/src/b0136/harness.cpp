/*
 * harness.cpp -- differential test for PBSD batch b0136.
 *
 * Every case is fed to BOTH the C++23 port and the ref_ oracle compiled from
 * the original C, and the results are compared bit for bit.  Arguments are
 * staged inside 0x7f-guarded scratch buffers (one per implementation) and
 * the ENTIRE buffer is compared afterwards.
 */

import pbsd.lib.msun.src.b0136;

#include <bit>
#include <cfloat>
#include <climits>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>

extern "C" long double ref_scalbnl(long double x, int n);

namespace P = pbsd::lib_msun_src::b0136;

#if (LDBL_MANT_DIG == 64 || LDBL_MANT_DIG == 113) && LDBL_MAX_EXP == 16384
#define PBSD_HAVE_SCALBNL 1
#endif

#if LDBL_MANT_DIG == 64
#define PBSD_LD80 1
#define PBSD_LDBYTES 10
#elif LDBL_MANT_DIG == 113
#define PBSD_LDBYTES 16
#endif

static const unsigned char GUARD_BYTE = 0x7f;
static const int MAX_REPORT = 12;

struct Stat {
	const char *name;
	unsigned long long cases;
	unsigned long long failures;
	unsigned reported;
};

static Stat st_scalbnl = { "scalbnl", 0, 0, 0 };

static unsigned long long cov_n_hi1, cov_n_hi2, cov_n_hi3;
static unsigned long long cov_n_lo1, cov_n_lo2, cov_n_lo3;
static unsigned long long cov_n_mid;

static std::uint64_t rng_state = 0x243f6a8885a308d3ULL;

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
	unsigned char ba[sizeof(long double)];
	unsigned char bb[sizeof(long double)];

	std::memset(ba, 0, sizeof(ba));
	std::memset(bb, 0, sizeof(bb));
	std::memcpy(ba, &a, sizeof(a));
	std::memcpy(bb, &b, sizeof(b));
#ifdef PBSD_LDBYTES
	return std::memcmp(ba, bb, PBSD_LDBYTES) == 0;
#else
	return std::memcmp(ba, bb, sizeof(long double)) == 0;
#endif
}

static void
print_ld(const long double *v)
{
	unsigned char b[sizeof(long double)];
	std::size_t i, n;

	std::memset(b, 0, sizeof(b));
	std::memcpy(b, v, sizeof(*v));
#ifdef PBSD_LDBYTES
	n = PBSD_LDBYTES;
#else
	n = sizeof(long double);
#endif
	for (i = n; i-- > 0;)
		std::printf("%02x", b[i]);
}

#ifdef PBSD_LD80
static long double
ld_from_bits(std::uint64_t mant, unsigned exp15, unsigned sign)
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
#elif LDBL_MANT_DIG == 113
static long double
ld128_from_parts(std::uint64_t manl, std::uint64_t manh, unsigned exp15,
    unsigned sign)
{
	long double v;
	unsigned char b[sizeof(long double)];

	std::memset(b, 0, sizeof(b));
	std::memcpy(b, &manl, sizeof(manl));
	std::memcpy(b + 8, &manh, sizeof(manh));
	std::uint16_t se = (std::uint16_t)(((sign & 1u) << 15) | (exp15 & 0x7fffu));
	std::memcpy(b + 16, &se, sizeof(se));
	std::memcpy(&v, b, sizeof(v));
	return v;
}
#endif

union LdPad {
	long double v;
	unsigned char b[sizeof(long double)];
};

union IntPad {
	int v;
	unsigned char b[sizeof(int)];
};

static LdPad port_xbuf[6];
static LdPad ref_xbuf[6];
static IntPad port_nbuf[6];
static IntPad ref_nbuf[6];

static const int X_SLOT = 3;
static const int N_SLOT = 3;

static void
fill_ld_pad(LdPad *pad, long double x)
{
	for (int i = 0; i < 6; i++)
		std::memset(pad[i].b, GUARD_BYTE, sizeof(pad[i].b));
	pad[X_SLOT].v = x;
}

static void
fill_int_pad(IntPad *pad, int n)
{
	for (int i = 0; i < 6; i++)
		std::memset(pad[i].b, GUARD_BYTE, sizeof(pad[i].b));
	pad[N_SLOT].v = n;
}

static bool
ld_pad_ok(const LdPad *pad, long double x)
{
	for (int i = 0; i < 6; i++) {
		if (i == X_SLOT) {
			if (!ld_same(pad[i].v, x))
				return false;
			continue;
		}
		for (std::size_t j = 0; j < sizeof(pad[i].b); j++) {
			if (pad[i].b[j] != GUARD_BYTE)
				return false;
		}
	}
	return true;
}

static bool
int_pad_ok(const IntPad *pad, int n)
{
	for (int i = 0; i < 6; i++) {
		if (i == N_SLOT) {
			if (pad[i].v != n)
				return false;
			continue;
		}
		for (std::size_t j = 0; j < sizeof(pad[i].b); j++) {
			if (pad[i].b[j] != GUARD_BYTE)
				return false;
		}
	}
	return true;
}

static void
track_n(int n)
{
	if (n > 16383) {
		int t = n - 16383;
		cov_n_hi1++;
		if (t > 16383) {
			cov_n_hi2++;
			if (t - 16383 > 16383)
				cov_n_hi3++;
		}
	} else if (n < -16382) {
		int t = n + 16382 - 113;
		cov_n_lo1++;
		if (t < -16382) {
			cov_n_lo2++;
			if (t + 16382 - 113 < -16382)
				cov_n_lo3++;
		}
	} else {
		cov_n_mid++;
	}
}

static void
check_scalbnl(long double x, int n)
{
	long double got, want;
	bool bad;

	st_scalbnl.cases++;

	fill_ld_pad(port_xbuf, x);
	fill_ld_pad(ref_xbuf, x);
	fill_int_pad(port_nbuf, n);
	fill_int_pad(ref_nbuf, n);

	got = P::scalbnl(port_xbuf[X_SLOT].v, port_nbuf[N_SLOT].v);
	want = ref_scalbnl(ref_xbuf[X_SLOT].v, ref_nbuf[N_SLOT].v);

	bad = !ld_same(got, want);
	if (!ld_pad_ok(port_xbuf, x) || !ld_pad_ok(ref_xbuf, x))
		bad = true;
	if (!int_pad_ok(port_nbuf, n) || !int_pad_ok(ref_nbuf, n))
		bad = true;
	if (std::memcmp(port_xbuf, ref_xbuf, sizeof(port_xbuf)) != 0)
		bad = true;
	if (std::memcmp(port_nbuf, ref_nbuf, sizeof(port_nbuf)) != 0)
		bad = true;

	track_n(n);

	if (bad) {
		st_scalbnl.failures++;
		if (st_scalbnl.reported++ < MAX_REPORT) {
			std::printf("  scalbnl FAIL x=");
			print_ld(&x);
			std::printf(" n=%d port=", n);
			print_ld(&got);
			std::printf(" ref=");
			print_ld(&want);
			std::printf("\n");
		}
	}
}

static const int n_edges[] = {
	0, 1, -1, 2, -2, 3, -3,
	16381, 16382, 16383, 16384, 16385, 16386,
	32765, 32766, 32767, 32768, 32769, 32770,
	49148, 49149, 49150, 49151, 49152, 49153, 65535,
	-16380, -16381, -16382, -16383, -16384, -16385, -16386,
	-32763, -32764, -32765, -32766, -32767, -32768,
	-49145, -49146, -49147, -49148, -49149, -49150, -65535,
	100000, -100000, 1000000, -1000000,
	0x7ffffffe, 0x7fffffff, -0x7fffffff, INT_MIN,
};

static void
run_edges(void)
{
	static const long double x_vals[] = {
		0.0L, -0.0L, 1.0L, -1.0L, 2.0L, -2.0L,
		0.5L, -0.5L, 3.0L, -3.0L,
		0x1p-16382L, -0x1p-16382L,
		0x1p-1L, -0x1p-1L,
		0x1p1L, -0x1p1L,
		0x1p10L, -0x1p10L,
		0x1p100L, -0x1p100L,
		0x1p1000L, -0x1p1000L,
		0x1p10000L, -0x1p10000L,
		0x1p16000L, -0x1p16000L,
		LDBL_MIN, -LDBL_MIN,
		LDBL_TRUE_MIN, -LDBL_TRUE_MIN,
		LDBL_MAX, -LDBL_MAX,
		LDBL_EPSILON, -LDBL_EPSILON,
	};
	std::size_t i, j;

	for (i = 0; i < sizeof(x_vals) / sizeof(x_vals[0]); i++)
		for (j = 0; j < sizeof(n_edges) / sizeof(n_edges[0]); j++)
			check_scalbnl(x_vals[i], n_edges[j]);

	check_scalbnl((long double)HUGE_VALL, 0);
	check_scalbnl(-(long double)HUGE_VALL, 0);
	check_scalbnl(std::nanl(""), 0);
	check_scalbnl(-std::nanl(""), 0);
	check_scalbnl((long double)HUGE_VALL, 16384);
	check_scalbnl(-(long double)HUGE_VALL, -16383);
	check_scalbnl(std::nanl(""), 49150);
	check_scalbnl(1.0L, INT_MAX);
	check_scalbnl(1.0L, INT_MIN);
	check_scalbnl(-1.0L, INT_MAX);
	check_scalbnl(-1.0L, INT_MIN);

#ifdef PBSD_LD80
	static const unsigned exps[] = {
		0u, 1u, 2u, 0x3ffdu, 0x3ffeu, 0x3fffu, 0x4000u,
		0x7ffdu, 0x7ffeu, 0x7fffu,
	};
	static const std::uint64_t mants[] = {
		0x0000000000000000ULL, 0x0000000000000001ULL,
		0x0000000000000080ULL, 0x7fffffffffffff80ULL,
		0x7fffffffffffffffULL, 0x8000000000000000ULL,
		0x8000000000000001ULL, 0xc000000000000000ULL,
		0xffffffffffffffffULL,
	};
	std::size_t k, s;

	for (i = 0; i < sizeof(exps) / sizeof(exps[0]); i++)
		for (j = 0; j < sizeof(mants) / sizeof(mants[0]); j++)
			for (k = 0; k < sizeof(n_edges) / sizeof(n_edges[0]); k++)
				for (s = 0; s < 2; s++)
					check_scalbnl(
					    ld_from_bits(mants[j], exps[i],
						(unsigned)s),
					    n_edges[k]);
#elif LDBL_MANT_DIG == 113
	static const unsigned exps[] = {
		0u, 1u, 0x3ffdu, 0x3ffeu, 0x3fffu, 0x4000u,
		0x7ffdu, 0x7ffeu, 0x7fffu,
	};
	static const std::uint64_t manls[] = {
		0x0000000000000000ULL, 0x0000000000000001ULL,
		0xffffffffffffffffULL,
	};
	static const std::uint64_t manhs[] = {
		0x0000000000000000ULL, 0x0000000000000001ULL,
		0x0000800000000000ULL, 0xffffffffffffULL,
	};
	std::size_t k, s, mh;

	for (i = 0; i < sizeof(exps) / sizeof(exps[0]); i++)
		for (j = 0; j < sizeof(manls) / sizeof(manls[0]); j++)
			for (mh = 0; mh < sizeof(manhs) / sizeof(manhs[0]); mh++)
				for (k = 0; k < 8; k++)
					for (s = 0; s < 2; s++)
						check_scalbnl(
						    ld128_from_parts(manls[j],
							manhs[mh], exps[i],
							(unsigned)s),
						    n_edges[k % (sizeof(n_edges)
							/ sizeof(n_edges[0]))]);
#endif
}

static void
run_random(void)
{
	unsigned long long i;

	for (i = 0; i < 200000ULL; i++) {
		long double x;
		int n;

#ifdef PBSD_LD80
		std::uint64_t mant = rnd64();
		unsigned exp15 = (unsigned)(rnd64() & 0x7fffu);
		unsigned sign = (unsigned)(rnd64() & 1u);
		x = ld_from_bits(mant, exp15, sign);
#else
		std::uint64_t manl = rnd64();
		std::uint64_t manh = rnd64() & 0xffffffffffffULL;
		unsigned exp15 = (unsigned)(rnd64() & 0x7fffu);
		unsigned sign = (unsigned)(rnd64() & 1u);
		x = ld128_from_parts(manl, manh, exp15, sign);
#endif
		n = (int)(std::int32_t)rnd64();
		check_scalbnl(x, n);

		if ((i & 0xffu) == 0u)
			check_scalbnl(x, (int)(16383 + (rnd64() % 40000u)));
		if ((i & 0xffu) == 1u)
			check_scalbnl(x, (int)(-16382 - (int)(rnd64() % 40000u)));
		if ((i & 0xffu) == 2u)
			check_scalbnl(1.0L, (int)(32767 + (rnd64() % 20000u)));
		if ((i & 0xffu) == 3u)
			check_scalbnl(-1.0L,
			    (int)(-32765 - (int)(rnd64() % 20000u)));
	}
}

int
main(void)
{
#ifdef PBSD_HAVE_SCALBNL
	run_edges();
	run_random();

	std::printf("\n=== scalbnl differential test ===\n");
	std::printf("%-10s %12s %12s\n", "function", "cases", "failures");
	std::printf("%-10s %12llu %12llu\n",
	    st_scalbnl.name, st_scalbnl.cases, st_scalbnl.failures);
	std::printf("\nbranch coverage (input n before call):\n");
	std::printf("  n>16383:        %llu\n", cov_n_hi1);
	std::printf("  n>32766:        %llu\n", cov_n_hi2);
	std::printf("  n>49149:        %llu\n", cov_n_hi3);
	std::printf("  n<-16382:       %llu\n", cov_n_lo1);
	std::printf("  n<-32765:       %llu\n", cov_n_lo2);
	std::printf("  n<-49148:       %llu\n", cov_n_lo3);
	std::printf("  -16382<=n<=16383: %llu\n", cov_n_mid);

	return st_scalbnl.failures == 0 ? 0 : 1;
#else
	std::printf("scalbnl: unsupported long double format on this target\n");
	return 0;
#endif
}
