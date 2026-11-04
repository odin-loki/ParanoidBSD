/*
 * harness.cpp -- differential test for PBSD batch b0010s1.
 *
 * Compares the C++23 port in port.cppm against the unmodified C reference in
 * oracle.c for every function in the batch:
 *
 *   nedf2      (== __nedf2 of nedf2.c, the batch function)
 *   float64_eq (the SoftFloat primitive __nedf2 is built out of)
 *
 * Neither function writes through a pointer, so the "two guard buffers"
 * protocol is applied to the *result*: both observations are recorded into
 * their own 32-byte buffer pre-filled with the guard byte 0x7f, the result is
 * written at a fixed offset, and the ENTIRE buffer is compared afterwards.
 * That catches a result whose width or padding differs between port and
 * reference, not merely a differing value.  The accumulated
 * float_exception_flags word is compared too, so the signalling-NaN path is
 * observable beyond its return value.
 *
 * Inputs are raw float64 bit patterns.  The hand-written set covers the
 * degenerate patterns (all-zero words, single-bit words, every repeated byte
 * 0x00-0xff including the whole high-bit range 0x80-0xff, the guard pattern
 * itself) and both sides of every boundary in the code under test:
 * exponent == 0x7ff vs 0x7fe, fraction == 0 vs != 0 at that exponent
 * (infinity vs NaN), equal bit patterns vs one bit apart, the +0/-0 pair that
 * makes the (a|b)<<1 == 0 term decide equality, and signalling vs quiet NaN.
 */

#include <cstdio>
#include <cstring>
#include <cstdint>
#include <cstdlib>

import pbsd.lib.libc.softfloat.b0010s1;

namespace port = pbsd::lib_libc_softfloat::b0010s1;

extern "C" {
char ref___nedf2(unsigned long long a, unsigned long long b);
char float64_eq(unsigned long long a, unsigned long long b);
extern unsigned int float_exception_flags;
}

typedef unsigned long long u64;

enum { GUARD = 32, SLOT = 8 };

struct Obs {
	int ret;
	unsigned int flags;
	unsigned char buf[GUARD];
};

static Obs
port_nedf2(u64 a, u64 b)
{
	Obs o;
	memset(o.buf, 0x7f, sizeof o.buf);
	port::float_exception_flags = 0;
	port::flag r = port::nedf2(a, b);
	memcpy(o.buf + SLOT, &r, sizeof r);
	o.ret = (int)r;
	o.flags = (unsigned int)port::float_exception_flags;
	return o;
}

static Obs
ref_nedf2(u64 a, u64 b)
{
	Obs o;
	memset(o.buf, 0x7f, sizeof o.buf);
	float_exception_flags = 0;
	char r = ref___nedf2(a, b);
	memcpy(o.buf + SLOT, &r, sizeof r);
	o.ret = (int)r;
	o.flags = float_exception_flags;
	return o;
}

static Obs
port_eq(u64 a, u64 b)
{
	Obs o;
	memset(o.buf, 0x7f, sizeof o.buf);
	port::float_exception_flags = 0;
	port::flag r = port::float64_eq(a, b);
	memcpy(o.buf + SLOT, &r, sizeof r);
	o.ret = (int)r;
	o.flags = (unsigned int)port::float_exception_flags;
	return o;
}

static Obs
ref_eq(u64 a, u64 b)
{
	Obs o;
	memset(o.buf, 0x7f, sizeof o.buf);
	float_exception_flags = 0;
	char r = float64_eq(a, b);
	memcpy(o.buf + SLOT, &r, sizeof r);
	o.ret = (int)r;
	o.flags = float_exception_flags;
	return o;
}

struct Fn {
	const char *name;
	Obs (*port_fn)(u64, u64);
	Obs (*ref_fn)(u64, u64);
	unsigned long long cases;
	unsigned long long fails;
	int reported;
};

static Fn fns[] = {
	{ "nedf2 (__nedf2)", port_nedf2, ref_nedf2, 0, 0, 0 },
	{ "float64_eq",      port_eq,    ref_eq,    0, 0, 0 },
};

enum { NFN = (int)(sizeof fns / sizeof fns[0]) };

static void
run_case(u64 a, u64 b, const char *tag)
{
	for (int i = 0; i < NFN; i++) {
		Fn &f = fns[i];
		Obs p = f.port_fn(a, b);
		Obs r = f.ref_fn(a, b);
		f.cases++;
		int bufdiff = memcmp(p.buf, r.buf, GUARD) != 0;
		if (p.ret != r.ret || p.flags != r.flags || bufdiff) {
			f.fails++;
			if (f.reported < 8) {
				f.reported++;
				printf("FAIL %-16s [%s] a=%016llx b=%016llx "
				    "port{ret=%d flags=0x%x} ref{ret=%d "
				    "flags=0x%x} buffer=%s\n",
				    f.name, tag, a, b, p.ret, p.flags,
				    r.ret, r.flags,
				    bufdiff ? "DIFFERS" : "equal");
			}
		}
	}
}

/*
 * Hand-picked patterns.  Every one of these appears on both sides of every
 * pair, so all sign/class combinations are exercised in both argument
 * positions.
 */
static const u64 base_pool[] = {
	0x0000000000000000ULL,	/* +0 */
	0x8000000000000000ULL,	/* -0 */
	0x0000000000000001ULL,	/* smallest +denormal */
	0x8000000000000001ULL,	/* smallest -denormal */
	0x000FFFFFFFFFFFFFULL,	/* largest +denormal (frac all ones, exp 0) */
	0x800FFFFFFFFFFFFFULL,
	0x0010000000000000ULL,	/* smallest +normal */
	0x8010000000000000ULL,
	0x0008000000000000ULL,	/* denormal, frac top bit only */
	0x8008000000000000ULL,
	0x3FE0000000000000ULL,	/* +0.5 */
	0xBFE0000000000000ULL,	/* -0.5 */
	0x3FF0000000000000ULL,	/* +1.0 */
	0xBFF0000000000000ULL,	/* -1.0 */
	0x3FF0000000000001ULL,	/* nextafter(+1, +inf) */
	0xBFF0000000000001ULL,
	0x4000000000000000ULL,	/* +2.0 */
	0xC000000000000000ULL,	/* -2.0 */
	0x4008000000000000ULL,	/* +3.0 */
	0xC008000000000000ULL,	/* -3.0 */
	0x7FE0000000000000ULL,	/* exp 0x7fe, frac 0: finite, one below inf */
	0xFFE0000000000000ULL,
	0x7FEFFFFFFFFFFFFFULL,	/* +DBL_MAX */
	0xFFEFFFFFFFFFFFFFULL,	/* -DBL_MAX */
	0x7FF0000000000000ULL,	/* +inf: exp 0x7ff, frac == 0 */
	0xFFF0000000000000ULL,	/* -inf */
	0x7FF0000000000001ULL,	/* +NaN, smallest fraction (signalling) */
	0xFFF0000000000001ULL,
	0x7FF8000000000000ULL,	/* +quiet NaN */
	0xFFF8000000000000ULL,
	0x7FFFFFFFFFFFFFFFULL,	/* NaN, fraction all ones */
	0xFFFFFFFFFFFFFFFFULL,
	0x7F7F7F7F7F7F7F7FULL,	/* the guard byte pattern itself */
	0x8080808080808080ULL,	/* high-bit bytes throughout */
	0x00000000FFFFFFFFULL,
	0xFFFFFFFF00000000ULL,
	0x0000000080000000ULL,
	0x00FF00FF00FF00FFULL,
	0xFF00FF00FF00FF00ULL,
	0x0101010101010101ULL,
};

enum { NBASE = (int)(sizeof base_pool / sizeof base_pool[0]) };

/* base_pool + every repeated byte 0x00..0xff + every top byte 0x00..0xff. */
enum { NPOOL = NBASE + 256 + 256 };
static u64 pool[NPOOL];

static void
build_pool(void)
{
	int n = 0;
	for (int i = 0; i < NBASE; i++)
		pool[n++] = base_pool[i];
	for (int b = 0; b <= 0xFF; b++) {
		u64 v = 0;
		for (int k = 0; k < 8; k++)
			v = (v << 8) | (u64)b;
		pool[n++] = v;
	}
	/*
	 * Top byte sweeps sign + high exponent bits across every value,
	 * with a low bit set so exp==0x7ff cases here are NaN rather than
	 * infinity; the base pool supplies the frac==0 counterparts.
	 */
	for (int b = 0; b <= 0xFF; b++)
		pool[n++] = ((u64)b << 56) | 1ULL;
}

static uint64_t rng_state;

static uint64_t
next_rand(void)
{
	uint64_t z = (rng_state += 0x9E3779B97F4A7C15ULL);
	z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
	z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
	return z ^ (z >> 31);
}

int
main(void)
{
	build_pool();

	/*
	 * 1. Targeted cases.  Each pair below is one where a single flipped
	 *    operator, comparison or constant in the port changes the answer,
	 *    so they pin the code down from both sides of each boundary.
	 */
	run_case(0x3FF0000000000000ULL, 0x3FF0000000000000ULL, "eq +1,+1");
	run_case(0x3FF0000000000000ULL, 0x4000000000000000ULL, "neq +1,+2");
	run_case(0x4000000000000000ULL, 0x3FF0000000000000ULL, "neq +2,+1");
	run_case(0xBFF0000000000000ULL, 0xC000000000000000ULL, "neq -1,-2");
	run_case(0xC000000000000000ULL, 0xBFF0000000000000ULL, "neq -2,-1");
	run_case(0xC000000000000000ULL, 0xC000000000000000ULL, "eq -2,-2");
	run_case(0x3FF0000000000000ULL, 0xBFF0000000000000ULL, "neq +1,-1");
	run_case(0xBFF0000000000000ULL, 0x3FF0000000000000ULL, "neq -1,+1");
	run_case(0x0000000000000000ULL, 0x8000000000000000ULL, "eq +0,-0");
	run_case(0x8000000000000000ULL, 0x0000000000000000ULL, "eq -0,+0");
	run_case(0x0000000000000000ULL, 0x0000000000000000ULL, "eq +0,+0");
	run_case(0x8000000000000000ULL, 0x8000000000000000ULL, "eq -0,-0");
	run_case(0x0000000000000000ULL, 0x8000000000000001ULL, "neq +0,-denorm");
	run_case(0x8000000000000001ULL, 0x0000000000000000ULL, "neq -denorm,+0");
	run_case(0x7FF0000000000000ULL, 0x7FF0000000000000ULL, "eq +inf,+inf");
	run_case(0xFFF0000000000000ULL, 0xFFF0000000000000ULL, "eq -inf,-inf");
	run_case(0x7FF0000000000000ULL, 0xFFF0000000000000ULL, "neq +inf,-inf");
	run_case(0xFFF0000000000000ULL, 0x7FF0000000000000ULL, "neq -inf,+inf");
	run_case(0x7FF0000000000000ULL, 0x7FEFFFFFFFFFFFFFULL, "neq +inf,+max");
	run_case(0x7FEFFFFFFFFFFFFFULL, 0x7FF0000000000000ULL, "neq +max,+inf");
	run_case(0x7FF0000000000001ULL, 0x3FF0000000000000ULL, "neq sNaN,+1");
	run_case(0x3FF0000000000000ULL, 0x7FF0000000000001ULL, "neq +1,sNaN");
	run_case(0x7FF8000000000000ULL, 0x7FF8000000000000ULL, "neq qNaN,qNaN");
	run_case(0xFFF8000000000000ULL, 0x3FF0000000000000ULL, "neq -qNaN,+1");
	run_case(0x7FF0000000000000ULL, 0x7FF8000000000000ULL, "neq +inf,qNaN");
	run_case(0x7FF8000000000000ULL, 0x7FF0000000000000ULL, "neq qNaN,+inf");
	run_case(0x7FE0000000000000ULL, 0x7FE0000000000000ULL, "eq exp7fe,exp7fe");
	run_case(0x7FE0000000000001ULL, 0x7FE0000000000000ULL, "neq exp7fe frac1");
	run_case(0x0000000000000001ULL, 0x0000000000000002ULL, "neq denorm 1,2");
	run_case(0x0000000000000002ULL, 0x0000000000000001ULL, "neq denorm 2,1");
	run_case(0x8000000000000002ULL, 0x8000000000000001ULL, "neq -denorm 2,1");
	run_case(0x7FF4000000000000ULL, 0x7FF8000000000000ULL, "neq sNaN,qNaN");
	run_case(0x7FF8000000000000ULL, 0x7FF4000000000000ULL, "neq qNaN,sNaN");

	/*
	 * 2. Full cross product of the hand-written pool: every pattern
	 *    against every pattern, in both orders.
	 */
	for (int i = 0; i < NPOOL; i++)
		for (int j = 0; j < NPOOL; j++)
			run_case(pool[i], pool[j], "pool");

	/*
	 * 3. Fixed-seed randomised sweep.  The modes deliberately over-sample
	 *    the interesting neighbourhoods: equal operands, operands one bit
	 *    apart, forced exp==0x7ff (NaN and infinity), forced equal and
	 *    forced opposite bit patterns, and zero/denormal magnitudes.
	 */
	rng_state = 0x0010B0010B0010ULL;
	const long ITERS = 260000;
	for (long it = 0; it < ITERS; it++) {
		u64 a = next_rand();
		u64 b = next_rand();
		unsigned mode = (unsigned)(next_rand() % 8u);
		switch (mode) {
		case 0:
			break;			/* wholly random */
		case 1:
			a = pool[next_rand() % NPOOL] ^
			    (next_rand() & 0x3ULL);
			b = pool[next_rand() % NPOOL] ^
			    (next_rand() & 0x3ULL);
			break;
		case 2:
			b = a;			/* a == b */
			break;
		case 3:
			b = a ^ (1ULL << (next_rand() % 64));
			break;
		case 4: {			/* exp == 0x7ff on one or both */
			u64 fa = next_rand() & 0x000FFFFFFFFFFFFFULL;
			u64 fb = next_rand() & 0x000FFFFFFFFFFFFFULL;
			if ((next_rand() & 1) == 0)
				fa = 0;		/* infinity, not NaN */
			if ((next_rand() & 1) == 0)
				fb = 0;
			a = ((next_rand() & 1) << 63) |
			    0x7FF0000000000000ULL | fa;
			if ((next_rand() & 1) == 0)
				b = ((next_rand() & 1) << 63) |
				    0x7FF0000000000000ULL | fb;
			else
				b = pool[next_rand() % NPOOL];
			break;
		}
		case 5: {			/* identical bit patterns */
			u64 t = pool[next_rand() % NPOOL];
			a = t;
			b = t;
			break;
		}
		case 6: {			/* +0 / -0 pair */
			if ((next_rand() & 1) == 0) {
				a = 0x0000000000000000ULL;
				b = 0x8000000000000000ULL;
			} else {
				a = 0x8000000000000000ULL;
				b = 0x0000000000000000ULL;
			}
			break;
		}
		default: {			/* zero / denormal magnitudes */
			a = ((next_rand() & 1) << 63) |
			    (next_rand() & 0x000000000000000FULL);
			b = ((next_rand() & 1) << 63) |
			    (next_rand() & 0x000000000000000FULL);
			break;
		}
		}
		run_case(a, b, "random");
	}

	unsigned long long total_fails = 0;
	printf("\n%-20s %12s %12s\n", "function", "cases", "failures");
	printf("%-20s %12s %12s\n", "--------------------",
	    "------------", "------------");
	for (int i = 0; i < NFN; i++) {
		printf("%-20s %12llu %12llu\n", fns[i].name, fns[i].cases,
		    fns[i].fails);
		total_fails += fns[i].fails;
	}
	printf("\n%s: %llu failure(s)\n",
	    total_fails == 0 ? "PASS" : "FAIL", total_fails);
	return total_fails == 0 ? 0 : 1;
}
