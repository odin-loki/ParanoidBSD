/*
 * PBSD batch b0093s4 -- differential test.
 *
 * Compares the C++23 port (module pbsd.lib.msun.ld80.b0093s4) against the
 * unmodified C oracle (oracle.c) for:
 *
 *	tanpil            <->  ref_tanpil
 *	__kernel_tanpil   <->  ref___kernel_tanpil
 *
 * Neither function writes to a buffer and neither returns a pointer, so the
 * guard-byte / offset machinery does not apply.  The equivalent strictness is
 * obtained by comparing the FULL 80-bit representation of the result (all 64
 * mantissa bits plus the 16-bit expsign word) rather than using operator==.
 * That distinguishes +0 from -0 and distinguishes NaN payloads, both of which
 * a plain == comparison would silently accept.
 */

#include <cstdio>
#include <cstdint>
#include <cmath>
#include <cstring>
#include <vector>

import pbsd.lib.msun.ld80.b0093s4;

extern "C" long double ref_tanpil(long double);
extern "C" long double ref___kernel_tanpil(long double);

namespace port = pbsd::lib_msun_ld80::b0093s4;

/* ------------------------------------------------------------------ bits */

union ldbits {
	long double	e;
	struct {
		unsigned long	man	:64;
		unsigned int	expsign	:16;
		unsigned long	junk	:48;
	} xbits;
};

static long double
mk(std::uint16_t expsign, std::uint64_t man)
{
	ldbits u;
	u.xbits.man = man;
	u.xbits.expsign = expsign;
	return u.e;
}

/* Build a value with a *valid* x87 extended encoding. */
static long double
mk_valid(std::uint16_t expsign, std::uint64_t man)
{
	if ((expsign & 0x7fff) == 0)
		man &= ~(std::uint64_t)1 << 63;
	else
		man |= (std::uint64_t)1 << 63;
	return mk(expsign, man);
}

static std::uint16_t
bits_expsign(long double v)
{
	ldbits u;
	u.e = v;
	return (std::uint16_t)u.xbits.expsign;
}

static std::uint64_t
bits_man(long double v)
{
	ldbits u;
	u.e = v;
	return (std::uint64_t)u.xbits.man;
}

static bool
same_bits(long double a, long double b)
{
	return bits_expsign(a) == bits_expsign(b) && bits_man(a) == bits_man(b);
}

/* ------------------------------------------------------------------- rng */

static std::uint64_t rng_state;

static void
rng_seed(std::uint64_t s)
{
	rng_state = s;
}

static std::uint64_t
rnd(void)
{
	std::uint64_t z = (rng_state += 0x9E3779B97F4A7C15ull);
	z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
	z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
	return z ^ (z >> 31);
}

/* --------------------------------------------------------------- checker */

struct fnstat {
	const char	*name;
	long long	cases;
	long long	fails;
	int		printed;
};

static fnstat st_tanpil        = { "tanpil",          0, 0, 0 };
static fnstat st_kernel_tanpil = { "__kernel_tanpil", 0, 0, 0 };

static void
report(fnstat &s, const char *tag, long double x, long double got,
    long double want)
{
	s.fails++;
	if (s.printed < 25) {
		s.printed++;
		std::printf("  FAIL %-16s %-22s x=%04x:%016llx  "
		    "port=%04x:%016llx  ref=%04x:%016llx\n",
		    s.name, tag,
		    (unsigned)bits_expsign(x),
		    (unsigned long long)bits_man(x),
		    (unsigned)bits_expsign(got),
		    (unsigned long long)bits_man(got),
		    (unsigned)bits_expsign(want),
		    (unsigned long long)bits_man(want));
	}
}

static void
check_tanpil(long double x, const char *tag)
{
	st_tanpil.cases++;
	long double got = port::tanpil(x);
	long double want = ref_tanpil(x);
	if (!same_bits(got, want))
		report(st_tanpil, tag, x, got, want);
}

static void
check_kernel(long double x, const char *tag)
{
	st_kernel_tanpil.cases++;
	long double got = port::__kernel_tanpil(x);
	long double want = ref___kernel_tanpil(x);
	if (!same_bits(got, want))
		report(st_kernel_tanpil, tag, x, got, want);
}

/* Every value is tested with both signs. */
static void
check_both(long double x, const char *tag)
{
	check_tanpil(x, tag);
	check_tanpil(-x, tag);
	check_kernel(x, tag);
	check_kernel(-x, tag);
}

/* ------------------------------------------------------- hand-written set */

static void
edge_cases(void)
{
	/* Zeros, including the sign-of-zero path (x == 0 returns x). */
	check_both(0.0L, "zero");
	check_both(mk(0x8000, 0), "negzero-raw");

	/* Smallest subnormals and the subnormal/normal boundary. */
	check_both(mk_valid(0x0000, 0x0000000000000001ull), "denorm-min");
	check_both(mk_valid(0x0000, 0x0000000000000002ull), "denorm-2");
	check_both(mk_valid(0x0000, 0x00000000ffffffffull), "denorm-lo32");
	check_both(mk_valid(0x0000, 0x0000000100000000ull), "denorm-hi");
	check_both(mk_valid(0x0000, 0x7fffffffffffffffull), "denorm-max");
	check_both(mk_valid(0x0001, 0x8000000000000000ull), "min-normal");
	check_both(mk_valid(0x0001, 0x8000000000000001ull), "min-normal+1");

	/* The |x| < 0x1p-34 boundary (ix == 0x3fdd) from both sides. */
	for (int e = 0x3fd8; e <= 0x3fe2; e++) {
		check_both(mk_valid((std::uint16_t)e, 0x8000000000000000ull),
		    "p-34-boundary");
		check_both(mk_valid((std::uint16_t)e, 0x8000000000000001ull),
		    "p-34-boundary");
		check_both(mk_valid((std::uint16_t)e, 0xffffffffffffffffull),
		    "p-34-boundary");
		check_both(mk_valid((std::uint16_t)e, 0xffffffff00000000ull),
		    "p-34-boundary");
		check_both(mk_valid((std::uint16_t)e, 0x00000000ffffffffull),
		    "p-34-boundary");
	}
	{
		long double p34 = std::ldexp(1.0L, -34);
		check_both(p34, "0x1p-34");
		check_both(std::nextafterl(p34, 0.0L), "0x1p-34-ulp");
		check_both(std::nextafterl(p34, 1.0L), "0x1p-34+ulp");
	}

	/* Quarter grid over [0, 64]: integers, halves and quarters, plus one
	 * ulp either side of each. */
	for (int k = 0; k <= 256; k++) {
		long double v = (long double)k * 0.25L;
		check_both(v, "quarter-grid");
		check_both(std::nextafterl(v, -1.0e9L), "quarter-grid-ulp");
		check_both(std::nextafterl(v, 1.0e9L), "quarter-grid+ulp");
	}

	/* Eighths and sixteenths, to straddle 0.25 inside the kernel. */
	for (int k = 0; k <= 128; k++) {
		check_both((long double)k * 0.0625L, "sixteenth-grid");
		check_both((long double)k * 0.03125L, "thirtysecond-grid");
	}

	/* Dense sweep just around 0.25 and 0.75 (the kernel's t = 1 case and
	 * the reflection). */
	for (int i = -8; i <= 8; i++) {
		long double v = 0.25L;
		for (int j = 0; j < (i < 0 ? -i : i); j++)
			v = std::nextafterl(v, i < 0 ? 0.0L : 1.0L);
		check_both(v, "near-0.25");
		long double w = 0.75L;
		for (int j = 0; j < (i < 0 ? -i : i); j++)
			w = std::nextafterl(w, i < 0 ? 0.0L : 1.0L);
		check_both(w, "near-0.75");
		long double h = 0.5L;
		for (int j = 0; j < (i < 0 ? -i : i); j++)
			h = std::nextafterl(h, i < 0 ? 0.0L : 1.0L);
		check_both(h, "near-0.5");
		long double o = 1.0L;
		for (int j = 0; j < (i < 0 ? -i : i); j++)
			o = std::nextafterl(o, i < 0 ? 0.0L : 2.0L);
		check_both(o, "near-1.0");
	}

	/* Exponent boundaries around every branch test in tanpil. */
	static const std::uint16_t exps[] = {
		0x0000, 0x0001, 0x3fda, 0x3fdb, 0x3fdc, 0x3fdd, 0x3fde,
		0x3fdf, 0x3ffb, 0x3ffc, 0x3ffd, 0x3ffe, 0x3fff, 0x4000,
		0x4001, 0x401c, 0x401d, 0x401e, 0x401f, 0x4020, 0x403b,
		0x403c, 0x403d, 0x403e, 0x403f, 0x4040, 0x4041, 0x7ffd,
		0x7ffe, 0x7fff
	};
	static const std::uint64_t mans[] = {
		0x0000000000000000ull, 0x0000000000000001ull,
		0x0000000100000000ull, 0x00000000ffffffffull,
		0x8000000000000000ull, 0x8000000000000001ull,
		0x8000000080000000ull, 0x80000000ffffffffull,
		0xc000000000000000ull, 0xc90fdaa22168c235ull,
		0xfffffffffffffffeull, 0xffffffffffffffffull,
		0xaaaaaaaaaaaaaaaaull, 0x5555555555555555ull
	};
	for (unsigned i = 0; i < sizeof(exps) / sizeof(exps[0]); i++)
		for (unsigned j = 0; j < sizeof(mans) / sizeof(mans[0]); j++) {
			check_both(mk_valid(exps[i], mans[j]), "exp-grid");
			check_both(mk(exps[i], mans[j]), "exp-grid-raw-enc");
		}

	/* 0x1p63 <= |x| < 0x1p64: the even/odd integer decision on lx & 1. */
	for (int i = 0; i < 64; i++) {
		check_both(mk(0x403e, 0x8000000000000000ull | (std::uint64_t)i),
		    "p63-parity");
		check_both(mk(0x403e, 0xffffffffffffffffull - (std::uint64_t)i),
		    "p63-parity");
	}
	/* |x| >= 0x1p64: always even. */
	for (int e = 0x403f; e <= 0x4045; e++) {
		check_both(mk(( std::uint16_t)e, 0x8000000000000000ull),
		    "ge-p64");
		check_both(mk((std::uint16_t)e, 0x8000000000000001ull),
		    "ge-p64");
		check_both(mk((std::uint16_t)e, 0xffffffffffffffffull),
		    "ge-p64");
	}

	/* Large exact values straddling 0x1p63 (the ix < 0x403e test). */
	{
		long double p62 = std::ldexp(1.0L, 62);
		long double p63 = std::ldexp(1.0L, 63);
		long double p64 = std::ldexp(1.0L, 64);
		check_both(p62, "0x1p62");
		check_both(p62 + 0.5L, "0x1p62+0.5");
		check_both(p62 + 0.25L, "0x1p62+0.25");
		check_both(p62 + 1.0L, "0x1p62+1");
		check_both(std::nextafterl(p63, 0.0L), "0x1p63-ulp");
		check_both(p63, "0x1p63");
		check_both(std::nextafterl(p63, p64), "0x1p63+ulp");
		check_both(std::nextafterl(p64, 0.0L), "0x1p64-ulp");
		check_both(p64, "0x1p64");
		check_both(std::nextafterl(p64, 1.0e30L), "0x1p64+ulp");
	}

	/* Infinities and NaNs. */
	check_both(mk(0x7fff, 0x8000000000000000ull), "inf");
	check_both(mk(0x7fff, 0xc000000000000000ull), "qnan");
	check_both(mk(0x7fff, 0x8000000000000001ull), "snan");
	check_both(mk(0x7fff, 0xffffffffffffffffull), "nan-all-ones");
	check_both(mk(0x7fff, 0x0000000000000000ull), "pseudo-inf");

	/* LDBL_MAX and friends. */
	check_both(mk(0x7ffe, 0xffffffffffffffffull), "ldbl-max");

	/* Odd/even integer parity across a wide magnitude range. */
	for (int sh = 0; sh <= 62; sh++) {
		long double b = std::ldexp(1.0L, sh);
		check_both(b, "pow2");
		check_both(b + 1.0L, "pow2+1");
		check_both(b - 1.0L, "pow2-1");
		check_both(b + 0.5L, "pow2+0.5");
		check_both(b - 0.5L, "pow2-0.5");
		check_both(b + 0.25L, "pow2+0.25");
		check_both(b - 0.25L, "pow2-0.25");
	}

	/* Kernel-only: the whole [0, 1] range at fine resolution, so that both
	 * sides of the x < 0.25 / x > 0.25 / x == 0.25 three-way split and the
	 * sign change of 0.5 - x are exercised. */
	for (int k = 0; k <= 2000; k++) {
		long double v = (long double)k / 2000.0L;
		check_kernel(v, "kernel-unit");
		check_kernel(-v, "kernel-unit");
	}
}

/* ------------------------------------------------------- randomised sweep */

static void
random_sweep(void)
{
	const int N = 50000;
	int i;

	/* 1. Uniform over the whole encoding space (valid encodings). */
	for (i = 0; i < N; i++) {
		std::uint64_t r = rnd();
		std::uint16_t es = (std::uint16_t)(r & 0x7fff);
		if (rnd() & 1)
			es |= 0x8000;
		check_both(mk_valid(es, rnd()), "rand-anyexp");
	}

	/* 2. Uniform-ish over [-8, 8): dense in the |x| < 1 and small-integer
	 *    branches. */
	for (i = 0; i < N; i++) {
		std::int64_t r = (std::int64_t)rnd();
		long double v = (long double)r / 1152921504606846976.0L; /* 2^60 */
		check_both(v, "rand-small");
	}

	/* 3. Structured: integer part + a quarter-grid fraction (+/- noise).
	 *    Drives the ix == 0 / ax == 0.5 / reflection branches. */
	for (i = 0; i < N; i++) {
		std::uint64_t n = rnd() % 100000000ull;
		unsigned q = (unsigned)(rnd() & 3);
		long double v = (long double)n + (long double)q * 0.25L;
		unsigned k = (unsigned)(rnd() % 5);
		for (unsigned j = 0; j < k; j++)
			v = std::nextafterl(v, (rnd() & 1) ? 0.0L : 1.0e30L);
		check_both(v, "rand-quarter");
	}

	/* 4. Exponents spanning both FFLOORL80 shift branches and the
	 *    0x403e / 0x403f cut-offs. */
	for (i = 0; i < N; i++) {
		std::uint16_t es = (std::uint16_t)(0x3ff0 + (rnd() % 0x0055));
		if (rnd() & 1)
			es |= 0x8000;
		check_both(mk_valid(es, rnd()), "rand-ffloor");
	}

	/* 5. Kernel domain plus its callers' arguments: [0, 1] and the exact
	 *    quarter neighbourhood. */
	for (i = 0; i < N; i++) {
		std::uint64_t r = rnd();
		long double v = (long double)(r >> 1) /
		    9223372036854775808.0L; /* 2^63 -> [0,1) */
		check_kernel(v, "rand-kernel");
		check_kernel(-v, "rand-kernel");
		check_kernel(0.25L + (v - 0.5L) * 1.0e-18L, "rand-kernel-q");
		check_tanpil(v, "rand-unit");
		check_tanpil(-v, "rand-unit");
	}
}

/* ------------------------------------------------------------------ main */

int
main(void)
{
	rng_seed(0x123456789ABCDEFull);

	std::printf("PBSD b0093s4 differential test "
	    "(port vs. oracle, full 80-bit compare)\n\n");

	edge_cases();
	random_sweep();

	std::printf("\n%-20s %12s %12s\n", "function", "cases", "failures");
	std::printf("%-20s %12s %12s\n", "--------------------",
	    "------------", "------------");
	std::printf("%-20s %12lld %12lld\n", st_tanpil.name,
	    st_tanpil.cases, st_tanpil.fails);
	std::printf("%-20s %12lld %12lld\n", st_kernel_tanpil.name,
	    st_kernel_tanpil.cases, st_kernel_tanpil.fails);

	long long total = st_tanpil.cases + st_kernel_tanpil.cases;
	long long bad = st_tanpil.fails + st_kernel_tanpil.fails;
	std::printf("%-20s %12lld %12lld\n", "TOTAL", total, bad);
	std::printf("\n%s\n", bad == 0 ? "PASS" : "FAIL");

	return bad == 0 ? 0 : 1;
}
