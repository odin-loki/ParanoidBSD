/*
 * Differential test harness for PBSD batch b0036.
 *
 * Every function of the port is compared against the ref_ oracle built from
 * the unmodified HardenedBSD sources.  All fifteen functions take a single
 * unsigned integer and return unsigned int; none of them writes through a
 * pointer and none returns a pointer.  The input is nevertheless staged inside
 * a 0x7f-filled guard buffer that is compared in full after every call, so a
 * port that scribbles on memory is caught as well.
 */

#include <climits>
#include <cstdint>
#include <cstdio>
#include <cstring>

import pbsd.lib.libc.stdbit.b0036;

extern "C" {
unsigned int ref_stdc_trailing_ones_uc(unsigned char);
unsigned int ref_stdc_trailing_ones_us(unsigned short);
unsigned int ref_stdc_trailing_ones_ui(unsigned int);
unsigned int ref_stdc_trailing_ones_ul(unsigned long);
unsigned int ref_stdc_trailing_ones_ull(unsigned long long);

unsigned int ref_stdc_trailing_zeros_uc(unsigned char);
unsigned int ref_stdc_trailing_zeros_us(unsigned short);
unsigned int ref_stdc_trailing_zeros_ui(unsigned int);
unsigned int ref_stdc_trailing_zeros_ul(unsigned long);
unsigned int ref_stdc_trailing_zeros_ull(unsigned long long);

unsigned int ref_stdc_leading_ones_uc(unsigned char);
unsigned int ref_stdc_leading_ones_us(unsigned short);
unsigned int ref_stdc_leading_ones_ui(unsigned int);
unsigned int ref_stdc_leading_ones_ul(unsigned long);
unsigned int ref_stdc_leading_ones_ull(unsigned long long);
}

namespace P = pbsd::lib_libc_stdbit::b0036;

namespace {

constexpr int MAXFN = 16;
constexpr long RANDOM_ITERATIONS = 250000;

struct Stat {
	const char *name;
	unsigned long long cases;
	unsigned long long fails;
	int reported;
};

Stat stats[MAXFN];
int nstats;

template <class T>
using Fn = unsigned int (*)(T);

uint64_t rng_state;

inline uint64_t
rng_next(void)
{
	uint64_t x = rng_state;

	x ^= x << 13;
	x ^= x >> 7;
	x ^= x << 17;
	rng_state = x;

	return (x);
}

template <class T>
void
check(Stat &s, Fn<T> pf, Fn<T> rf, T x)
{
	constexpr int BUFSZ = 64;
	constexpr int OFF = 16;

	unsigned char abuf[BUFSZ], bbuf[BUFSZ], pristine[BUFSZ];

	memset(abuf, 0x7f, BUFSZ);
	memset(bbuf, 0x7f, BUFSZ);
	memcpy(abuf + OFF, &x, sizeof(T));
	memcpy(bbuf + OFF, &x, sizeof(T));
	memcpy(pristine, abuf, BUFSZ);

	T xa, xb;
	memcpy(&xa, abuf + OFF, sizeof(T));
	memcpy(&xb, bbuf + OFF, sizeof(T));

	unsigned int ra = pf(xa);
	unsigned int rb = rf(xb);

	int buf_a_ok = memcmp(abuf, pristine, BUFSZ) == 0;
	int buf_b_ok = memcmp(bbuf, pristine, BUFSZ) == 0;

	s.cases++;
	if (ra != rb || !buf_a_ok || !buf_b_ok) {
		s.fails++;
		if (s.reported < 8) {
			s.reported++;
			printf("  FAIL %-28s x=0x%0*llx  port=%u  ref=%u%s%s\n",
			    s.name, (int)(2 * sizeof(T)),
			    (unsigned long long)x, ra, rb,
			    buf_a_ok ? "" : "  [port buffer clobbered]",
			    buf_b_ok ? "" : "  [ref buffer clobbered]");
		}
	}
}

/*
 * Exercise one function over: the complete domain when it is narrow enough,
 * a hand-written edge set covering both sides of every boundary the code
 * tests, per-bit-position masks so that every shift distance and width
 * constant is observable, and a fixed-seed randomised sweep.
 */
template <class T>
void
run(const char *name, Fn<T> pf, Fn<T> rf, uint64_t seed)
{
	Stat &s = stats[nstats++];

	s.name = name;
	s.cases = 0;
	s.fails = 0;
	s.reported = 0;

	constexpr int W = (int)(sizeof(T) * CHAR_BIT);
	const T max = (T)~(T)0;

	/* Exhaustive over the whole domain for the 8 and 16 bit variants. */
	if (W <= 16)
		for (unsigned long v = 0; v <= (unsigned long)max; v++)
			check<T>(s, pf, rf, (T)v);

	/* Hand-written edge cases, both sides of 0 and of ~0. */
	const T edges[] = {
		(T)0, (T)1, (T)2, (T)3,
		max, (T)(max - 1), (T)(max - 2), (T)(max ^ (T)1),
		(T)(max >> 1), (T)~(T)(max >> 1),
		(T)(max - 3), (T)(max ^ (T)3),
	};
	for (T v : edges)
		check<T>(s, pf, rf, v);

	/* Every single bit, every run of low ones, every run of high ones. */
	for (int k = 0; k < W; k++) {
		const T bit = (T)((T)1 << k);
		const T low = (T)(bit - (T)1);

		check<T>(s, pf, rf, bit);
		check<T>(s, pf, rf, (T)~bit);
		check<T>(s, pf, rf, low);
		check<T>(s, pf, rf, (T)~low);
		check<T>(s, pf, rf, (T)(max ^ bit));
		check<T>(s, pf, rf, (T)(low | bit));
		check<T>(s, pf, rf, (T)(bit | (T)1));
		check<T>(s, pf, rf, (T)(max << k));
		check<T>(s, pf, rf, (T)(max >> k));
		check<T>(s, pf, rf, (T)~(T)(max >> k));
		check<T>(s, pf, rf, (T)~(T)(max << k));
	}

	/* Alternating and nibble patterns. */
	T alt = 0;
	for (int i = 0; i < W; i += 2)
		alt = (T)(alt | (T)((T)1 << i));
	check<T>(s, pf, rf, alt);
	check<T>(s, pf, rf, (T)~alt);

	T nib = 0;
	for (int i = 0; i < W; i += 4)
		nib = (T)(nib | (T)((T)3 << i));
	check<T>(s, pf, rf, nib);
	check<T>(s, pf, rf, (T)~nib);

	/* Fixed-seed randomised sweep. */
	rng_state = seed;
	for (long i = 0; i < RANDOM_ITERATIONS; i++) {
		uint64_t r = rng_next();
		uint64_t v;

		switch (i % 8) {
		case 0:
			v = r;
			break;
		case 1:	/* sparse: few set bits */
			v = r & rng_next();
			break;
		case 2:	/* very sparse */
			v = r & rng_next() & rng_next();
			break;
		case 3:	/* dense: few clear bits */
			v = r | rng_next();
			break;
		case 4:	/* very dense, close to ~0 */
			v = r | rng_next() | rng_next();
			break;
		case 5:	/* exactly one clear bit */
			v = ~((uint64_t)1 << (rng_next() & 63));
			break;
		case 6:	/* a run of low ones */
			v = ((uint64_t)1 << (rng_next() & 63)) - 1;
			break;
		default: /* a run of high ones */
			v = ~(uint64_t)0 << (rng_next() & 63);
			break;
		}

		check<T>(s, pf, rf, (T)v);
	}
}

} /* namespace */

int
main(void)
{
	run<unsigned char>("stdc_trailing_ones_uc",
	    &P::stdc_trailing_ones_uc, &ref_stdc_trailing_ones_uc,
	    0x9e3779b97f4a7c15ULL);
	run<unsigned short>("stdc_trailing_ones_us",
	    &P::stdc_trailing_ones_us, &ref_stdc_trailing_ones_us,
	    0x9e3779b97f4a7c16ULL);
	run<unsigned int>("stdc_trailing_ones_ui",
	    &P::stdc_trailing_ones_ui, &ref_stdc_trailing_ones_ui,
	    0x9e3779b97f4a7c17ULL);
	run<unsigned long>("stdc_trailing_ones_ul",
	    &P::stdc_trailing_ones_ul, &ref_stdc_trailing_ones_ul,
	    0x9e3779b97f4a7c18ULL);
	run<unsigned long long>("stdc_trailing_ones_ull",
	    &P::stdc_trailing_ones_ull, &ref_stdc_trailing_ones_ull,
	    0x9e3779b97f4a7c19ULL);

	run<unsigned char>("stdc_trailing_zeros_uc",
	    &P::stdc_trailing_zeros_uc, &ref_stdc_trailing_zeros_uc,
	    0xd1b54a32d192ed03ULL);
	run<unsigned short>("stdc_trailing_zeros_us",
	    &P::stdc_trailing_zeros_us, &ref_stdc_trailing_zeros_us,
	    0xd1b54a32d192ed04ULL);
	run<unsigned int>("stdc_trailing_zeros_ui",
	    &P::stdc_trailing_zeros_ui, &ref_stdc_trailing_zeros_ui,
	    0xd1b54a32d192ed05ULL);
	run<unsigned long>("stdc_trailing_zeros_ul",
	    &P::stdc_trailing_zeros_ul, &ref_stdc_trailing_zeros_ul,
	    0xd1b54a32d192ed06ULL);
	run<unsigned long long>("stdc_trailing_zeros_ull",
	    &P::stdc_trailing_zeros_ull, &ref_stdc_trailing_zeros_ull,
	    0xd1b54a32d192ed07ULL);

	run<unsigned char>("stdc_leading_ones_uc",
	    &P::stdc_leading_ones_uc, &ref_stdc_leading_ones_uc,
	    0x2545f4914f6cdd1dULL);
	run<unsigned short>("stdc_leading_ones_us",
	    &P::stdc_leading_ones_us, &ref_stdc_leading_ones_us,
	    0x2545f4914f6cdd1eULL);
	run<unsigned int>("stdc_leading_ones_ui",
	    &P::stdc_leading_ones_ui, &ref_stdc_leading_ones_ui,
	    0x2545f4914f6cdd1fULL);
	run<unsigned long>("stdc_leading_ones_ul",
	    &P::stdc_leading_ones_ul, &ref_stdc_leading_ones_ul,
	    0x2545f4914f6cdd20ULL);
	run<unsigned long long>("stdc_leading_ones_ull",
	    &P::stdc_leading_ones_ull, &ref_stdc_leading_ones_ull,
	    0x2545f4914f6cdd21ULL);

	unsigned long long total_cases = 0, total_fails = 0;

	printf("\n%-28s %12s %12s\n", "function", "cases", "failures");
	printf("---------------------------------------------------------\n");
	for (int i = 0; i < nstats; i++) {
		printf("%-28s %12llu %12llu\n", stats[i].name,
		    stats[i].cases, stats[i].fails);
		total_cases += stats[i].cases;
		total_fails += stats[i].fails;
	}
	printf("---------------------------------------------------------\n");
	printf("%-28s %12llu %12llu\n", "TOTAL", total_cases, total_fails);
	printf("\n%s\n", total_fails == 0 ? "PASS" : "FAIL");

	return (total_fails == 0 ? 0 : 1);
}
