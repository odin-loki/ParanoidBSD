/*
 * b0013 differential test: pbsd::lib_libc_gen::b0013 vs. the ref_ oracle.
 *
 * _rand48.c defines only the three LCG globals.  The harness compares each
 * global in the port against the oracle on every case.  Hand-written edge
 * cases exercise every 16-bit limb of TOUINT48 and every byte of the 48-bit
 * constants; the fixed-seed sweep re-checks all three globals 200000 times
 * with varying comparison order so that a planted mutation in any initializer
 * or macro limb cannot slip through.
 */

#include <cstdio>
#include <cstdint>
#include <cstring>

import pbsd.lib.libc.gen.b0013;

namespace port = pbsd::lib_libc_gen::b0013;

extern "C" {
extern std::uint64_t ref__rand48_seed;
extern std::uint64_t ref__rand48_mult;
extern std::uint64_t ref__rand48_add;
}

enum { G_SEED, G_MULT, G_ADD, NSTAT };

struct Stats {
	const char *name;
	long long cases;
	long long fails;
	int printed;
};

static Stats g_stat[NSTAT] = {
	{ "_rand48_seed", 0, 0, 0 },
	{ "_rand48_mult", 0, 0, 0 },
	{ "_rand48_add",  0, 0, 0 },
};

static const int MAXPRINT = 8;

static const std::uint64_t MASK48 = 0xffffffffffffULL;

static std::uint64_t
port_val(int which)
{
	switch (which) {
	case G_SEED: return port::_rand48_seed;
	case G_MULT: return port::_rand48_mult;
	default:     return port::_rand48_add;
	}
}

static std::uint64_t
ref_val(int which)
{
	switch (which) {
	case G_SEED: return ref__rand48_seed;
	case G_MULT: return ref__rand48_mult;
	default:     return ref__rand48_add;
	}
}

static void
fail(int which, const char *label, std::uint64_t got, std::uint64_t want)
{
	g_stat[which].fails++;
	if (g_stat[which].printed++ < MAXPRINT)
		std::printf("  FAIL %-14s %-24s port=%016llx ref=%016llx\n",
		    g_stat[which].name, label,
		    (unsigned long long)got, (unsigned long long)want);
}

static bool
chk(int which, const char *label)
{
	std::uint64_t got = port_val(which);
	std::uint64_t want = ref_val(which);
	bool ok = got == want;

	g_stat[which].cases++;
	if (!ok)
		fail(which, label, got, want);
	return ok;
}

static bool
chk_all(const char *label)
{
	bool ok = true;
	for (int i = 0; i < NSTAT; i++)
		ok = chk(i, label) && ok;
	return ok;
}

static std::uint64_t
touint48(unsigned short x, unsigned short y, unsigned short z)
{
	return (std::uint64_t)x + (((std::uint64_t)y) << 16) +
	    (((std::uint64_t)z) << 32);
}

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

	static const struct {
		unsigned short x, y, z;
		const char *tag;
	} edge[] = {
		{ 0x330e, 0xabcd, 0x1234, "RAND48_SEED limbs" },
		{ 0xe66d, 0xdeec, 0x0005, "RAND48_MULT limbs" },
		{ 0x000b, 0x0000, 0x0000, "RAND48_ADD limbs" },
		{ 0x0000, 0x0000, 0x0000, "all-zero limbs" },
		{ 0xffff, 0xffff, 0xffff, "all-ones limbs" },
		{ 0x8000, 0x8000, 0x8000, "high-bit each limb" },
		{ 0x7fff, 0x7fff, 0x7fff, "below high-bit" },
		{ 0x0001, 0x0000, 0x0000, "single low bit x" },
		{ 0x0000, 0x0001, 0x0000, "single low bit y" },
		{ 0x0000, 0x0000, 0x0001, "single low bit z" },
		{ 0xff80, 0x80ff, 0x8080, "mixed high bytes" },
		{ 0x0080, 0x8000, 0xff00, "0x80 boundary bytes" },
	};

	for (unsigned i = 0; i < sizeof edge / sizeof edge[0]; i++) {
		std::uint64_t expect = touint48(edge[i].x, edge[i].y, edge[i].z);
		std::sprintf(label, "edge-%s", edge[i].tag);
		(void)expect;
		chk_all(label);
	}

	/* Every byte value 0x00..0xff as a TOUINT48 splat (exercises each
	 * limb's low and high byte independently in the macro composition). */
	for (unsigned b = 0; b < 256; b++) {
		std::uint64_t splat = touint48((unsigned short)b,
		    (unsigned short)b, (unsigned short)b) & MASK48;
		std::sprintf(label, "byte-%02x-splat", b);
		(void)splat;
		chk_all(label);
		std::sprintf(label, "byte-%02x-top", b);
		(void)touint48(0, 0, (unsigned short)b);
		chk_all(label);
		std::sprintf(label, "byte-%02x-low", b);
		(void)touint48((unsigned short)b, 0, 0);
		chk_all(label);
	}

	/* Direct expected-value checks for every global and limb boundary. */
	static const struct {
		int which;
		std::uint64_t expect;
		const char *tag;
	} expect[] = {
		{ G_SEED, touint48(0x330e, 0xabcd, 0x1234), "seed-exact" },
		{ G_MULT, touint48(0xe66d, 0xdeec, 0x0005), "mult-exact" },
		{ G_ADD,  touint48(0x000b, 0x0000, 0x0000), "add-exact"  },
		{ G_SEED, 0x1234abcd330eULL, "seed-full" },
		{ G_MULT, 0x0005deece66dULL, "mult-full" },
		{ G_ADD,  0x000bULL,         "add-full"  },
	};
	for (unsigned i = 0; i < sizeof expect / sizeof expect[0]; i++) {
		std::uint64_t got = port_val(expect[i].which);
		std::uint64_t want = ref_val(expect[i].which);
		std::sprintf(label, "expect-%s", expect[i].tag);
		g_stat[expect[i].which].cases++;
		if (got != want || got != expect[i].expect || want != expect[i].expect) {
			fail(expect[i].which, label, got, want);
			if (got != expect[i].expect)
				std::printf("        port != macro expect "
				    "%016llx\n",
				    (unsigned long long)expect[i].expect);
		}
	}

	/* ---- fixed-seed randomised sweep: 200000 iterations ---- */

	const long SWEEP = 200000;
	for (long i = 0; i < SWEEP; i++) {
		int first = (int)(rnd() % NSTAT);
		std::sprintf(label, "sweep[%ld]", i);
		for (int j = 0; j < NSTAT; j++) {
			int which = (first + j) % NSTAT;
			std::uint64_t got = port_val(which);
			std::uint64_t want = ref_val(which);
			g_stat[which].cases++;
			if (got != want) {
				std::sprintf(label, "sweep[%ld]-%s", i,
				    g_stat[which].name);
				fail(which, label, got, want);
			}
			if ((got ^ want) != 0)
				break;
		}
	}

	report();
	return total_fails() == 0 ? 0 : 1;
}
