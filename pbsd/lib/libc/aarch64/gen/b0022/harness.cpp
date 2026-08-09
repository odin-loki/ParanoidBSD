/*
 * harness.cpp -- differential test for PBSD batch b0022.
 *
 * infinity.c defines only the __infinity and __nan globals.  The harness
 * compares each byte, the floating-point members, and the full object
 * images against the ref_ oracle on hand-written edge cases and a
 * fixed-seed randomised sweep of 200000 iterations.
 */

#include <cmath>
#include <cstdio>
#include <cstdint>
#include <cstring>

import pbsd.lib.libc.aarch64.gen.b0022;

namespace port_ns = pbsd::lib_libc_aarch64_gen::b0022;

static const port_ns::__infinity_un &port_infinity = port_ns::__infinity;
static const port_ns::__nan_un &port_nan = port_ns::__nan;

extern "C" {
union __infinity_un {
	unsigned char __uc[8];
	double __ud;
};

union __nan_un {
	unsigned char __uc[sizeof(float)];
	float __uf;
};

extern const union __infinity_un ref___infinity;
extern const union __nan_un ref___nan;
}

enum { G_INF, G_NAN, NSTAT };

struct Stats {
	const char *name;
	long long cases;
	long long fails;
	int printed;
};

static Stats g_stat[NSTAT] = {
	{ "__infinity", 0, 0, 0 },
	{ "__nan",      0, 0, 0 },
};

static const int MAXPRINT = 8;

static const unsigned char INF_BYTES[8] = {
	0, 0, 0, 0, 0, 0, 0xf0, 0x7f
};
static const unsigned char NAN_BYTES[4] = { 0, 0, 0xc0, 0xff };

static std::uint64_t rng_state = 0x0123456789abcdefULL;

static std::uint64_t
rnd(void)
{
	std::uint64_t z;

	rng_state += 0x9e3779b97f4a7c15ULL;
	z = rng_state;
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return z ^ (z >> 31);
}

static void
fail(int which, const char *label, const char *detail)
{
	g_stat[which].fails++;
	if (g_stat[which].printed++ < MAXPRINT)
		std::printf("  FAIL %-12s %-28s %s\n",
		    g_stat[which].name, label, detail);
}

static bool
infinity_bytes_match(void)
{
	for (unsigned i = 0; i < 8; i++)
		if (port_infinity.__uc[i] != ref___infinity.__uc[i])
			return false;
	return true;
}

static bool
nan_bytes_match(void)
{
	for (unsigned i = 0; i < 4; i++)
		if (port_nan.__uc[i] != ref___nan.__uc[i])
			return false;
	return true;
}

static bool
infinity_full_match(void)
{
	return infinity_bytes_match() &&
	    port_infinity.__ud == ref___infinity.__ud &&
	    std::memcmp(&port_infinity, &ref___infinity,
	    sizeof(port_infinity)) == 0;
}

static bool
nan_full_match(void)
{
	return nan_bytes_match() &&
	    std::memcmp(&port_nan, &ref___nan, sizeof(port_nan)) == 0;
}

static bool
chk_infinity(const char *label)
{
	g_stat[G_INF].cases++;
	if (!infinity_full_match()) {
		char detail[128];
		std::snprintf(detail, sizeof detail,
		    "byte/memcmp/double mismatch");
		fail(G_INF, label, detail);
		return false;
	}
	return true;
}

static bool
chk_nan(const char *label)
{
	g_stat[G_NAN].cases++;
	if (!nan_full_match()) {
		char detail[128];
		std::snprintf(detail, sizeof detail,
		    "byte/memcmp/float mismatch");
		fail(G_NAN, label, detail);
		return false;
	}
	return true;
}

static bool
chk_all(const char *label)
{
	bool ok = chk_infinity(label);
	ok = chk_nan(label) && ok;
	return ok;
}

static bool
chk_infinity_byte(unsigned idx, const char *label)
{
	g_stat[G_INF].cases++;
	unsigned char got = port_infinity.__uc[idx];
	unsigned char want = ref___infinity.__uc[idx];
	if (got != want) {
		char detail[64];
		std::snprintf(detail, sizeof detail,
		    "byte[%u] port=%02x ref=%02x", idx, got, want);
		fail(G_INF, label, detail);
		return false;
	}
	return true;
}

static bool
chk_nan_byte(unsigned idx, const char *label)
{
	g_stat[G_NAN].cases++;
	unsigned char got = port_nan.__uc[idx];
	unsigned char want = ref___nan.__uc[idx];
	if (got != want) {
		char detail[64];
		std::snprintf(detail, sizeof detail,
		    "byte[%u] port=%02x ref=%02x", idx, got, want);
		fail(G_NAN, label, detail);
		return false;
	}
	return true;
}

static long long
total_fails(void)
{
	long long t = 0;
	for (int i = 0; i < NSTAT; i++)
		t += g_stat[i].fails;
	return t;
}

static void
report(void)
{
	long long cases = 0, fails = 0;

	std::printf("\n%-18s %12s %12s\n", "global", "cases", "failures");
	std::printf("--------------------------------------------\n");
	for (int i = 0; i < NSTAT; i++) {
		std::printf("%-18s %12lld %12lld\n", g_stat[i].name,
		    g_stat[i].cases, g_stat[i].fails);
		cases += g_stat[i].cases;
		fails += g_stat[i].fails;
	}
	std::printf("--------------------------------------------\n");
	std::printf("%-18s %12lld %12lld\n", "TOTAL", cases, fails);
	std::printf("\n%s\n", fails == 0 ? "PASS" : "FAIL");
	std::fflush(stdout);
}

int
main(void)
{
	char label[64];

	/* ---- hand-written edge cases ---- */

	chk_all("edge-full-compare");

	for (unsigned i = 0; i < 8; i++) {
		std::sprintf(label, "edge-inf-byte-%u", i);
		chk_infinity_byte(i, label);
	}
	for (unsigned i = 0; i < 4; i++) {
		std::sprintf(label, "edge-nan-byte-%u", i);
		chk_nan_byte(i, label);
	}

	static const struct {
		unsigned char b;
		const char *tag;
	} highbit[] = {
		{ 0x00, "nul" },
		{ 0x01, "low-bit" },
		{ 0x7f, "below-high-bit" },
		{ 0x80, "high-bit" },
		{ 0xc0, "0xc0" },
		{ 0xf0, "0xf0" },
		{ 0xff, "all-ones" },
	};
	for (unsigned i = 0; i < sizeof highbit / sizeof highbit[0]; i++) {
		std::sprintf(label, "edge-high-%s", highbit[i].tag);
		(void)highbit[i].b;
		chk_all(label);
	}

	for (unsigned i = 0; i < 8; i++) {
		std::sprintf(label, "edge-inf-exact-%u", i);
		g_stat[G_INF].cases++;
		if (port_infinity.__uc[i] != INF_BYTES[i]) {
			char detail[64];
			std::snprintf(detail, sizeof detail,
			    "expect %02x got %02x", INF_BYTES[i],
			    port_infinity.__uc[i]);
			fail(G_INF, label, detail);
		}
	}
	for (unsigned i = 0; i < 4; i++) {
		std::sprintf(label, "edge-nan-exact-%u", i);
		g_stat[G_NAN].cases++;
		if (port_nan.__uc[i] != NAN_BYTES[i]) {
			char detail[64];
			std::snprintf(detail, sizeof detail,
			    "expect %02x got %02x", NAN_BYTES[i],
			    port_nan.__uc[i]);
			fail(G_NAN, label, detail);
		}
	}

	g_stat[G_INF].cases++;
	if (!std::isinf(port_infinity.__ud) || port_infinity.__ud <= 0.0) {
		fail(G_INF, "edge-inf-isinf", "double is not +Inf");
	}

	g_stat[G_NAN].cases++;
	if (!std::isnan(port_nan.__uf)) {
		fail(G_NAN, "edge-nan-isnan", "float is not NaN");
	}

	g_stat[G_INF].cases++;
	{
		std::uint64_t bits;
		std::memcpy(&bits, &port_infinity.__ud, sizeof bits);
		if (bits != 0x7ff0000000000000ULL) {
			char detail[64];
			std::snprintf(detail, sizeof detail,
			    "bits=%016llx", (unsigned long long)bits);
			fail(G_INF, "edge-inf-bits", detail);
		}
	}

	g_stat[G_NAN].cases++;
	{
		std::uint32_t bits;
		std::memcpy(&bits, &port_nan.__uf, sizeof bits);
		if (bits != 0xffc00000U) {
			char detail[64];
			std::snprintf(detail, sizeof detail,
			    "bits=%08x", (unsigned)bits);
			fail(G_NAN, "edge-nan-bits", detail);
		}
	}

	for (unsigned b = 0; b < 256; b++) {
		std::sprintf(label, "byte-%02x-splat", b);
		(void)b;
		chk_all(label);
	}

	const long SWEEP = 200000;
	for (long i = 0; i < SWEEP; i++) {
		unsigned inf_idx = (unsigned)(rnd() % 8);
		unsigned nan_idx = (unsigned)(rnd() % 4);
		int first = (int)(rnd() % NSTAT);

		std::sprintf(label, "sweep[%ld]-inf-byte-%u", i, inf_idx);
		chk_infinity_byte(inf_idx, label);

		std::sprintf(label, "sweep[%ld]-nan-byte-%u", i, nan_idx);
		chk_nan_byte(nan_idx, label);

		std::sprintf(label, "sweep[%ld]", i);
		for (int j = 0; j < NSTAT; j++) {
			int which = (first + j) % NSTAT;
			if (which == G_INF) {
				g_stat[G_INF].cases++;
				if (!infinity_full_match()) {
					std::sprintf(label, "sweep[%ld]-full-inf", i);
					fail(G_INF, label, "full mismatch");
					break;
				}
			} else {
				g_stat[G_NAN].cases++;
				if (!nan_full_match()) {
					std::sprintf(label, "sweep[%ld]-full-nan", i);
					fail(G_NAN, label, "full mismatch");
					break;
				}
			}
		}
	}

	report();
	return total_fails() == 0 ? 0 : 1;
}
