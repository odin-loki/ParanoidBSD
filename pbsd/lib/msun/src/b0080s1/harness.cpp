// Differential test harness for PBSD batch b0080s1.
//
// conjf is compared bit-for-bit against the unmodified C reference in
// oracle.c.  Results are staged through 0x7f-guarded buffers so that any
// stray write past the nominal result window is caught.

#include <cfloat>
#include <complex>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

import pbsd.lib.msun.src.b0080s1;

namespace port = pbsd::lib_msun_src::b0080s1;

extern "C" {
float _Complex ref_conjf(float _Complex z);
}

static const unsigned char GUARD = 0x7f;
static const std::size_t GUARD_BUF = 64;
static const std::size_t GUARD_OFF = 16;

static bool
guarded_equal(const void *pa, const void *pb, std::size_t n)
{
	unsigned char a[GUARD_BUF], b[GUARD_BUF];

	std::memset(a, GUARD, sizeof(a));
	std::memset(b, GUARD, sizeof(b));
	std::memcpy(a + GUARD_OFF, pa, n);
	std::memcpy(b + GUARD_OFF, pb, n);
	return (std::memcmp(a, b, sizeof(a)) == 0);
}

static std::uint32_t
fbits(float f)
{
	std::uint32_t u;

	std::memcpy(&u, &f, sizeof(u));
	return u;
}

static float
fromfbits(std::uint32_t u)
{
	float f;

	std::memcpy(&f, &u, sizeof(f));
	return f;
}

static float _Complex
mkfc(float re, float im)
{
	float _Complex z;

	__real__ z = re;
	__imag__ z = im;
	return z;
}

struct stat {
	const char *name;
	unsigned long long cases;
	unsigned long long fails;
	unsigned reported;
};

static stat st_conjf = { "conjf", 0, 0, 0 };

static const unsigned MAX_REPORT = 12;

static void
check_conjf(std::uint32_t re, std::uint32_t im, const char *tag)
{
	std::uint32_t gp[2], go[2];
	std::complex<float> pz;
	float _Complex oz;

	st_conjf.cases++;

	pz = port::conjf(std::complex<float>(fromfbits(re), fromfbits(im)));
	oz = ref_conjf(mkfc(fromfbits(re), fromfbits(im)));

	gp[0] = fbits(pz.real());
	gp[1] = fbits(pz.imag());
	go[0] = fbits(__real__ oz);
	go[1] = fbits(__imag__ oz);

	if (guarded_equal(gp, go, sizeof(gp)))
		return;

	st_conjf.fails++;
	if (st_conjf.reported < MAX_REPORT) {
		st_conjf.reported++;
		std::printf("  conjf FAIL [%s] in=(%08x,%08x) "
		    "port=(%08x,%08x) ref=(%08x,%08x)\n",
		    tag, re, im, gp[0], gp[1], go[0], go[1]);
	}
}

/*
 * Interesting single-precision object representations.  Includes both signed
 * zeroes, both infinities, quiet and signalling NaNs with assorted payloads,
 * the subnormal boundary from either side, the normal boundary, the largest
 * finite value, and a spread of patterns whose bytes are all in the high-bit
 * 0x80-0xff range (or straddle it) so that any sign/byte handling mistake is
 * exercised.
 */
static const std::uint32_t fvec[] = {
	0x00000000u,	/* +0 */
	0x80000000u,	/* -0 */
	0x00000001u,	/* smallest +subnormal */
	0x80000001u,	/* smallest -subnormal */
	0x00000080u,	/* subnormal, high bit of low byte */
	0x00008000u,	/* subnormal */
	0x007fffffu,	/* largest +subnormal */
	0x807fffffu,	/* largest -subnormal */
	0x00800000u,	/* smallest +normal */
	0x80800000u,	/* smallest -normal */
	0x00800001u,	/* just above smallest normal */
	0x3f000000u,	/* +0.5 */
	0xbf000000u,	/* -0.5 */
	0x3f800000u,	/* +1 */
	0xbf800000u,	/* -1 */
	0x40000000u,	/* +2 */
	0xc0000000u,	/* -2 */
	0x40490fdbu,	/* +pi */
	0xc0490fdbu,	/* -pi */
	0x4b000000u,	/* 2^23 */
	0xcb000000u,	/* -2^23 */
	0x7f7fffffu,	/* FLT_MAX */
	0xff7fffffu,	/* -FLT_MAX */
	0x7f7ffffeu,	/* just below FLT_MAX */
	0x7f800000u,	/* +inf */
	0xff800000u,	/* -inf */
	0x7f800001u,	/* smallest +sNaN */
	0xff800001u,	/* smallest -sNaN */
	0x7fa00000u,	/* +sNaN */
	0xffa00000u,	/* -sNaN */
	0x7fbfffffu,	/* largest sNaN */
	0x7fc00000u,	/* +qNaN */
	0xffc00000u,	/* -qNaN */
	0x7fc00001u,	/* +qNaN, odd payload */
	0x7fffffffu,	/* +qNaN, all payload bits */
	0xffffffffu,	/* -qNaN, all payload bits */
	0x80808080u,	/* every byte >= 0x80 */
	0x7f7f7f7fu,	/* guard-byte pattern */
	0x00ff00ffu,
	0xff00ff00u,
	0x8000ffffu,
	0x0000ff80u,
	0x33333333u,
	0xb3333333u,
};
static const std::size_t NFVEC = sizeof(fvec) / sizeof(fvec[0]);

static void
edge_cases(void)
{
	std::size_t i, j;

	for (i = 0; i < NFVEC; i++)
		for (j = 0; j < NFVEC; j++)
			check_conjf(fvec[i], fvec[j], "cross");

	/* Every quadrant, both axes, signed zero and infinity combinations. */
	{
		static const float fq[] = {
			0.0f, -0.0f, 1.0f, -1.0f, 2.0f, -2.0f,
			__builtin_inff(), -__builtin_inff(),
			__builtin_nanf(""), -__builtin_nanf(""),
			FLT_MIN, -FLT_MIN, FLT_MAX, -FLT_MAX,
			1e-45f, -1e-45f,
		};
		std::size_t a, b;
		const std::size_t nf = sizeof(fq) / sizeof(fq[0]);

		for (a = 0; a < nf; a++)
			for (b = 0; b < nf; b++)
				check_conjf(fbits(fq[a]), fbits(fq[b]),
				    "quadrant");
	}

	/* Walk both sides of every float boundary that affects negation. */
	static const std::uint32_t kWalk[] = {
		0x00000000u, 0x80000000u, 0x00800000u, 0x007fffffu,
		0x7f800000u, 0x7f7fffffu, 0x7fffffffu, 0x3f800000u,
		0xbf800000u, 0x80800000u, 0xff800000u, 0xffffffffu,
	};
	for (std::size_t k = 0; k < sizeof kWalk / sizeof kWalk[0]; k++) {
		for (int d = -32; d <= 32; d++) {
			std::uint32_t re = kWalk[k] + (std::uint32_t)d;
			std::uint32_t im = kWalk[k] + (std::uint32_t)d;

			check_conjf(re, im, "walk");
			check_conjf(re, kWalk[k], "walk");
			check_conjf(kWalk[k], im, "walk");
			check_conjf(re, re ^ 0x80000000u, "walk");
		}
	}

	/* Single-component cases: real-only and imag-only. */
	for (i = 0; i < NFVEC; i++) {
		check_conjf(fvec[i], 0u, "real-only");
		check_conjf(0u, fvec[i], "imag-only");
		check_conjf(fvec[i], 0x80000000u, "imag-negzero");
		check_conjf(0x80000000u, fvec[i], "real-negzero");
	}
}

static std::uint64_t rng_state;

static std::uint64_t
rng_next(void)
{
	std::uint64_t z;

	rng_state += 0x9e3779b97f4a7c15ull;
	z = rng_state;
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ull;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebull;
	return (z ^ (z >> 31));
}

static const unsigned long long ITERS = 250000ull;

static void
random_sweep(void)
{
	unsigned long long i;

	rng_state = 0xd1ce4e5b91234567ull;
	for (i = 0; i < ITERS; i++) {
		std::uint64_t r = rng_next();
		std::uint32_t re = (std::uint32_t)r;
		std::uint32_t im = (std::uint32_t)(r >> 32);

		/*
		 * Every other iteration force one component onto an exact
		 * signed zero / one so the axis cases keep getting hit.
		 */
		if ((i & 7) == 0)
			im = (i & 8) ? 0x80000000u : 0x00000000u;
		if ((i & 7) == 1)
			re = (i & 8) ? 0x80000000u : 0x00000000u;

		check_conjf(re, im, "random");
	}
}

static void
row(const stat &s)
{
	std::printf("  %-14s %12llu %10llu   %s\n", s.name, s.cases, s.fails,
	    s.fails == 0 ? "PASS" : "FAIL");
}

int
main(void)
{
	unsigned long long fails;

	std::printf("pbsd batch b0080s1 differential test\n\n");

	edge_cases();
	random_sweep();

	std::printf("\n  %-14s %12s %10s   %s\n", "function", "cases",
	    "failures", "result");
	std::printf("  ----------------------------------------------------\n");
	row(st_conjf);

	fails = st_conjf.fails;
	std::printf("\n%s: %llu total failures\n",
	    fails == 0 ? "PASS" : "FAIL", fails);

	if (st_conjf.cases == 0) {
		std::printf("conjf: no cases were run\n");
		return 1;
	}

	return (fails == 0 ? 0 : 1);
}
