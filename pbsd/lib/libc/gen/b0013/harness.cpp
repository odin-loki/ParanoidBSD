/*
 * Differential test harness for batch b0013:
 *	lib/libc/gen/_rand48.c   (_rand48_seed, _rand48_mult, _rand48_add)
 *	lib/libc/gen/nrand48.c
 *	lib/libc/gen/erand48.c
 *	lib/libc/gen/jrand48.c
 *
 * Every case is run against both the C++23 port and the ref_ oracle.  The
 * three functions each mutate a three-element unsigned short array in place,
 * so each case uses two independent 32-byte buffers pre-filled with the guard
 * byte 0x7f, with the seed placed 8 bytes in, and the WHOLE buffer is compared
 * afterwards -- guard bytes both before and after the nominal write window.
 * None of these functions returns a pointer, so there are no offsets to
 * compare; nrand48/jrand48 return long (compared exactly) and erand48 returns
 * double (compared as raw bits, so -0.0 and NaN payloads would show up).
 *
 * The `_rand48 (globals)' rows compare the port's three global state words
 * against the oracle's directly, because _rand48_seed is not reachable through
 * any function in this batch.
 *
 * The `IEEEd2bits layout' rows likewise probe the port's copy of the fpmath.h
 * union field by field.  erand48 only ever stores exponents in [975, 1022] and
 * never names manl or sign, so a wrong width on those fields is invisible from
 * erand48's results alone; the probe compares the raw 64 bits produced by
 * storing into each field against the definition in lib/libc/include/fpmath.h.
 */

#include <limits.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

import pbsd.lib.libc.gen.b0013;

namespace P = pbsd::lib_libc_gen::b0013;

extern "C" {
extern uint64_t ref__rand48_seed;
extern uint64_t ref__rand48_mult;
extern uint64_t ref__rand48_add;
long ref_nrand48(unsigned short xseed[3]);
double ref_erand48(unsigned short xseed[3]);
long ref_jrand48(unsigned short xseed[3]);
}

#define	MASK48	0xffffffffffffULL

enum { FN_N = 0, FN_E = 1, FN_J = 2, FN_S = 3, FN_L = 4, NFN = 5 };

/* lib/libc/include/fpmath.h + lib/libc/{amd64,aarch64}/_fpmath.h: the
 * specification the port's union has to reproduce exactly. */
union IEEEd2bits_spec {
	double	d;
	struct {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		unsigned int	manl	:32;
		unsigned int	manh	:20;
		unsigned int	exp	:11;
		unsigned int	sign	:1;
#else /* _BIG_ENDIAN */
		unsigned int	sign	:1;
		unsigned int	exp	:11;
		unsigned int	manh	:20;
		unsigned int	manl	:32;
#endif
	} bits;
};

struct Stat {
	const char *name;
	unsigned long long cases;
	unsigned long long fails;
	unsigned long long reported;
};

static Stat stats[NFN] = {
	{ "nrand48", 0, 0, 0 },
	{ "erand48", 0, 0, 0 },
	{ "jrand48", 0, 0, 0 },
	{ "_rand48 (globals)", 0, 0, 0 },
	{ "IEEEd2bits layout", 0, 0, 0 },
};

enum { BUFSZ = 32, PRE = 8 };

struct Buf {
	alignas(16) unsigned char b[BUFSZ];
};

static void
prep(Buf &x, const unsigned short in[3])
{
	memset(x.b, 0x7f, BUFSZ);
	memcpy(x.b + PRE, in, 3 * sizeof(unsigned short));
}

static unsigned short *
sp(Buf &x)
{
	return reinterpret_cast<unsigned short *>(x.b + PRE);
}

static uint64_t
dbits(double d)
{
	uint64_t u;

	memcpy(&u, &d, sizeof u);
	return u;
}

static void
dump(const char *tag, const Buf &x)
{
	printf("        %s", tag);
	for (int i = 0; i < BUFSZ; i++)
		printf(" %02x", x.b[i]);
	printf("\n");
}

/*
 * Run one call of one function on both sides.  Returns true if the return
 * value or any byte of the buffer differs.
 */
static bool
step(int fn, Buf &pa, Buf &pb, char *rv, size_t rvsz)
{
	bool bad = false;

	switch (fn) {
	case FN_N: {
		long a = P::nrand48(sp(pa));
		long b = ref_nrand48(sp(pb));
		if (a != b)
			bad = true;
		snprintf(rv, rvsz, "ret port=%ld ref=%ld", a, b);
		break;
	}
	case FN_J: {
		long a = P::jrand48(sp(pa));
		long b = ref_jrand48(sp(pb));
		if (a != b)
			bad = true;
		snprintf(rv, rvsz, "ret port=%ld ref=%ld", a, b);
		break;
	}
	default: {
		double a = P::erand48(sp(pa));
		double b = ref_erand48(sp(pb));
		if (dbits(a) != dbits(b))
			bad = true;
		snprintf(rv, rvsz, "ret port=%.17g [%016llx] ref=%.17g [%016llx]",
		    a, (unsigned long long)dbits(a),
		    b, (unsigned long long)dbits(b));
		break;
	}
	}
	if (memcmp(pa.b, pb.b, BUFSZ) != 0)
		bad = true;
	return bad;
}

static void
account(int fn, bool bad, const unsigned short in[3], const char *tag, long iter,
    Buf &pa, Buf &pb, const char *rv)
{
	Stat &st = stats[fn];

	st.cases++;
	if (!bad)
		return;
	st.fails++;
	if (st.reported >= 10)
		return;
	st.reported++;
	printf("  FAIL %-8s tag=%-16s iter=%-4ld seed={%04x,%04x,%04x} "
	    "mult=%012llx add=%012llx\n        %s\n",
	    st.name, tag, iter, in[0], in[1], in[2],
	    (unsigned long long)ref__rand48_mult,
	    (unsigned long long)ref__rand48_add, rv);
	dump("port:", pa);
	dump("ref :", pb);
	fflush(stdout);
}

static void
single(int fn, const unsigned short in[3], const char *tag)
{
	Buf pa, pb;
	char rv[192];

	prep(pa, in);
	prep(pb, in);
	account(fn, step(fn, pa, pb, rv, sizeof rv), in, tag, 0, pa, pb, rv);
}

static void
single_all(const unsigned short in[3], const char *tag)
{
	single(FN_N, in, tag);
	single(FN_E, in, tag);
	single(FN_J, in, tag);
}

/*
 * These generators are stateful: the seed array is both input and output, so
 * drive each one for many successive calls from the same start state and
 * compare the return value and the entire buffer after EVERY iteration.
 */
static void
seq_all(const unsigned short in[3], int iters, const char *tag)
{
	for (int fn = 0; fn < 3; fn++) {
		Buf pa, pb;

		prep(pa, in);
		prep(pb, in);
		for (int i = 0; i < iters; i++) {
			char rv[192];
			bool bad = step(fn, pa, pb, rv, sizeof rv);
			account(fn, bad, in, tag, i, pa, pb, rv);
		}
	}
}

static void
check_state(const char *tag)
{
	Stat &st = stats[FN_S];
	struct {
		const char *n;
		unsigned long long p, r;
	} v[3] = {
		{ "_rand48_seed", (unsigned long long)P::_rand48_seed,
		    (unsigned long long)ref__rand48_seed },
		{ "_rand48_mult", (unsigned long long)P::_rand48_mult,
		    (unsigned long long)ref__rand48_mult },
		{ "_rand48_add", (unsigned long long)P::_rand48_add,
		    (unsigned long long)ref__rand48_add },
	};

	for (int i = 0; i < 3; i++) {
		st.cases++;
		if (v[i].p == v[i].r)
			continue;
		st.fails++;
		if (st.reported >= 10)
			continue;
		st.reported++;
		printf("  FAIL %s tag=%s %s port=%012llx ref=%012llx\n",
		    st.name, tag, v[i].n, v[i].p, v[i].r);
		fflush(stdout);
	}
}

static void
check_layout(void)
{
	static const char *const fname[4] = { "manl", "manh", "exp", "sign" };
	static const unsigned int pat[] = {
		0u, 1u, 2u, 0x3ffu, 0x7feu, 0x7ffu, 0x800u, 0xfffffu, 0x100000u,
		0x7fffffffu, 0x80000000u, 0xffffffffu, 0x55555555u, 0xaaaaaaaau,
		975u, 1022u, 1023u, 2047u,
	};
	union pbits {
		P::IEEEd2bits ieee;
		uint64_t u64;
	} p;
	union rbits {
		union IEEEd2bits_spec ieee;
		uint64_t u64;
	} r;
	Stat &st = stats[FN_L];

	st.cases++;
	if (sizeof(P::IEEEd2bits) != sizeof(union IEEEd2bits_spec)) {
		st.fails++;
		st.reported++;
		printf("  FAIL %s sizeof port=%zu ref=%zu\n", st.name,
		    sizeof(P::IEEEd2bits), sizeof(union IEEEd2bits_spec));
		fflush(stdout);
	}

	for (size_t i = 0; i < sizeof pat / sizeof pat[0]; i++)
		for (int f = 0; f < 4; f++) {
			p.u64 = 0;
			r.u64 = 0;
			switch (f) {
			case 0:
				p.ieee.bits.manl = pat[i];
				r.ieee.bits.manl = pat[i];
				break;
			case 1:
				p.ieee.bits.manh = pat[i];
				r.ieee.bits.manh = pat[i];
				break;
			case 2:
				p.ieee.bits.exp = pat[i];
				r.ieee.bits.exp = pat[i];
				break;
			default:
				p.ieee.bits.sign = pat[i];
				r.ieee.bits.sign = pat[i];
				break;
			}
			st.cases++;
			if (p.u64 == r.u64)
				continue;
			st.fails++;
			if (st.reported >= 10)
				continue;
			st.reported++;
			printf("  FAIL %s bits.%s = %#x -> port=%016llx "
			    "ref=%016llx\n", st.name, fname[f], pat[i],
			    (unsigned long long)p.u64,
			    (unsigned long long)r.u64);
			fflush(stdout);
		}

	/* And the whole-double round trip both ways. */
	for (size_t i = 0; i < sizeof pat / sizeof pat[0]; i++) {
		uint64_t bits = ((uint64_t)pat[i] << 32) | pat[i];

		p.u64 = bits;
		r.u64 = bits;
		st.cases++;
		if (dbits(p.ieee.d) == dbits(r.ieee.d) &&
		    p.ieee.bits.manl == r.ieee.bits.manl &&
		    p.ieee.bits.manh == r.ieee.bits.manh &&
		    p.ieee.bits.exp == r.ieee.bits.exp &&
		    p.ieee.bits.sign == r.ieee.bits.sign)
			continue;
		st.fails++;
		if (st.reported >= 10)
			continue;
		st.reported++;
		printf("  FAIL %s decode %016llx port={%u,%u,%u,%u} "
		    "ref={%u,%u,%u,%u}\n", st.name, (unsigned long long)bits,
		    p.ieee.bits.manl, p.ieee.bits.manh, p.ieee.bits.exp,
		    p.ieee.bits.sign, r.ieee.bits.manl, r.ieee.bits.manh,
		    r.ieee.bits.exp, r.ieee.bits.sign);
		fflush(stdout);
	}
}

/* splitmix64, fixed seed. */
static uint64_t rng_state = 0x0123456789abcdefULL;

static uint64_t
rnd(void)
{
	uint64_t z;

	rng_state += 0x9e3779b97f4a7c15ULL;
	z = rng_state;
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return (z ^ (z >> 31));
}

/*
 * NUL-heavy, all-bits-set, high-bit-only, single-bit and the library's own
 * constants: the interesting 16-bit halfwords.
 */
static const unsigned short pool[] = {
	0x0000, 0x0001, 0x0002, 0x0003, 0x000b, 0x0080, 0x00ff, 0x0100,
	0x1234, 0x330e, 0x5555, 0x7f7f, 0x7ffe, 0x7fff, 0x8000, 0x8001,
	0x8080, 0xaaaa, 0xabcd, 0xdeec, 0xe66d, 0xff00, 0xff80, 0xfffe,
	0xffff,
};
static const int NP = (int)(sizeof pool / sizeof pool[0]);

static void
set3(unsigned short out[3], uint64_t x)
{
	out[0] = (unsigned short)x;
	out[1] = (unsigned short)(x >> 16);
	out[2] = (unsigned short)(x >> 32);
}

/* Inverse of an odd m modulo 2^48 (Newton's iteration doubles the number of
 * correct bits each round). */
static uint64_t
inv48(uint64_t m)
{
	uint64_t inv = 1;

	for (int i = 0; i < 6; i++)
		inv = inv * (2 - m * inv);
	return (inv & MASK48);
}

/*
 * Build the seed whose single step lands on exactly `v' (mod 2^48).  This is
 * what makes it possible to aim at erand48's zero early-out, at every possible
 * number of normalisation shifts, and at both sides of jrand48's sign bit.
 * Requires an odd multiplier, which the default 0x5deece66d is.
 */
static void
seed_for_post(uint64_t v, unsigned short out[3])
{
	uint64_t x;

	x = (((v - ref__rand48_add) & MASK48) * inv48(ref__rand48_mult)) & MASK48;
	set3(out, x);
}

static void
rand_seed(unsigned short out[3])
{
	uint64_t r = rnd();

	for (int c = 0; c < 3; c++) {
		uint64_t sel = rnd();

		if ((sel & 3) == 0)
			out[c] = pool[(sel >> 8) % (uint64_t)NP];
		else
			out[c] = (unsigned short)(r >> (16 * c));
	}
}

extern "C" void
on_alarm(int sig)
{
	static const char msg[] = "\nTIMEOUT: the harness did not finish; the "
	    "port is looping forever.  Treating as failure.\n";
	ssize_t n;

	(void)sig;
	n = write(2, msg, sizeof msg - 1);
	(void)n;
	_exit(1);
}

int
main(void)
{
	unsigned short in[3];
	uint64_t pseed0, pmult0, padd0;
	uint64_t rseed0, rmult0, radd0;

	signal(SIGALRM, on_alarm);
	alarm(300);

	pseed0 = P::_rand48_seed;
	pmult0 = P::_rand48_mult;
	padd0 = P::_rand48_add;
	rseed0 = ref__rand48_seed;
	rmult0 = ref__rand48_mult;
	radd0 = ref__rand48_add;

	printf("b0013 differential test: nrand48, erand48, jrand48, _rand48\n");
	fflush(stdout);

	/* 1. The initial global state, which no function in this batch reads
	 *    back out (_rand48_seed in particular), and the bit layout of the
	 *    union erand48 punnes its result through. */
	check_state("initial");
	check_layout();

	/* 2. Hand-written edge cases. */
	{
		static const unsigned short edge[][3] = {
			{ 0x0000, 0x0000, 0x0000 },	/* all-NUL */
			{ 0x0001, 0x0000, 0x0000 },
			{ 0x0000, 0x0001, 0x0000 },
			{ 0x0000, 0x0000, 0x0001 },
			{ 0xffff, 0x0000, 0x0000 },
			{ 0x0000, 0xffff, 0x0000 },
			{ 0x0000, 0x0000, 0xffff },
			{ 0xffff, 0xffff, 0xffff },	/* all-bits */
			{ 0x8000, 0x0000, 0x0000 },
			{ 0x0000, 0x8000, 0x0000 },
			{ 0x0000, 0x0000, 0x8000 },
			{ 0x8000, 0x8000, 0x8000 },
			{ 0x7fff, 0x7fff, 0x7fff },
			{ 0x7fff, 0x8000, 0x7fff },
			{ 0x8000, 0x7fff, 0x8000 },
			{ 0x0080, 0x0080, 0x0080 },	/* high bit of each byte */
			{ 0x8080, 0x8080, 0x8080 },
			{ 0xff80, 0x80ff, 0x8080 },
			{ 0x00ff, 0xff00, 0x00ff },
			{ 0x330e, 0xabcd, 0x1234 },	/* the default seed */
			{ 0xe66d, 0xdeec, 0x0005 },	/* the multiplier */
			{ 0x000b, 0x0000, 0x0000 },	/* the addend */
			{ 0xfffe, 0xfffe, 0xfffe },
			{ 0x0001, 0x0001, 0x0001 },
			{ 0xaaaa, 0x5555, 0xaaaa },
			{ 0x5555, 0xaaaa, 0x5555 },
		};

		for (size_t i = 0; i < sizeof edge / sizeof edge[0]; i++)
			single_all(edge[i], "edge");
	}

	/* 3. Every combination of the interesting halfwords in every position:
	 *    this is what separates the three TOUINT48 terms from each other
	 *    and the three STORERAND48 stores from each other. */
	for (int i = 0; i < NP; i++)
		for (int j = 0; j < NP; j++)
			for (int k = 0; k < NP; k++) {
				in[0] = pool[i];
				in[1] = pool[j];
				in[2] = pool[k];
				single_all(in, "pool");
			}

	/* 4. Aimed post-step values.  1<<k for every k drives erand48's
	 *    normalisation loop through every shift count 0..47, and straddles
	 *    both the >>16 of jrand48 and the >>17 of nrand48; bit 47 is
	 *    jrand48's sign bit, so both sides of the int truncation are hit. */
	for (int k = 0; k < 48; k++) {
		uint64_t bit = 1ULL << k;

		seed_for_post(bit, in);
		single_all(in, "post=1<<k");
		if (k > 0) {
			/* k == 0 would be post == 0, which section 10 owns. */
			seed_for_post(bit - 1, in);
			single_all(in, "post=(1<<k)-1");
		}
		seed_for_post(bit | 1, in);
		single_all(in, "post=(1<<k)|1");
		seed_for_post((bit - 1) ^ MASK48, in);
		single_all(in, "post=~((1<<k)-1)");
		seed_for_post(MASK48 ^ bit, in);
		single_all(in, "post=~(1<<k)");
	}
	{
		static const uint64_t aimed[] = {
			MASK48, MASK48 >> 1, 1ULL << 47, (1ULL << 47) | 1,
			(1ULL << 47) - 1, 0xffff, 0x10000, 0x1ffff, 0x20000,
			0x1fffe, 0xffffULL << 32, 0x800000000001ULL,
			0x7fffffffffffULL, 0x555555555555ULL,
			0xaaaaaaaaaaaaULL, 0x0000ffff0000ULL,
			0xffff0000ffffULL, 1, 2, 3,
		};

		for (size_t i = 0; i < sizeof aimed / sizeof aimed[0]; i++) {
			seed_for_post(aimed[i], in);
			single_all(in, "post=aimed");
		}
	}

	/* 5. Post-step values with exactly k significant bits, k = 1..48. */
	for (int k = 1; k <= 48; k++)
		for (int r = 0; r < 150; r++) {
			uint64_t v = (rnd() & ((1ULL << (k - 1)) - 1)) |
			    (1ULL << (k - 1));

			seed_for_post(v & MASK48, in);
			single_all(in, "post=kbits");
		}

	/* 6. tmp is NOT masked to 48 bits before nrand48 and jrand48 shift it,
	 *    so their 0x7fffffff / 0xffffffff masks are load-bearing exactly
	 *    when bit 48 of tmp is set.  Cover both states of that bit. */
	for (int want = 1; want >= 0; want--)
		for (int r = 0; r < 300; r++) {
			uint64_t x, tmp;

			do {
				x = rnd() & MASK48;
				tmp = x * ref__rand48_mult + ref__rand48_add;
			} while ((int)((tmp >> 48) & 1) != want);
			set3(in, x);
			single_all(in, want ? "tmp[48]=1" : "tmp[48]=0");
		}
	{
		/* ... and seeds small enough that tmp has no bits above 47 at
		 * all, where the masks are no-ops. */
		uint64_t bound = (MASK48 - ref__rand48_add) / ref__rand48_mult;

		for (int r = 0; r < 400; r++) {
			set3(in, bound ? rnd() % (bound + 1) : 0);
			single_all(in, "tmp<2^48");
		}
	}

	/* 7. Stateful sequences: compare after every single step. */
	{
		static const unsigned short starts[][3] = {
			{ 0x330e, 0xabcd, 0x1234 },
			{ 0x0000, 0x0000, 0x0000 },
			{ 0xffff, 0xffff, 0xffff },
			{ 0x8000, 0x0000, 0x8000 },
			{ 0x0001, 0x0002, 0x0003 },
		};

		for (size_t i = 0; i < sizeof starts / sizeof starts[0]; i++)
			seq_all(starts[i], 4000, "sequence");
	}

	/* 8. Fixed-seed randomised sweep. */
	for (long i = 0; i < 250000; i++) {
		rand_seed(in);
		single_all(in, "random");
	}

	/* 9. _rand48_mult and _rand48_add are mutable globals (srand48 and
	 *    lcong48 write them), so sweep the arithmetic over other values
	 *    too.  Both sides are always set to the same value. */
	{
		static const struct {
			uint64_t mult, add;
		} cfg[] = {
			{ 0x5deece66dULL, 0x000b },
			{ 0x5deece66dULL, 0x0000 },
			{ 0x5deece66dULL, 0x0001 },
			{ 1, 0 },
			{ 1, 1 },
			{ 2, 3 },
			{ 3, MASK48 },
			{ MASK48, MASK48 },
			{ MASK48, 1 },
			{ 0x800000000000ULL, 0x800000000000ULL },
			{ 0x0000ffffULL, 0x00010000ULL },
			{ 0x123456789abULL, 0xcdefULL },
			{ 0xdeadbeefcafeULL, 0x1234567890abULL },
			{ 0xfffffffffffeULL, 0x000000000002ULL },
			{ 0, 0x000b },
			{ 0, 1 },
			{ 0, 0 },	/* every step lands on zero */
		};

		for (size_t c = 0; c < sizeof cfg / sizeof cfg[0]; c++) {
			P::_rand48_mult = cfg[c].mult;
			ref__rand48_mult = cfg[c].mult;
			P::_rand48_add = cfg[c].add;
			ref__rand48_add = cfg[c].add;

			for (int r = 0; r < 4000; r++) {
				rand_seed(in);
				single_all(in, "cfg");
			}
			for (int i = 0; i < NP; i++) {
				in[0] = pool[i];
				in[1] = pool[NP - 1 - i];
				in[2] = pool[(i * 7) % NP];
				seq_all(in, 40, "cfg-seq");
			}
		}

		P::_rand48_mult = pmult0;
		P::_rand48_add = padd0;
		ref__rand48_mult = rmult0;
		ref__rand48_add = radd0;
	}

	/* 10. erand48's zero early-out.  Kept last: a port that inverts that
	 *     test never terminates here, and every other divergence has
	 *     already been recorded and printed by now. */
	seed_for_post(0, in);
	single_all(in, "post=0");
	seq_all(in, 8, "post=0-seq");

	/* 11. Globals again, in case anything trampled them. */
	P::_rand48_seed = pseed0;
	ref__rand48_seed = rseed0;
	check_state("final");

	{
		unsigned long long tc = 0, tf = 0;

		printf("\n%-20s %14s %12s %s\n", "function", "cases",
		    "failures", "result");
		printf("--------------------------------------------------"
		    "-----------\n");
		for (int i = 0; i < NFN; i++) {
			printf("%-20s %14llu %12llu %s\n", stats[i].name,
			    stats[i].cases, stats[i].fails,
			    stats[i].fails ? "FAIL" : "ok");
			tc += stats[i].cases;
			tf += stats[i].fails;
		}
		printf("--------------------------------------------------"
		    "-----------\n");
		printf("%-20s %14llu %12llu %s\n", "TOTAL", tc, tf,
		    tf ? "FAIL" : "ok");
		fflush(stdout);
		return (tf == 0 ? 0 : 1);
	}
}
