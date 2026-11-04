/*
 * Differential test for PBSD batch b0001.
 *
 * Every case is run against BOTH the C++23 port and the ref_ oracle built
 * from the original HardenedBSD C, and the following are compared:
 *
 *   - the return value, bit for bit;
 *   - the set of floating point exception flags raised by the call
 *     (these functions exist solely to manipulate the exception flags,
 *     so comparing return values alone would pass a broken port);
 *   - the rounding mode left behind by the call;
 *   - the above under every rounding mode and with a variety of
 *     pre-existing exception flags, since feholdexcept()/feupdateenv()
 *     must merge the caller's flags back in.
 *
 * None of the functions in this batch write to a buffer, return a pointer,
 * or carry iterator state, so the buffer-guard / offset-comparison /
 * drive-to-exhaustion protocols do not apply here.  signgam is a plain
 * object and is compared as one.
 */

import pbsd.lib.msun.src.b0001;

#include <cfenv>
#include <cfloat>
#include <climits>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>

extern "C" {
long ref_lrintf(float);
long long ref_llrint(double);
long long ref_llrintf(float);
extern int signgam;
}

namespace port = pbsd::lib_msun_src::b0001;

/* ------------------------------------------------------------------ */

namespace {

struct Stat {
	const char *name;
	unsigned long long cases;
	unsigned long long failures;
	unsigned long long reported;
};

Stat st_lrintf   = { "lrintf",  0, 0, 0 };
Stat st_llrint   = { "llrint",  0, 0, 0 };
Stat st_llrintf  = { "llrintf", 0, 0, 0 };
Stat st_signgam  = { "signgam", 0, 0, 0 };

const int kRoundModes[4] = {
	FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO
};

const char *
round_name(int m)
{
	switch (m) {
	case FE_TONEAREST:  return "TONEAREST";
	case FE_DOWNWARD:   return "DOWNWARD";
	case FE_UPWARD:     return "UPWARD";
	case FE_TOWARDZERO: return "TOWARDZERO";
	default:            return "?";
	}
}

/* The presets exercise the flag-merging done by feupdateenv(). */
const int kPresets[5] = {
	0,
	FE_INEXACT,
	FE_INVALID,
	FE_INEXACT | FE_INVALID,
	FE_ALL_EXCEPT
};

float
f32(uint32_t bits)
{
	float f;
	std::memcpy(&f, &bits, sizeof(f));
	return f;
}

double
f64(uint64_t bits)
{
	double d;
	std::memcpy(&d, &bits, sizeof(d));
	return d;
}

uint32_t
bits32(float f)
{
	uint32_t u;
	std::memcpy(&u, &f, sizeof(u));
	return u;
}

uint64_t
bits64(double d)
{
	uint64_t u;
	std::memcpy(&u, &d, sizeof(u));
	return u;
}

/*
 * Result of one invocation: return value plus every observable piece of
 * floating point environment state.
 */
struct Obs {
	long long ret;
	int raised;
	int round_after;
};

bool
operator!=(const Obs &a, const Obs &b)
{
	return a.ret != b.ret || a.raised != b.raised ||
	    a.round_after != b.round_after;
}

void
prepare(int mode, int preset)
{
	std::fesetround(mode);
	std::feclearexcept(FE_ALL_EXCEPT);
	if (preset != 0)
		std::feraiseexcept(preset);
}

/*
 * volatile in/out keeps the compiler from folding the call away or
 * hoisting it across the fenv manipulation.
 */
template <typename T, typename R>
Obs
run(R (*fp)(T), T x, int mode, int preset)
{
	volatile T arg = x;
	Obs o;

	prepare(mode, preset);
	volatile R r = fp(arg);
	o.raised = std::fetestexcept(FE_ALL_EXCEPT);
	o.round_after = std::fegetround();
	o.ret = (long long)r;
	return o;
}

void
report_f(Stat &s, uint32_t xbits, int mode, int preset, const Obs &p,
    const Obs &r, const char *tag)
{
	s.failures++;
	if (s.reported >= 20)
		return;
	s.reported++;
	std::printf("  FAIL %-8s %-6s x=0x%08x (%.9g) round=%s preset=0x%02x\n",
	    s.name, tag, (unsigned)xbits, (double)f32(xbits),
	    round_name(mode), (unsigned)preset);
	std::printf("       port: ret=%lld raised=0x%02x round_after=%s\n",
	    p.ret, (unsigned)p.raised, round_name(p.round_after));
	std::printf("       ref : ret=%lld raised=0x%02x round_after=%s\n",
	    r.ret, (unsigned)r.raised, round_name(r.round_after));
}

void
report_d(Stat &s, uint64_t xbits, int mode, int preset, const Obs &p,
    const Obs &r, const char *tag)
{
	s.failures++;
	if (s.reported >= 20)
		return;
	s.reported++;
	std::printf("  FAIL %-8s %-6s x=0x%016llx (%.17g) round=%s "
	    "preset=0x%02x\n", s.name, tag, (unsigned long long)xbits,
	    f64(xbits), round_name(mode), (unsigned)preset);
	std::printf("       port: ret=%lld raised=0x%02x round_after=%s\n",
	    p.ret, (unsigned)p.raised, round_name(p.round_after));
	std::printf("       ref : ret=%lld raised=0x%02x round_after=%s\n",
	    r.ret, (unsigned)r.raised, round_name(r.round_after));
}

/*
 * Reachability probe for the `if (fetestexcept(FE_INVALID))
 * feclearexcept(FE_INEXACT);' guard.
 *
 * The guard only has an observable effect when the roundit() call raised
 * FE_INEXACT *and* the subsequent conversion to dtype raised FE_INVALID.
 * This instrument reproduces the state the function sees internally --
 * flags cleared by feholdexcept(), then roundit(), then the conversion --
 * and counts how often each flag, and both together, actually occur.  It
 * touches neither the port nor the oracle; it exists so the coverage of
 * that branch is reported as a measurement instead of being assumed.
 */
struct GuardCov {
	const char *name;
	unsigned long long invalid;
	unsigned long long inexact;
	unsigned long long both;
};

GuardCov cov_lrintf  = { "lrintf",  0, 0, 0 };
GuardCov cov_llrint  = { "llrint",  0, 0, 0 };
GuardCov cov_llrintf = { "llrintf", 0, 0, 0 };

template <typename T, typename R>
void
probe_guard(GuardCov &c, T (*roundit)(T), T x, int mode)
{
	volatile T arg = x;
	int flags;

	std::fesetround(mode);
	std::feclearexcept(FE_ALL_EXCEPT);
	volatile T y = roundit(arg);
	volatile R d = (R)y;
	(void)d;
	flags = std::fetestexcept(FE_ALL_EXCEPT);

	if (flags & FE_INVALID)
		c.invalid++;
	if (flags & FE_INEXACT)
		c.inexact++;
	if ((flags & FE_INVALID) && (flags & FE_INEXACT))
		c.both++;
	std::feclearexcept(FE_ALL_EXCEPT);
}

/* One float case, both orderings (port-first and ref-first). */
void
check_lrintf(uint32_t xbits, int mode, int preset)
{
	float x = f32(xbits);
	Obs p, r;

	p = run<float, long>(port::lrintf, x, mode, preset);
	r = run<float, long>(ref_lrintf, x, mode, preset);
	st_lrintf.cases++;
	if (p != r)
		report_f(st_lrintf, xbits, mode, preset, p, r, "p1st");

	r = run<float, long>(ref_lrintf, x, mode, preset);
	p = run<float, long>(port::lrintf, x, mode, preset);
	st_lrintf.cases++;
	if (p != r)
		report_f(st_lrintf, xbits, mode, preset, p, r, "r1st");

	probe_guard<float, long>(cov_lrintf, ::rintf, x, mode);
}

void
check_llrintf(uint32_t xbits, int mode, int preset)
{
	float x = f32(xbits);
	Obs p, r;

	p = run<float, long long>(port::llrintf, x, mode, preset);
	r = run<float, long long>(ref_llrintf, x, mode, preset);
	st_llrintf.cases++;
	if (p != r)
		report_f(st_llrintf, xbits, mode, preset, p, r, "p1st");

	r = run<float, long long>(ref_llrintf, x, mode, preset);
	p = run<float, long long>(port::llrintf, x, mode, preset);
	st_llrintf.cases++;
	if (p != r)
		report_f(st_llrintf, xbits, mode, preset, p, r, "r1st");

	probe_guard<float, long long>(cov_llrintf, ::rintf, x, mode);
}

void
check_llrint(uint64_t xbits, int mode, int preset)
{
	double x = f64(xbits);
	Obs p, r;

	p = run<double, long long>(port::llrint, x, mode, preset);
	r = run<double, long long>(ref_llrint, x, mode, preset);
	st_llrint.cases++;
	if (p != r)
		report_d(st_llrint, xbits, mode, preset, p, r, "p1st");

	r = run<double, long long>(ref_llrint, x, mode, preset);
	p = run<double, long long>(port::llrint, x, mode, preset);
	st_llrint.cases++;
	if (p != r)
		report_d(st_llrint, xbits, mode, preset, p, r, "r1st");

	probe_guard<double, long long>(cov_llrint, ::rint, x, mode);
}

/* ------------------------------------------------------------------ */
/* Hand-written edge cases.                                           */

const float kFloatEdges[] = {
	/* zeroes and the smallest magnitudes */
	0.0f, -0.0f,
	f32(0x00000001u), f32(0x80000001u),	/* min subnormal */
	f32(0x007fffffu), f32(0x807fffffu),	/* max subnormal */
	FLT_MIN, -FLT_MIN,
	/* sub-one, around every tie and every rounding boundary */
	f32(0x3effffffu), -f32(0x3effffffu),	/* nextafter(0.5, 0) */
	0.5f, -0.5f,
	f32(0x3f000001u), -f32(0x3f000001u),	/* nextafter(0.5, 1) */
	0.25f, -0.25f, 0.75f, -0.75f,
	1.0f, -1.0f, 1.5f, -1.5f, 2.5f, -2.5f, 3.5f, -3.5f,
	4.5f, -4.5f,
	f32(0x3fbfffffu), -f32(0x3fbfffffu),	/* nextafter(1.5, 1) */
	f32(0x3fc00001u), -f32(0x3fc00001u),	/* nextafter(1.5, 2) */
	/* mantissa boundaries: last float with a fraction, first without */
	8388607.5f, -8388607.5f,		/* 2^23 - 0.5 */
	8388608.0f, -8388608.0f,		/* 2^23 */
	16777216.0f, -16777216.0f,		/* 2^24 */
	/* 32 bit boundaries */
	2147483520.0f, -2147483520.0f,		/* largest float < 2^31 */
	2147483648.0f, -2147483648.0f,		/* 2^31 */
	2147483904.0f, -2147483904.0f,		/* smallest float > 2^31 */
	4294967040.0f, -4294967040.0f,		/* largest float < 2^32 */
	4294967296.0f, -4294967296.0f,		/* 2^32 */
	/* 64 bit boundaries: where long/long long overflow */
	f32(0x5effffffu), -f32(0x5effffffu),	/* largest float < 2^63 */
	9223372036854775808.0f,			/* 2^63 */
	-9223372036854775808.0f,
	f32(0x5f000001u), -f32(0x5f000001u),	/* smallest float > 2^63 */
	f32(0x5f7fffffu), -f32(0x5f7fffffu),	/* largest float < 2^64 */
	18446744073709551616.0f,		/* 2^64 */
	-18446744073709551616.0f,
	/* extremes */
	FLT_MAX, -FLT_MAX,
	f32(0x7f800000u), f32(0xff800000u),	/* +inf, -inf */
	/* NaNs: quiet, negative quiet, signalling, max payload */
	f32(0x7fc00000u), f32(0xffc00000u),
	f32(0x7f800001u), f32(0xff800001u),
	f32(0x7fbfffffu), f32(0x7fffffffu),
	/* miscellaneous magnitudes */
	1e-30f, -1e-30f, 1e30f, -1e30f,
	123456.789f, -123456.789f,
	f32(0x4b7fffffu), f32(0xcb7fffffu),
};

const double kDoubleEdges[] = {
	0.0, -0.0,
	f64(0x0000000000000001ull), f64(0x8000000000000001ull),
	f64(0x000fffffffffffffull), f64(0x800fffffffffffffull),
	DBL_MIN, -DBL_MIN,
	f64(0x3fdfffffffffffffull), -f64(0x3fdfffffffffffffull),
	0.5, -0.5,
	f64(0x3fe0000000000001ull), -f64(0x3fe0000000000001ull),
	0.25, -0.25, 0.75, -0.75,
	1.0, -1.0, 1.5, -1.5, 2.5, -2.5, 3.5, -3.5, 4.5, -4.5,
	f64(0x3ff7ffffffffffffull), -f64(0x3ff7ffffffffffffull),
	f64(0x3ff8000000000001ull), -f64(0x3ff8000000000001ull),
	/* mantissa boundaries */
	4503599627370495.5, -4503599627370495.5,	/* 2^52 - 0.5 */
	4503599627370496.0, -4503599627370496.0,	/* 2^52 */
	9007199254740992.0, -9007199254740992.0,	/* 2^53 */
	/* 32 bit boundaries */
	2147483647.0, -2147483647.0,
	2147483647.5, -2147483647.5,
	2147483648.0, -2147483648.0,
	4294967295.0, -4294967295.0,
	4294967296.0, -4294967296.0,
	/* 64 bit boundaries: where long long overflows */
	f64(0x43dfffffffffffffull), -f64(0x43dfffffffffffffull),
	9223372036854775808.0, -9223372036854775808.0,	/* 2^63 */
	f64(0x43e0000000000001ull), -f64(0x43e0000000000001ull),
	f64(0x43efffffffffffffull), -f64(0x43efffffffffffffull),
	18446744073709551616.0, -18446744073709551616.0,
	/* extremes */
	DBL_MAX, -DBL_MAX,
	f64(0x7ff0000000000000ull), f64(0xfff0000000000000ull),
	/* NaNs */
	f64(0x7ff8000000000000ull), f64(0xfff8000000000000ull),
	f64(0x7ff0000000000001ull), f64(0xfff0000000000001ull),
	f64(0x7ff7ffffffffffffull), f64(0x7fffffffffffffffull),
	/* miscellaneous magnitudes */
	1e-300, -1e-300, 1e300, -1e300,
	123456.789, -123456.789,
	1e18, -1e18, 1e19, -1e19,
};

/* ------------------------------------------------------------------ */
/* Fixed-seed randomised sweep.                                       */

uint64_t rng_state;

void
rng_seed(uint64_t s)
{
	rng_state = s;
}

uint64_t
rng_next(void)
{
	uint64_t z = (rng_state += 0x9e3779b97f4a7c15ull);
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ull;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebull;
	return z ^ (z >> 31);
}

/*
 * Six generators.  Uniform bit patterns alone would spend nearly all of
 * their time on absurd exponents, so most strategies steer the exponent
 * into the ranges where rounding, the 2^31 boundary and the 2^63
 * overflow boundary actually live.  Nothing here narrows the input set:
 * strategy 0 still covers the entire encoding space.
 */
uint32_t
random_float_bits(unsigned strategy)
{
	uint64_t r = rng_next();
	uint32_t sign = (uint32_t)(r >> 63) << 31;
	uint32_t mant = (uint32_t)(r & 0x7fffffu);
	int exp;

	switch (strategy % 6) {
	case 0:
		return (uint32_t)r;			/* whole space */
	case 1:
		exp = 127 + (int)((r >> 24) % 9) - 4;	/* 2^-4 .. 2^4 */
		break;
	case 2:
		exp = 127 + (int)((r >> 24) % 26);	/* 2^0 .. 2^25 */
		break;
	case 3:
		exp = 127 + 31 + (int)((r >> 24) % 5) - 2;   /* around 2^31 */
		break;
	case 4:
		exp = 127 + 63 + (int)((r >> 24) % 5) - 2;   /* around 2^63 */
		break;
	default:
		exp = (int)((r >> 24) % 255) + 1;	/* all finite exps */
		break;
	}
	return sign | ((uint32_t)exp << 23) | mant;
}

uint64_t
random_double_bits(unsigned strategy)
{
	uint64_t r = rng_next();
	uint64_t r2 = rng_next();
	uint64_t sign = (r >> 63) << 63;
	uint64_t mant = r2 & 0xfffffffffffffull;
	int exp;

	switch (strategy % 6) {
	case 0:
		return r ^ (r2 << 1);			/* whole space */
	case 1:
		exp = 1023 + (int)((r >> 24) % 9) - 4;
		break;
	case 2:
		exp = 1023 + (int)((r >> 24) % 55);
		break;
	case 3:
		exp = 1023 + 31 + (int)((r >> 24) % 5) - 2;
		break;
	case 4:
		exp = 1023 + 63 + (int)((r >> 24) % 5) - 2;
		break;
	default:
		exp = (int)((r >> 24) % 2047) + 1;
		break;
	}
	return sign | ((uint64_t)exp << 52) | mant;
}

/* ------------------------------------------------------------------ */

void
test_signgam(void)
{
	static const int kValues[] = {
		0, 1, -1, INT_MAX, INT_MIN, 0x7f, -0x80, 0x80, 0xff, 42, 0
	};

	/* Initial value, before anything has touched either object. */
	st_signgam.cases++;
	if (port::signgam != signgam) {
		st_signgam.failures++;
		std::printf("  FAIL signgam initial: port=%d ref=%d\n",
		    port::signgam, signgam);
	}

	for (unsigned i = 0; i < sizeof(kValues) / sizeof(kValues[0]); i++) {
		port::signgam = kValues[i];
		signgam = kValues[i];
		st_signgam.cases++;
		if (port::signgam != signgam) {
			st_signgam.failures++;
			if (st_signgam.reported < 20) {
				st_signgam.reported++;
				std::printf("  FAIL signgam store %d: "
				    "port=%d ref=%d\n", kValues[i],
				    port::signgam, signgam);
			}
		}
	}

	/* They must be distinct objects, exactly as the two translation
	 * units define them. */
	port::signgam = 0;
	signgam = 0;
	st_signgam.cases++;
	if (&port::signgam == &signgam) {
		st_signgam.failures++;
		std::printf("  FAIL signgam: port and ref alias\n");
	}
}

const unsigned long long kSweepIters = 250000;

void
run_all(void)
{
	const unsigned nfe = sizeof(kFloatEdges) / sizeof(kFloatEdges[0]);
	const unsigned nde = sizeof(kDoubleEdges) / sizeof(kDoubleEdges[0]);

	for (int mi = 0; mi < 4; mi++) {
		int mode = kRoundModes[mi];
		for (int pi = 0; pi < 5; pi++) {
			int preset = kPresets[pi];

			for (unsigned i = 0; i < nfe; i++) {
				uint32_t b = bits32(kFloatEdges[i]);
				check_lrintf(b, mode, preset);
				check_llrintf(b, mode, preset);
			}
			for (unsigned i = 0; i < nde; i++) {
				uint64_t b = bits64(kDoubleEdges[i]);
				check_llrint(b, mode, preset);
			}
			/* Doubles promoted from the float edge list, and
			 * floats demoted from the double edge list. */
			for (unsigned i = 0; i < nfe; i++)
				check_llrint(bits64((double)kFloatEdges[i]),
				    mode, preset);
			for (unsigned i = 0; i < nde; i++) {
				uint32_t b = bits32((float)kDoubleEdges[i]);
				check_lrintf(b, mode, preset);
				check_llrintf(b, mode, preset);
			}
		}
	}

	rng_seed(0x5eed1234abcd0001ull);
	for (int mi = 0; mi < 4; mi++) {
		int mode = kRoundModes[mi];
		for (unsigned long long n = 0; n < kSweepIters; n++) {
			int preset = kPresets[(unsigned)(n % 5)];
			uint32_t fb = random_float_bits((unsigned)n);
			uint64_t db = random_double_bits((unsigned)n);

			check_lrintf(fb, mode, preset);
			check_llrintf(fb, mode, preset);
			check_llrint(db, mode, preset);
		}
	}

	test_signgam();
}

void
row(const Stat &s)
{
	std::printf("  %-12s %14llu %14llu   %s\n", s.name, s.cases,
	    s.failures, s.failures == 0 ? "PASS" : "FAIL");
}

void
covrow(const GuardCov &c)
{
	std::printf("  %-12s %14llu %14llu %14llu\n", c.name, c.invalid,
	    c.inexact, c.both);
}

} /* namespace */

int
main(void)
{
	run_all();

	std::printf("\n");
	std::printf("  %-12s %14s %14s   %s\n", "function", "cases",
	    "failures", "result");
	std::printf("  ------------------------------------------------"
	    "------\n");
	row(st_lrintf);
	row(st_llrint);
	row(st_llrintf);
	row(st_signgam);
	std::printf("\n");

	unsigned long long total = st_lrintf.cases + st_llrint.cases +
	    st_llrintf.cases + st_signgam.cases;
	unsigned long long bad = st_lrintf.failures + st_llrint.failures +
	    st_llrintf.failures + st_signgam.failures;

	std::printf("  total %llu cases, %llu failures: %s\n", total, bad,
	    bad == 0 ? "OK" : "DIVERGENCE");

	std::printf("\n  branch coverage of \"if (fetestexcept(FE_INVALID)) "
	    "feclearexcept(FE_INEXACT);\"\n");
	std::printf("  %-12s %14s %14s %14s\n", "function", "FE_INVALID",
	    "FE_INEXACT", "both (taken)");
	std::printf("  ------------------------------------------------"
	    "--------------------\n");
	covrow(cov_lrintf);
	covrow(cov_llrint);
	covrow(cov_llrintf);
	std::printf("\n");

	return bad == 0 ? 0 : 1;
}
