/*
 * harness.cpp -- differential test for PBSD batch b0079s2.
 *
 * Every case is fed to BOTH the C++23 port and the ref_ oracle compiled from
 * the original C, and the results are compared bit for bit.  The batch has no
 * buffer-writing functions, so in addition to comparing return values each
 * case stages its input inside a 0x7f-guarded scratch buffer (one per
 * implementation) and the ENTIRE buffer is compared afterwards, which catches
 * any stray write through the argument as well as any guard corruption.
 */

#include <cstdio>
#include <cstdint>
#include <cstring>

import pbsd.lib.msun.src.b0079s2;

extern "C" int ref_finitef(float x);

namespace port = pbsd::lib_msun_src::b0079s2;

/* ------------------------------------------------------------------ */
/* bookkeeping                                                         */
/* ------------------------------------------------------------------ */

namespace {

struct Stats {
	const char *name;
	unsigned long long cases;
	unsigned long long failures;
};

Stats finitef_stats = { "finitef", 0, 0 };

const std::size_t BUFSZ = 16;
const std::size_t OFF = 6;	/* deliberately unaligned staging offset */
const unsigned char GUARD = 0x7f;

unsigned long long reported = 0;

void
fail_report(const char *what, std::uint32_t bits, long long got,
    long long expected)
{
	if (reported < 25) {
		std::printf("FAIL %s: bits=0x%08lx port=%lld oracle=%lld\n",
		    what, (unsigned long)bits, got, expected);
		reported++;
	} else if (reported == 25) {
		std::printf("FAIL ... further failures suppressed\n");
		reported++;
	}
}

/*
 * Run one finitef case.  The float argument is materialised out of a guarded
 * scratch buffer -- a distinct one for the port and for the oracle -- and both
 * whole buffers are compared once the calls have returned.
 */
void
check_finitef(std::uint32_t bits)
{
	unsigned char buf_port[BUFSZ];
	unsigned char buf_ref[BUFSZ];

	std::memset(buf_port, GUARD, BUFSZ);
	std::memset(buf_ref, GUARD, BUFSZ);
	std::memcpy(buf_port + OFF, &bits, sizeof(bits));
	std::memcpy(buf_ref + OFF, &bits, sizeof(bits));

	float x_port;
	float x_ref;
	std::memcpy(&x_port, buf_port + OFF, sizeof(x_port));
	std::memcpy(&x_ref, buf_ref + OFF, sizeof(x_ref));

	int r_port = port::finitef(x_port);
	int r_ref = ref_finitef(x_ref);

	finitef_stats.cases++;

	bool bad = false;

	if (r_port != r_ref) {
		bad = true;
		fail_report("finitef return", bits, r_port, r_ref);
	}
	if (std::memcmp(buf_port, buf_ref, BUFSZ) != 0) {
		bad = true;
		fail_report("finitef buffer", bits, -1, -1);
	}
	for (std::size_t i = 0; i < BUFSZ; i++) {
		if (i >= OFF && i < OFF + sizeof(bits))
			continue;
		if (buf_port[i] != GUARD || buf_ref[i] != GUARD) {
			bad = true;
			fail_report("finitef guard", bits, buf_port[i],
			    buf_ref[i]);
			break;
		}
	}
	/* The staged input itself must survive the call untouched. */
	std::uint32_t back_port;
	std::uint32_t back_ref;
	std::memcpy(&back_port, buf_port + OFF, sizeof(back_port));
	std::memcpy(&back_ref, buf_ref + OFF, sizeof(back_ref));
	if (back_port != bits || back_ref != bits) {
		bad = true;
		fail_report("finitef input clobbered", bits, back_port,
		    back_ref);
	}

	if (bad)
		finitef_stats.failures++;
}

/* ------------------------------------------------------------------ */
/* hand-written edge cases                                             */
/* ------------------------------------------------------------------ */

const std::uint32_t edge_bits[] = {
	/* zeroes */
	0x00000000u, 0x80000000u,
	/* smallest subnormals */
	0x00000001u, 0x80000001u,
	0x00000002u, 0x80000002u,
	/* largest subnormals / smallest normals: both sides of the step */
	0x007fffffu, 0x807fffffu,
	0x00800000u, 0x80800000u,
	0x00800001u, 0x80800001u,
	/* ordinary values */
	0x3f800000u, 0xbf800000u,	/* +-1.0 */
	0x40490fdbu, 0xc0490fdbu,	/* +-pi */
	0x00000010u, 0x7f000000u,
	0x0000ffffu, 0xffff0000u,
	/* just below the infinity/NaN exponent: still finite */
	0x7effffffu, 0xfeffffffu,
	0x7f7ffffeu, 0xff7ffffeu,
	0x7f7fffffu, 0xff7fffffu,	/* +-FLT_MAX, boundary - 1 */
	/* the exponent boundary itself: infinities */
	0x7f800000u, 0xff800000u,
	/* signalling NaNs, minimum and maximum payload */
	0x7f800001u, 0xff800001u,
	0x7f800002u, 0xff800002u,
	0x7fbffffeu, 0xffbffffeu,
	0x7fbfffffu, 0xffbfffffu,
	/* quiet NaNs, minimum and maximum payload */
	0x7fc00000u, 0xffc00000u,
	0x7fc00001u, 0xffc00001u,
	0x7ffffffeu, 0xfffffffeu,
	0x7fffffffu, 0xffffffffu,
	/* single-bit patterns around bit 30 and bit 31 */
	0x40000000u, 0xc0000000u,
	0x20000000u, 0xa0000000u,
	0x00000000u ^ 0x7fffffffu,
};

void
run_edge_cases(void)
{
	for (std::size_t i = 0; i < sizeof(edge_bits) / sizeof(edge_bits[0]);
	    i++)
		check_finitef(edge_bits[i]);

	/*
	 * Byte-oriented sweep: every combination of four "interesting" bytes,
	 * so that high-bit bytes 0x80-0xff appear in every position of the
	 * argument, together with NUL-heavy and all-ones words.
	 */
	static const unsigned char interesting[] = {
		0x00, 0x01, 0x7e, 0x7f, 0x80, 0x81, 0xfe, 0xff
	};
	const std::size_t nint = sizeof(interesting) / sizeof(interesting[0]);

	for (std::size_t a = 0; a < nint; a++)
		for (std::size_t b = 0; b < nint; b++)
			for (std::size_t c = 0; c < nint; c++)
				for (std::size_t d = 0; d < nint; d++) {
					unsigned char raw[4] = {
						interesting[a], interesting[b],
						interesting[c], interesting[d]
					};
					std::uint32_t bits;
					std::memcpy(&bits, raw, sizeof(bits));
					check_finitef(bits);
				}

	/* Every possible value of every single byte, over a 0x00 and a 0xff
	 * background, in every byte position. */
	for (unsigned shift = 0; shift < 32; shift += 8)
		for (unsigned v = 0; v < 256; v++) {
			std::uint32_t lo = (std::uint32_t)v << shift;
			std::uint32_t hi = 0xffffffffu &
			    ~((std::uint32_t)0xffu << shift);
			check_finitef(lo);
			check_finitef(hi | lo);
		}

	/*
	 * Exhaustive walk of the exponent field with a handful of mantissas
	 * and both signs: this straddles the 0x7f800000 threshold from both
	 * directions for every sign/mantissa combination.
	 */
	static const std::uint32_t mantissas[] = {
		0x000000u, 0x000001u, 0x000002u, 0x400000u,
		0x7ffffeu, 0x7fffffu
	};
	const std::size_t nman = sizeof(mantissas) / sizeof(mantissas[0]);

	for (unsigned sign = 0; sign < 2; sign++)
		for (unsigned exp = 0; exp < 256; exp++)
			for (std::size_t m = 0; m < nman; m++) {
				std::uint32_t bits =
				    ((std::uint32_t)sign << 31) |
				    ((std::uint32_t)exp << 23) |
				    mantissas[m];
				check_finitef(bits);
			}

	/* Single-bit and single-bit-cleared words: 32 + 32 cases. */
	for (unsigned bit = 0; bit < 32; bit++) {
		check_finitef((std::uint32_t)1u << bit);
		check_finitef(~((std::uint32_t)1u << bit));
	}

	/* Immediate neighbourhood of the 0x7f800000 subtrahend, unsigned, so
	 * that any perturbation of that constant changes an answer. */
	for (std::uint32_t delta = 0; delta <= 4; delta++) {
		check_finitef(0x7f800000u - delta);
		check_finitef(0x7f800000u + delta);
		check_finitef(0xff800000u - delta);
		check_finitef(0xff800000u + delta);
	}
}

/* ------------------------------------------------------------------ */
/* fixed-seed randomised sweep                                         */
/* ------------------------------------------------------------------ */

std::uint64_t rng_state;

void
rng_seed(std::uint64_t s)
{
	rng_state = s;
}

std::uint64_t
rng_next(void)
{
	/* splitmix64: self-contained so the sequence is toolchain agnostic. */
	std::uint64_t z = (rng_state += 0x9e3779b97f4a7c15ull);
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ull;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebull;
	return z ^ (z >> 31);
}

std::uint32_t
rng_u32(void)
{
	return (std::uint32_t)(rng_next() >> 32);
}

const unsigned long RANDOM_ITERATIONS = 400000ul;

void
run_random_sweep(void)
{
	rng_seed(0x50425344b0079002ull);	/* "PBSD" b0079s2 */

	for (unsigned long i = 0; i < RANDOM_ITERATIONS; i++) {
		std::uint32_t r = rng_u32();
		std::uint32_t bits;

		switch (i % 5) {
		case 0:
			/* Uniform over the whole 32-bit space. */
			bits = r;
			break;
		case 1:
			/* Random sign and mantissa, exponent pinned to the
			 * two values that straddle the threshold. */
			bits = (r & 0x807fffffu) |
			    ((std::uint32_t)(254u + (r >> 31)) << 23);
			break;
		case 2:
			/* Random sign and mantissa, exponent within +-2 of
			 * the threshold. */
			bits = (r & 0x807fffffu) |
			    ((std::uint32_t)(253u + (rng_u32() % 3)) << 23);
			break;
		case 3:
			/* Subnormals and tiny normals. */
			bits = (r & 0x80ffffffu);
			break;
		case 4:
		default:
			/* Random exponent, extreme mantissas. */
			bits = (r & 0xff800000u) |
			    ((rng_u32() & 1u) ? 0x007fffffu : 0x00000000u);
			break;
		}

		check_finitef(bits);
	}
}

} /* namespace */

/* ------------------------------------------------------------------ */

int
main(void)
{
	run_edge_cases();
	run_random_sweep();

	const Stats *table[] = { &finitef_stats };
	const std::size_t n = sizeof(table) / sizeof(table[0]);

	unsigned long long total_failures = 0;

	std::printf("\n");
	std::printf("%-20s %12s %12s %8s\n", "function", "cases", "failures",
	    "result");
	std::printf("%-20s %12s %12s %8s\n", "--------------------",
	    "------------", "------------", "--------");
	for (std::size_t i = 0; i < n; i++) {
		std::printf("%-20s %12llu %12llu %8s\n", table[i]->name,
		    table[i]->cases, table[i]->failures,
		    table[i]->failures == 0 ? "PASS" : "FAIL");
		total_failures += table[i]->failures;
	}
	std::printf("\n");

	if (total_failures != 0) {
		std::printf("b0079s2: %llu failing case(s)\n", total_failures);
		return 1;
	}

	std::printf("b0079s2: all cases matched the oracle\n");
	return 0;
}
