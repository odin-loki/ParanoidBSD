/*
 * Batch b0129 differential test: pbsd::lib_libc_net::b0129 versus the ref_
 * oracle built from the original C.
 *
 * Usage: sh build.sh
 */

#include <climits>
#include <cstdint>
#include <cstdio>
#include <cstring>

import pbsd.lib.libc.net.b0129;

extern "C" const char *ref_gai_strerror(int ecode);

namespace port = pbsd::lib_libc_net::b0129;

namespace {

struct stats {
	const char *name;
	unsigned long long cases;
	unsigned long long failures;
};

stats gai_stats = { "gai_strerror", 0, 0 };

unsigned long long reported = 0;
const unsigned long long report_limit = 25;

void
show(const char *label, const char *s)
{
	if (s == nullptr)
		std::printf("%s=NULL", label);
	else
		std::printf("%s=\"%s\"", label, s);
}

/*
 * gai_strerror returns a pointer to a string that neither side owns, and the
 * two sides deliberately hold distinct copies of the literals, so the
 * observable result is the byte content and its length, never the address.
 */
void
check_gai(int ecode, const char *origin)
{
	gai_stats.cases++;

	const char *got = port::gai_strerror(ecode);
	const char *want = ref_gai_strerror(ecode);

	bool ok;
	if (got == nullptr || want == nullptr)
		ok = (got == nullptr && want == nullptr);
	else
		ok = std::strlen(got) == std::strlen(want) &&
		    std::strcmp(got, want) == 0;

	if (ok)
		return;

	gai_stats.failures++;
	if (reported < report_limit) {
		reported++;
		std::printf("FAIL gai_strerror [%s] ecode=%d ", origin, ecode);
		show("port", got);
		std::printf(" ");
		show("oracle", want);
		std::printf("\n");
	}
}

/* splitmix64, so the sweep is identical on every host and every run. */
std::uint64_t rng_state = 0x243F6A8885A308D3ULL;

std::uint64_t
next_u64()
{
	std::uint64_t z = (rng_state += 0x9E3779B97F4A7C15ULL);

	z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
	z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
	return z ^ (z >> 31);
}

/*
 * Hand-written cases.  The interesting structure in gai_strerror is the pair
 * of comparisons "ecode >= 0" and "ecode < EAI_MAX", so both sides of both
 * boundaries are pinned here with literal constants of this file's own -- a
 * bound taken from the port would move along with a mutated port.
 */
void
edge_cases()
{
	static const int fixed[] = {
		INT_MIN, INT_MIN + 1, INT_MIN + 2,
		-1000000, -65536, -256, -255, -129, -128, -127,
		-16, -15, -14, -3, -2, -1,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
		15, 16, 17, 18, 29, 30, 31,
		0x7e, 0x7f, 0x80, 0x81, 0xfe, 0xff, 0x100, 0x101,
		0x7fff, 0x8000, 0xffff, 0x10000,
		1000000, INT_MAX - 2, INT_MAX - 1, INT_MAX,
	};

	for (unsigned i = 0; i < sizeof(fixed) / sizeof(fixed[0]); i++)
		check_gai(fixed[i], "fixed");

	/* Every code in the table, plus one either side of the table. */
	for (int ecode = -1; ecode <= 15; ecode++)
		check_gai(ecode, "table");

	/* Dense range straddling both boundaries. */
	for (int ecode = -300; ecode <= 300; ecode++)
		check_gai(ecode, "dense");

	/*
	 * Every byte value as it arrives from a plain char (sign extended) and
	 * from an unsigned char, which is where the high-bit bytes 0x80-0xff
	 * turn into negative error codes.
	 */
	for (int b = 0; b < 256; b++) {
		check_gai(static_cast<int>(static_cast<signed char>(b)),
		    "signed-char");
		check_gai(static_cast<int>(static_cast<unsigned char>(b)),
		    "unsigned-char");
	}

	/* Powers of two and their neighbours, both signs. */
	for (int bit = 0; bit < 31; bit++) {
		int v = 1 << bit;

		check_gai(v - 1, "pow2");
		check_gai(v, "pow2");
		check_gai(v + 1, "pow2");
		check_gai(-v - 1, "pow2");
		check_gai(-v, "pow2");
		check_gai(-v + 1, "pow2");
	}
}

void
random_sweep()
{
	const unsigned long long iterations = 250000;

	for (unsigned long long i = 0; i < iterations; i++) {
		std::uint64_t r = next_u64();
		int bucket = static_cast<int>(r % 10);
		std::uint64_t v = next_u64();
		int ecode;

		switch (bucket) {
		case 0:
		case 1:
		case 2:
		case 3:
			/* [-5, 35]: both boundaries hit constantly. */
			ecode = static_cast<int>(v % 41) - 5;
			break;
		case 4:
		case 5:
			/* [11, 19]: tight cluster around the table end. */
			ecode = 11 + static_cast<int>(v % 9);
			break;
		case 6:
		case 7:
			/* The whole of int, both signs. */
			ecode = static_cast<int>(static_cast<std::uint32_t>(v));
			break;
		case 8:
			ecode = INT_MIN + static_cast<int>(v % 16);
			break;
		default:
			ecode = INT_MAX - static_cast<int>(v % 16);
			break;
		}
		check_gai(ecode, "random");
	}
}

void
print_row(const stats &s)
{
	std::printf("  %-24s %12llu %12llu  %s\n", s.name, s.cases,
	    s.failures, s.failures == 0 ? "ok" : "FAILED");
}

} /* namespace */

int
main()
{
	edge_cases();
	random_sweep();

	unsigned long long total_cases = gai_stats.cases;
	unsigned long long total_failures = gai_stats.failures;

	std::printf("\nbatch b0129 (lib/libc/net) differential results\n");
	std::printf("  %-24s %12s %12s  %s\n", "function", "cases",
	    "failures", "status");
	print_row(gai_stats);
	std::printf("  %-24s %12llu %12llu  %s\n", "TOTAL", total_cases,
	    total_failures, total_failures == 0 ? "ok" : "FAILED");

	if (total_failures != 0) {
		std::printf("\n%llu case(s) diverged from the oracle\n",
		    total_failures);
		return 1;
	}
	return 0;
}
