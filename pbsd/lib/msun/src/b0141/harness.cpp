/*
 * harness.cpp -- differential test for PBSD batch b0141.
 *
 * Every case is fed to BOTH the C++23 port and the ref_ oracle compiled from
 * the original C, and the results are compared bit for bit.  Arguments are
 * staged inside 0x7f-guarded scratch buffers (one per implementation) and
 * the ENTIRE buffer is compared afterwards.
 */

import pbsd.lib.msun.src.b0141;

#include <bit>
#include <cfloat>
#include <climits>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>

namespace P = pbsd::lib_msun_src::b0141;

extern "C" {
float ref_fminf(float x, float y);
float ref_sqrtf(float x);
#if LDBL_MAX_EXP == 16384 && (LDBL_MANT_DIG == 64 || LDBL_MANT_DIG == 113)
long double ref_fminl(long double x, long double y);
long double ref_nextafterl(long double x, long double y);
#endif
}

#if LDBL_MAX_EXP == 16384 && (LDBL_MANT_DIG == 64 || LDBL_MANT_DIG == 113)
#define PBSD_HAVE_LD80_FUNCS 1
#endif

#if LDBL_MANT_DIG == 64
#define PBSD_LDBYTES 10
#elif LDBL_MANT_DIG == 113
#define PBSD_LDBYTES 16
#endif

static const unsigned char GUARD_BYTE = 0x7f;
static const int MAX_REPORT = 12;
static const unsigned long RANDOM_ITERS = 200000u;

struct Stat {
	const char *name;
	unsigned long long cases;
	unsigned long long failures;
	unsigned reported;
};

static Stat st_fminf = { "fminf", 0, 0, 0 };
static Stat st_fminl = { "fminl", 0, 0, 0 };
static Stat st_nextafterl = { "nextafterl", 0, 0, 0 };
static Stat st_sqrtf = { "sqrtf", 0, 0, 0 };

static std::uint64_t rng_state = 0x243f6a8885a308d3ULL;

static std::uint64_t
rnd64(void)
{
	std::uint64_t z = (rng_state += 0x9E3779B97F4A7C15ULL);
	z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
	z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
	return z ^ (z >> 31);
}

static std::uint32_t
rnd32(void)
{
	return (std::uint32_t)(rnd64() >> 21);
}

static std::uint32_t
rndmod(std::uint32_t m)
{
	return (std::uint32_t)(rnd64() % m);
}

static std::uint32_t
fbits(float f)
{
	return std::bit_cast<std::uint32_t>(f);
}

static float
fromfbits(std::uint32_t u)
{
	return std::bit_cast<float>(u);
}

static bool
same_float(float a, float b)
{
	return fbits(a) == fbits(b);
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
print_f32(std::uint32_t u)
{
	std::printf("0x%08x", u);
}

#ifdef PBSD_LDBYTES
static void
print_ld(long double v)
{
	unsigned char b[sizeof(long double)];
	std::size_t i, n = PBSD_LDBYTES;

	std::memset(b, 0, sizeof(b));
	std::memcpy(b, &v, sizeof(v));
	for (i = n; i-- > 0;)
		std::printf("%02x", b[i]);
}
#endif

union FloatPad {
	float v;
	unsigned char b[sizeof(float)];
};

#ifdef PBSD_HAVE_LD80_FUNCS
union LdPad {
	long double v;
	unsigned char b[sizeof(long double)];
};
#endif

static FloatPad port_fbuf[6];
static FloatPad ref_fbuf[6];
#ifdef PBSD_HAVE_LD80_FUNCS
static LdPad port_ldbuf[6];
static LdPad ref_ldbuf[6];
#endif

static const int F_SLOT = 3;

static void
fill_float_pad(FloatPad *pad, float x, float y)
{
	for (int i = 0; i < 6; i++)
		std::memset(pad[i].b, GUARD_BYTE, sizeof(pad[i].b));
	pad[F_SLOT].v = x;
	pad[F_SLOT + 1].v = y;
}

#ifdef PBSD_HAVE_LD80_FUNCS
static void
fill_ld_pad(LdPad *pad, long double x, long double y)
{
	for (int i = 0; i < 6; i++)
		std::memset(pad[i].b, GUARD_BYTE, sizeof(pad[i].b));
	pad[F_SLOT].v = x;
	pad[F_SLOT + 1].v = y;
}

static long double
mkld(std::uint16_t se, std::uint64_t m)
{
	unsigned char b[sizeof(long double)];
	long double x;

	std::memset(b, 0, sizeof(b));
	std::memcpy(b, &m, sizeof(m));
	std::memcpy(b + 8, &se, sizeof(se));
	std::memcpy(&x, b, sizeof(x));
	return x;
}
#endif

static bool
float_pad_ok(const FloatPad *pad, float x, float y)
{
	for (int i = 0; i < 6; i++) {
		if (i == F_SLOT) {
			if (!same_float(pad[i].v, x))
				return false;
			continue;
		}
		if (i == F_SLOT + 1) {
			if (!same_float(pad[i].v, y))
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

#ifdef PBSD_HAVE_LD80_FUNCS
static bool
ld_pad_ok(const LdPad *pad, long double x, long double y)
{
	for (int i = 0; i < 6; i++) {
		if (i == F_SLOT) {
			if (!ld_same(pad[i].v, x))
				return false;
			continue;
		}
		if (i == F_SLOT + 1) {
			if (!ld_same(pad[i].v, y))
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
#endif

static bool
pads_equal(const FloatPad *a, const FloatPad *b)
{
	for (int i = 0; i < 6; i++) {
		if (std::memcmp(a[i].b, b[i].b, sizeof(a[i].b)) != 0)
			return false;
	}
	return true;
}

#ifdef PBSD_HAVE_LD80_FUNCS
static bool
ld_pads_equal(const LdPad *a, const LdPad *b)
{
	for (int i = 0; i < 6; i++) {
		if (std::memcmp(a[i].b, b[i].b, sizeof(a[i].b)) != 0)
			return false;
	}
	return true;
}
#endif

static void
report_fail(Stat &st, const char *tag, float x, float y, float got, float want)
{
	if (st.reported++ >= MAX_REPORT)
		return;
	std::printf("  FAIL [%s] x=", tag);
	print_f32(fbits(x));
	std::printf(" y=");
	print_f32(fbits(y));
	std::printf(" got=");
	print_f32(fbits(got));
	std::printf(" want=");
	print_f32(fbits(want));
	std::printf("\n");
}

#ifdef PBSD_HAVE_LD80_FUNCS
static void
report_fail_ld(Stat &st, const char *tag, long double x, long double y,
    long double got, long double want)
{
	if (st.reported++ >= MAX_REPORT)
		return;
	std::printf("  FAIL [%s] x=", tag);
	print_ld(x);
	std::printf(" y=");
	print_ld(y);
	std::printf(" got=");
	print_ld(got);
	std::printf(" want=");
	print_ld(want);
	std::printf("\n");
}
#endif

static void
check_fminf(float x, float y, const char *tag)
{
	float port_r, ref_r;

	st_fminf.cases++;
	fill_float_pad(port_fbuf, x, y);
	fill_float_pad(ref_fbuf, x, y);
	port_r = P::fminf(port_fbuf[F_SLOT].v, port_fbuf[F_SLOT + 1].v);
	ref_r = ref_fminf(ref_fbuf[F_SLOT].v, ref_fbuf[F_SLOT + 1].v);

	if (!same_float(port_r, ref_r) || !float_pad_ok(port_fbuf, x, y) ||
	    !float_pad_ok(ref_fbuf, x, y) || !pads_equal(port_fbuf, ref_fbuf)) {
		st_fminf.failures++;
		report_fail(st_fminf, tag, x, y, port_r, ref_r);
	}
}

#ifdef PBSD_HAVE_LD80_FUNCS
static void
check_fminl(long double x, long double y, const char *tag)
{
	long double port_r, ref_r;

	st_fminl.cases++;
	fill_ld_pad(port_ldbuf, x, y);
	fill_ld_pad(ref_ldbuf, x, y);
	port_r = P::fminl(port_ldbuf[F_SLOT].v, port_ldbuf[F_SLOT + 1].v);
	ref_r = ref_fminl(ref_ldbuf[F_SLOT].v, ref_ldbuf[F_SLOT + 1].v);

	if (!ld_same(port_r, ref_r) || !ld_pad_ok(port_ldbuf, x, y) ||
	    !ld_pad_ok(ref_ldbuf, x, y) || !ld_pads_equal(port_ldbuf, ref_ldbuf)) {
		st_fminl.failures++;
		report_fail_ld(st_fminl, tag, x, y, port_r, ref_r);
	}
}

static void
check_nextafterl(long double x, long double y, const char *tag)
{
	long double port_r, ref_r;

	st_nextafterl.cases++;
	fill_ld_pad(port_ldbuf, x, y);
	fill_ld_pad(ref_ldbuf, x, y);
	port_r = P::nextafterl(port_ldbuf[F_SLOT].v, port_ldbuf[F_SLOT + 1].v);
	ref_r = ref_nextafterl(ref_ldbuf[F_SLOT].v, ref_ldbuf[F_SLOT + 1].v);

	if (!ld_same(port_r, ref_r) || !ld_pad_ok(port_ldbuf, x, y) ||
	    !ld_pad_ok(ref_ldbuf, x, y) || !ld_pads_equal(port_ldbuf, ref_ldbuf)) {
		st_nextafterl.failures++;
		report_fail_ld(st_nextafterl, tag, x, y, port_r, ref_r);
	}
}
#endif

static void
check_sqrtf(float x, const char *tag)
{
	float port_r, ref_r;

	st_sqrtf.cases++;
	fill_float_pad(port_fbuf, x, 0.0f);
	fill_float_pad(ref_fbuf, x, 0.0f);
	port_r = P::sqrtf(port_fbuf[F_SLOT].v);
	ref_r = ref_sqrtf(ref_fbuf[F_SLOT].v);

	if (!same_float(port_r, ref_r) || !float_pad_ok(port_fbuf, x, 0.0f) ||
	    !float_pad_ok(ref_fbuf, x, 0.0f) || !pads_equal(port_fbuf, ref_fbuf)) {
		st_sqrtf.failures++;
		report_fail(st_sqrtf, tag, x, 0.0f, port_r, ref_r);
	}
}

static const std::uint32_t kEdgeF[] = {
	0x00000000u, 0x80000000u,
	0x00000001u, 0x80000001u,
	0x00000002u, 0x80000002u,
	0x00400000u, 0x80400000u,
	0x007ffffeu, 0x807ffffeu,
	0x007fffffu, 0x807fffffu,
	0x00800000u, 0x80800000u,
	0x00800001u, 0x80800001u,
	0x33800000u, 0xb3800000u,
	0x3e800000u, 0xbe800000u,
	0x3f000000u, 0xbf000000u,
	0x3f800000u, 0xbf800000u,
	0x3fc00000u, 0xbfc00000u,
	0x40000000u, 0xc0000000u,
	0x40400000u, 0xc0400000u,
	0x40a00000u, 0xc0a00000u,
	0x41200000u, 0xc1200000u,
	0x42c80000u, 0xc2c80000u,
	0x4b000000u, 0xcb000000u,
	0x4b800000u, 0xcb800000u,
	0x1f000000u, 0x9f000000u,
	0x5f000000u, 0xdf000000u,
	0x7effffffu, 0xfeffffffu,
	0x7f000000u, 0xff000000u,
	0x7f7ffffeu, 0xff7ffffeu,
	0x7f7fffffu, 0xff7fffffu,
	0x7f800000u, 0xff800000u,
	0x7f800001u, 0xff800001u,
	0x7fa5a5a5u, 0xffa5a5a5u,
	0x7fbfffffu, 0xffbfffffu,
	0x7fc00000u, 0xffc00000u,
	0x7fc5a5a5u, 0xffc5a5a5u,
	0x7ffffffeu, 0xfffffffeu,
	0x7fffffffu, 0xffffffffu,
	0x55555555u, 0xd5555555u,
	0x2aaaaaaau, 0xaaaaaaaau,
	0x0f0f0f0fu, 0x8f0f0f0fu,
	0x70f0f0f0u, 0xf0f0f0f0u,
	0x80808080u, 0x7f7f7f7fu,
	0x00ff00ffu, 0xff00ff00u,
	0x8000ffffu, 0x0000ff80u,
};
static const unsigned kEdgeFN =
    (unsigned)(sizeof kEdgeF / sizeof kEdgeF[0]);

#ifdef PBSD_HAVE_LD80_FUNCS
struct ldcase {
	std::uint16_t se;
	std::uint64_t m;
};

static const ldcase ldvec[] = {
	{ 0x0000u, 0x0000000000000000ull },
	{ 0x8000u, 0x0000000000000000ull },
	{ 0x0000u, 0x0000000000000001ull },
	{ 0x8000u, 0x0000000000000001ull },
	{ 0x0000u, 0x0000000000000080ull },
	{ 0x0000u, 0x0000000080000000ull },
	{ 0x8000u, 0x0000000080000000ull },
	{ 0x0000u, 0x7fffffffffffffffull },
	{ 0x8000u, 0x7fffffffffffffffull },
	{ 0x0000u, 0x8000000000000000ull },
	{ 0x8000u, 0x8000000000000000ull },
	{ 0x0000u, 0xc000000000000000ull },
	{ 0x0001u, 0x8000000000000000ull },
	{ 0x8001u, 0x8000000000000000ull },
	{ 0x0001u, 0x8000000000000001ull },
	{ 0x3ffeu, 0x8000000000000000ull },
	{ 0xbffeu, 0x8000000000000000ull },
	{ 0x3fffu, 0x8000000000000000ull },
	{ 0xbfffu, 0x8000000000000000ull },
	{ 0x4000u, 0x8000000000000000ull },
	{ 0xc000u, 0x8000000000000000ull },
	{ 0x403eu, 0x8000000000000000ull },
	{ 0x7ffeu, 0xffffffffffffffffull },
	{ 0xfffeu, 0xffffffffffffffffull },
	{ 0x7fffu, 0x8000000000000000ull },
	{ 0xffffu, 0x8000000000000000ull },
	{ 0x7fffu, 0xc000000000000000ull },
	{ 0xffffu, 0xc000000000000000ull },
	{ 0x7fffu, 0x8000000000000001ull },
	{ 0x7fffu, 0xffffffffffffffffull },
	{ 0x3fffu, 0x8080808080808080ull },
	{ 0xbfffu, 0xff00ff00ff00ff00ull },
	{ 0x3fffu, 0x8000000000000001ull },
	{ 0x3ffeu, 0xffffffffffffffffull },
	{ 0x3fffu, 0x0000000000000001ull },
};
static const unsigned NLDVEC = (unsigned)(sizeof ldvec / sizeof ldvec[0]);
#endif

static std::uint32_t
rand_bits_f(void)
{
	std::uint32_t r = rnd32();

	switch (rndmod(12u)) {
	case 0:
		return r;
	case 1:
		return r & 0x807fffffu;
	case 2:
		return (r & 0x807fffffu) | 0x7f800000u;
	case 3: {
		std::uint32_t d = rndmod(11u) - 5u;
		return (r & 0x80000000u) | ((0x7f800000u + d) & 0x7fffffffu);
	}
	case 4: {
		std::uint32_t d = rndmod(11u) - 5u;
		return (r & 0x80000000u) | ((0x00800000u + d) & 0x7fffffffu);
	}
	case 5: {
		std::uint32_t e = 97u + rndmod(61u);
		return (r & 0x80000000u) | (e << 23) | (r & 0x007fffffu);
	}
	case 6: {
		int k = (int)rndmod(2049u) - 1024;
		return fbits((float)k / 4.0f);
	}
	case 7: {
		int k = (int)rndmod(401u) - 200;
		return fbits((float)k);
	}
	case 8: {
		std::uint32_t e = (rnd64() & 1u) ? rndmod(8u) : 248u + rndmod(8u);
		return (r & 0x80000000u) | (e << 23) | (r & 0x007fffffu);
	}
	default:
		return r ^ rnd32();
	}
}

#ifdef PBSD_HAVE_LD80_FUNCS
static long double
rand_ld(void)
{
	std::uint16_t se;
	std::uint64_t m;

	switch (rndmod(10u)) {
	case 0:
		se = (std::uint16_t)rndmod(65536u);
		m = rnd64();
		break;
	case 1:
		se = (std::uint16_t)(rndmod(8u));
		m = rnd64();
		break;
	case 2:
		se = (std::uint16_t)(0x7fffu);
		m = rnd64() | 0x8000000000000001ull;
		break;
	case 3:
		se = (std::uint16_t)(0x3fffu + rndmod(5u) - 2u);
		m = 0x8000000000000000ull | (rnd64() & 0x7fffffffffffffffull);
		break;
	case 4:
		se = (std::uint16_t)(rndmod(3u));
		m = rnd64() | 1ull;
		break;
	case 5:
		se = (std::uint16_t)(0x7ffeu);
		m = 0xffffffffffffffffull;
		break;
	case 6:
		se = (std::uint16_t)(0x3fffu);
		m = 0x8000000000000000ull + (rnd64() & 0xffull);
		break;
	default:
		se = (std::uint16_t)(rndmod(32768u));
		m = rnd64();
		break;
	}
	return mkld(se, m);
}
#endif

static void
edge_cases(void)
{
	unsigned i, j;

	check_fminf(0.0f, -0.0f, "signed-zero");
	check_fminf(-0.0f, 0.0f, "signed-zero");

	for (i = 0; i < kEdgeFN; i++)
		for (j = 0; j < kEdgeFN; j++)
			check_fminf(fromfbits(kEdgeF[i]), fromfbits(kEdgeF[j]),
			    "fvec-cross");

	for (i = 0; i <= 255u; i++) {
		std::uint32_t infp = (i << 23);
		std::uint32_t infn = (i << 23) | 0x80000000u;
		std::uint32_t nanp = infp | 1u;
		std::uint32_t nann = infn | 1u;
		std::uint32_t normal = 0x3f800000u;

		check_fminf(fromfbits(normal), fromfbits(nanp), "nan-boundary");
		check_fminf(fromfbits(nanp), fromfbits(normal), "nan-boundary");
		check_fminf(fromfbits(normal), fromfbits(nann), "nan-boundary");
		check_fminf(fromfbits(nann), fromfbits(normal), "nan-boundary");
		check_fminf(fromfbits(infp), fromfbits(nanp), "nan-boundary");
		check_fminf(fromfbits(nanp), fromfbits(infp), "nan-boundary");
		check_fminf(fromfbits(nanp), fromfbits(nann), "nan-boundary");
	}

	for (i = 0; i < kEdgeFN; i++) {
		float x = fromfbits(kEdgeF[i]);
		check_fminf(x, x + 1.0f, "cmp-order");
		check_fminf(x, x - 1.0f, "cmp-order");
		check_fminf(x, x, "cmp-tie");
	}

	check_fminf(-1.0f, 1.0f, "sign-mix");
	check_fminf(1.0f, -1.0f, "sign-mix");

	for (i = 0; i < kEdgeFN; i++)
		check_sqrtf(fromfbits(kEdgeF[i]), "fvec");

	check_sqrtf(0.0f, "scalar-zero");
	check_sqrtf(-0.0f, "scalar-zero");
	check_sqrtf(1.0f, "scalar-one");
	check_sqrtf(4.0f, "scalar-four");
	check_sqrtf(-1.0f, "scalar-neg");
	check_sqrtf(-0.0f, "scalar-negzero");
	check_sqrtf(__builtin_inff(), "scalar-inf");
	check_sqrtf(-__builtin_inff(), "scalar-ninf");
	check_sqrtf(__builtin_nanf(""), "scalar-nan");
	check_sqrtf(__builtin_nanf("1"), "scalar-nan-payload");

	for (i = 1; i < 24u; i++) {
		std::uint32_t sub = (1u << i);
		check_sqrtf(fromfbits(sub), "subnormal");
		check_sqrtf(fromfbits(sub | 0x80000000u), "subnormal-neg");
	}

	for (i = 0; i < 256u; i++) {
		std::uint32_t e = (96u + (i % 64u)) << 23;
		std::uint32_t m = (i * 0x01010101u) & 0x007fffffu;
		check_sqrtf(fromfbits(e | m | 0x00800000u), "norm-sweep");
		check_sqrtf(fromfbits(e | m), "unnorm-sweep");
	}

	for (i = 0; i < kEdgeFN; i++) {
		std::uint32_t x = kEdgeF[i];
		if ((x & 0x7f800000u) == 0x7f800000u)
			continue;
		if ((int32_t)x <= 0 && (x & 0x7fffffffu) != 0)
			continue;
		check_sqrtf(fromfbits(x), "round-neighbor");
	}

#ifdef PBSD_HAVE_LD80_FUNCS
	check_fminl(0.0L, -0.0L, "signed-zero");
	check_fminl(-0.0L, 0.0L, "signed-zero");

	for (i = 0; i < NLDVEC; i++)
		for (j = 0; j < NLDVEC; j++)
			check_fminl(mkld(ldvec[i].se, ldvec[i].m),
			    mkld(ldvec[j].se, ldvec[j].m), "ldvec-cross");

	for (i = 0; i <= 32767u; i++) {
		std::uint16_t infse = (std::uint16_t)i;
		std::uint16_t nanse = (std::uint16_t)(i | 0x8000u);
		std::uint64_t infm = 0x8000000000000000ull;
		std::uint64_t nanm = 0x8000000000000001ull;
		long double normal = 1.0L;

		check_fminl(normal, mkld(infse, infm), "ld-nan-boundary");
		check_fminl(mkld(infse, nanm), normal, "ld-nan-boundary");
		check_fminl(normal, mkld(nanse, nanm), "ld-nan-boundary");
		check_fminl(mkld(nanse, nanm), normal, "ld-nan-boundary");
		check_fminl(mkld(infse, infm), mkld(infse, nanm), "ld-nan-boundary");
		check_fminl(mkld(infse, nanm), mkld(nanse, nanm), "ld-nan-boundary");
	}

	for (i = 0; i < NLDVEC; i++) {
		long double x = mkld(ldvec[i].se, ldvec[i].m);
		check_fminl(x, x + 1.0L, "cmp-order");
		check_fminl(x, x - 1.0L, "cmp-order");
		check_fminl(x, x, "cmp-tie");
	}

	check_fminl(-1.0L, 1.0L, "sign-mix");
	check_fminl(1.0L, -1.0L, "sign-mix");

	check_nextafterl(0.0L, 1.0L, "zero-toward-pos");
	check_nextafterl(0.0L, -1.0L, "zero-toward-neg");
	check_nextafterl(-0.0L, 1.0L, "negzero-toward-pos");
	check_nextafterl(-0.0L, -1.0L, "negzero-toward-neg");
	check_nextafterl(1.0L, 1.0L, "equal");
	check_nextafterl(1.0L, 2.0L, "inc");
	check_nextafterl(2.0L, 1.0L, "dec");
	check_nextafterl(1.0L, 0.0L, "toward-zero");
	check_nextafterl(-1.0L, 0.0L, "neg-toward-zero");
	check_nextafterl(__builtin_nanl(""), 1.0L, "nan-x");
	check_nextafterl(1.0L, __builtin_nanl(""), "nan-y");
	check_nextafterl(__builtin_infl(), __builtin_infl(), "inf-equal");
	check_nextafterl(__builtin_infl(), 0.0L, "inf-toward-zero");
	check_nextafterl(0.0L, __builtin_infl(), "zero-toward-inf");

	for (i = 0; i < NLDVEC; i++)
		for (j = 0; j < NLDVEC; j++)
			check_nextafterl(mkld(ldvec[i].se, ldvec[i].m),
			    mkld(ldvec[j].se, ldvec[j].m), "ldvec-cross");

	for (i = 0; i < NLDVEC; i++) {
		long double x = mkld(ldvec[i].se, ldvec[i].m);
		check_nextafterl(x, x + 1.0L, "step-up");
		check_nextafterl(x, x - 1.0L, "step-down");
		check_nextafterl(x, -x, "cross-zero");
	}

	{
		long double maxf = mkld(0x7ffeu, 0xffffffffffffffffull);
		check_nextafterl(maxf, __builtin_infl(), "near-max-up");
		check_nextafterl(-maxf, -__builtin_infl(), "near-min-down");
	}

	{
		long double sub = mkld(0x0000u, 0x0000000000000001ull);
		check_nextafterl(sub, 1.0L, "minsub-up");
		check_nextafterl(-sub, -1.0L, "minsub-down");
	}
#endif
}

static void
random_sweep(void)
{
	unsigned i;

	for (i = 0; i < RANDOM_ITERS; i++) {
		std::uint32_t ux = rand_bits_f();
		std::uint32_t uy = rand_bits_f();
		check_fminf(fromfbits(ux), fromfbits(uy), "random");
		check_sqrtf(fromfbits(ux), "random");
	}

#ifdef PBSD_HAVE_LD80_FUNCS
	for (i = 0; i < RANDOM_ITERS; i++) {
		long double x = rand_ld();
		long double y = rand_ld();
		check_fminl(x, y, "random");
		check_nextafterl(x, y, "random");
	}
#endif
}

static void
print_stat(const Stat &st)
{
	std::printf("%-12s %12llu %12llu\n", st.name, st.cases, st.failures);
}

int
main(void)
{
	unsigned long long total_fail = 0;

	edge_cases();
	random_sweep();

	std::printf("\n%-12s %12s %12s\n", "function", "cases", "failures");
	print_stat(st_fminf);
#ifdef PBSD_HAVE_LD80_FUNCS
	print_stat(st_fminl);
	print_stat(st_nextafterl);
#endif
	print_stat(st_sqrtf);

	total_fail = st_fminf.failures + st_sqrtf.failures;
#ifdef PBSD_HAVE_LD80_FUNCS
	total_fail += st_fminl.failures + st_nextafterl.failures;
#endif

	return total_fail == 0 ? 0 : 1;
}
