// Differential test for PBSD batch b0267 (copysignl, sincos).

import pbsd.lib.msun.src.b0267;

#include <bit>
#include <cfloat>
#include <climits>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>

namespace P = pbsd::lib_msun_src::b0267;

extern "C" {
long double ref_copysignl(long double x, long double y);
void ref_sincos(double x, double *sn, double *cs);
}

static const unsigned char GUARD_BYTE = 0x7f;
static const int MAX_REPORT = 12;

struct Stat {
	const char *name;
	unsigned long long cases;
	unsigned long long failures;
	unsigned reported;
};

static Stat st_copysignl = { "copysignl", 0, 0, 0 };
static Stat st_sincos    = { "sincos",    0, 0, 0 };

static std::uint64_t rng_state = 0x9e3779b97f4a7c15ULL;

static std::uint64_t next_u64(void)
{
	std::uint64_t z = (rng_state += 0x9e3779b97f4a7c15ULL);
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return z ^ (z >> 31);
}

static std::uint32_t next_mod(std::uint32_t m)
{
	return (std::uint32_t)(next_u64() % m);
}

#if LDBL_MANT_DIG == 64
#define PBSD_LD80 1
#define PBSD_LDBYTES 10
#elif LDBL_MANT_DIG == 113
#define PBSD_LDBYTES 16
#endif

static bool ld_same(long double a, long double b)
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

static void print_ld(const long double *v)
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
static long double ld_from_bits(std::uint64_t mant, unsigned exp15, unsigned sign)
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
static long double ld128_from_parts(std::uint64_t manl, std::uint64_t manh,
    unsigned exp15, unsigned sign)
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

union LdPad { long double v; unsigned char b[sizeof(long double)]; };
static const int LD_SLOT = 3;

static void fill_ld_pad(LdPad *pads, long double x)
{
	for (int i = 0; i < 6; i++)
		std::memset(pads[i].b, GUARD_BYTE, sizeof(pads[i].b));
	pads[LD_SLOT].v = x;
}

static bool ld_pad_ok(const LdPad *pads, long double x)
{
	for (int i = 0; i < 6; i++) {
		if (i == LD_SLOT) {
			if (!ld_same(pads[i].v, x))
				return false;
			continue;
		}
		for (std::size_t j = 0; j < sizeof(pads[i].b); j++)
			if (pads[i].b[j] != GUARD_BYTE)
				return false;
	}
	return true;
}

static void check_copysignl(long double x, long double y)
{
	LdPad port_x[6], ref_x[6], port_y[6], ref_y[6];
	long double got, want;
	bool bad;
	st_copysignl.cases++;
	fill_ld_pad(port_x, x);
	fill_ld_pad(ref_x, x);
	fill_ld_pad(port_y, y);
	fill_ld_pad(ref_y, y);
	got = P::copysignl(port_x[LD_SLOT].v, port_y[LD_SLOT].v);
	want = ref_copysignl(ref_x[LD_SLOT].v, ref_y[LD_SLOT].v);
	bad = !ld_same(got, want);
	if (!ld_pad_ok(port_x, x) || !ld_pad_ok(ref_x, x))
		bad = true;
	if (!ld_pad_ok(port_y, y) || !ld_pad_ok(ref_y, y))
		bad = true;
	if (std::memcmp(port_x, ref_x, sizeof(port_x)) != 0)
		bad = true;
	if (std::memcmp(port_y, ref_y, sizeof(ref_y)) != 0)
		bad = true;
	if (bad) {
		st_copysignl.failures++;
		if (st_copysignl.reported++ < MAX_REPORT) {
			std::printf("  copysignl FAIL x=");
			print_ld(&x);
			std::printf(" y=");
			print_ld(&y);
			std::printf(" port=");
			print_ld(&got);
			std::printf(" ref=");
			print_ld(&want);
			std::printf("\n");
		}
	}
}

static std::uint64_t dbits_of(double d) { return std::bit_cast<std::uint64_t>(d); }
static double double_of(std::uint64_t u) { return std::bit_cast<double>(u); }
static bool d_same(double a, double b) { return dbits_of(a) == dbits_of(b); }

union DblPad { double v; unsigned char b[sizeof(double)]; };
static const int DBL_SLOT = 3;

static void fill_dbl_pad(DblPad *pads, double v)
{
	for (int i = 0; i < 6; i++)
		std::memset(pads[i].b, GUARD_BYTE, sizeof(pads[i].b));
	pads[DBL_SLOT].v = v;
}

static bool dbl_pad_ok(const DblPad *pads, double v)
{
	for (int i = 0; i < 6; i++) {
		if (i == DBL_SLOT)
			return d_same(pads[i].v, v);
		for (std::size_t j = 0; j < sizeof(pads[i].b); j++)
			if (pads[i].b[j] != GUARD_BYTE)
				return false;
	}
	return true;
}

static void check_sincos(double x)
{
	DblPad port_sn[6], ref_sn[6], port_cs[6], ref_cs[6];
	bool bad;
	st_sincos.cases++;
	fill_dbl_pad(port_sn, 0.0);
	fill_dbl_pad(ref_sn, 0.0);
	fill_dbl_pad(port_cs, 0.0);
	fill_dbl_pad(ref_cs, 0.0);
	P::sincos(x, &port_sn[DBL_SLOT].v, &port_cs[DBL_SLOT].v);
	ref_sincos(x, &ref_sn[DBL_SLOT].v, &ref_cs[DBL_SLOT].v);
	bad = !d_same(port_sn[DBL_SLOT].v, ref_sn[DBL_SLOT].v);
	bad = bad || !d_same(port_cs[DBL_SLOT].v, ref_cs[DBL_SLOT].v);
	if (!dbl_pad_ok(port_sn, port_sn[DBL_SLOT].v))
		bad = true;
	if (!dbl_pad_ok(ref_sn, ref_sn[DBL_SLOT].v))
		bad = true;
	if (!dbl_pad_ok(port_cs, port_cs[DBL_SLOT].v))
		bad = true;
	if (!dbl_pad_ok(ref_cs, ref_cs[DBL_SLOT].v))
		bad = true;
	if (std::memcmp(port_sn, ref_sn, sizeof(port_sn)) != 0)
		bad = true;
	if (std::memcmp(port_cs, ref_cs, sizeof(ref_cs)) != 0)
		bad = true;
	if (bad) {
		st_sincos.failures++;
		if (st_sincos.reported++ < MAX_REPORT)
			std::printf("  sincos FAIL x=%#016llx sn port=%#016llx ref=%#016llx "
			    "cs port=%#016llx ref=%#016llx\n",
			    (unsigned long long)dbits_of(x),
			    (unsigned long long)dbits_of(port_sn[DBL_SLOT].v),
			    (unsigned long long)dbits_of(ref_sn[DBL_SLOT].v),
			    (unsigned long long)dbits_of(port_cs[DBL_SLOT].v),
			    (unsigned long long)dbits_of(ref_cs[DBL_SLOT].v));
	}
}

static const std::uint64_t kEdgeD[] = {
	0x0000000000000000ULL, 0x8000000000000000ULL,
	0x0000000000000001ULL, 0x8000000000000001ULL,
	0x3e40000000000000ULL, 0xbe40000000000000ULL,
	0x3e3fffff00000000ULL, 0xbe3fffff00000000ULL,
	0x3e40000100000000ULL, 0xbe40000100000000ULL,
	0x3fe921fb00000000ULL, 0xbfe921fb00000000ULL,
	0x3fe921fa80000000ULL, 0xbfe921fa80000000ULL,
	0x3fe921fb54442d18ULL, 0xbfe921fb54442d18ULL,
	0x3ff921fb00000000ULL, 0xbff921fb00000000ULL,
	0x4002d97c00000000ULL, 0xc002d97c00000000ULL,
	0x400921fb00000000ULL, 0xc00921fb00000000ULL,
	0x400f6a7a00000000ULL, 0xc00f6a7a00000000ULL,
	0x4012d97c00000000ULL, 0xc012d97c00000000ULL,
	0x401921fb00000000ULL, 0xc01921fb00000000ULL,
	0x413921fb00000000ULL, 0xc13921fb00000000ULL,
	0x7ff0000000000000ULL, 0xfff0000000000000ULL,
	0x7ff8000000000000ULL, 0xfff8000000000000ULL,
	0x7fffffffffffffffULL, 0xffffffffffffffffULL,
};

static std::uint64_t rand_dbits(void)
{
	std::uint64_t r = next_u64();
	switch (next_mod(10u)) {
	case 0: return r;
	case 1: return r & 0x800fffffffffffffULL;
	case 2: return (r & 0x800fffffffffffffULL) | 0x7ff0000000000000ULL;
	case 3: {
		std::uint64_t d = next_mod(11u) - 5u;
		return (r & 0x8000000000000000ULL) |
		    ((0x3fe921fb00000000ULL + d) & 0x7fffffffffffffffULL);
	}
	case 4: {
		std::uint64_t d = next_mod(11u) - 5u;
		return (r & 0x8000000000000000ULL) |
		    ((0x3e40000000000000ULL + d) & 0x7fffffffffffffffULL);
	}
	case 5: {
		std::uint64_t e = 970u + next_mod(61u);
		return (r & 0x8000000000000000ULL) | (e << 52) | (r & 0x000fffffffffffffULL);
	}
	default: return r ^ next_u64();
	}
}

#ifdef PBSD_LD80
static unsigned rand_ld_exp(void)
{
	static const unsigned exps[] = { 0u, 1u, 0x3fffu, 0x7fffu };
	return exps[next_mod(4u)];
}
#endif

static void run_copysignl_edges(void)
{
	static const long double vals[] = {
		0.0L, -0.0L, 1.0L, -1.0L, LDBL_MIN, -LDBL_MIN,
		LDBL_MAX, -LDBL_MAX, std::nanl(""), -std::nanl("1"),
	};
	for (std::size_t i = 0; i < sizeof(vals) / sizeof(vals[0]); i++)
		for (std::size_t j = 0; j < sizeof(vals) / sizeof(vals[0]); j++)
			check_copysignl(vals[i], vals[j]);
#ifdef PBSD_LD80
	for (int s = 0; s < 2; s++)
		for (int t = 0; t < 2; t++)
			check_copysignl(ld_from_bits(next_u64(), rand_ld_exp(), s),
			    ld_from_bits(next_u64(), rand_ld_exp(), t));
#endif
}

static void run_sincos_edges(void)
{
	for (std::size_t i = 0; i < sizeof kEdgeD / sizeof kEdgeD[0]; i++)
		check_sincos(double_of(kEdgeD[i]));
	check_sincos(M_PI_2);
	check_sincos(-M_PI_2);
	check_sincos(0x1p-30);
	check_sincos(-0x1p-30);
	check_sincos(0x1p20);
	check_sincos(-0x1p20);
	check_sincos(std::nan(""));
	check_sincos(std::numeric_limits<double>::infinity());
}

static void run_random(void)
{
	for (unsigned long long i = 0; i < 200000ULL; i++) {
#ifdef PBSD_LD80
		check_copysignl(ld_from_bits(next_u64(), rand_ld_exp(),
		    (unsigned)(next_u64() & 1u)),
		    ld_from_bits(next_u64(), rand_ld_exp(),
		    (unsigned)(next_u64() & 1u)));
#else
		check_copysignl((long double)next_u64(), (long double)next_u64());
#endif
		check_sincos(double_of(rand_dbits()));
	}
}

int main(void)
{
	run_copysignl_edges();
	run_sincos_edges();
	run_random();
	std::printf("\n%-12s %12s %12s\n", "function", "cases", "failures");
	std::printf("%-12s %12llu %12llu\n", st_copysignl.name,
	    st_copysignl.cases, st_copysignl.failures);
	std::printf("%-12s %12llu %12llu\n", st_sincos.name,
	    st_sincos.cases, st_sincos.failures);
	return (st_copysignl.failures != 0 || st_sincos.failures != 0) ? 1 : 0;
}
