/*
 * harness.cpp -- differential test for PBSD batch b0006.
 *
 * Drives pimd_priv_grant against the ref_ oracle with hand-written edge
 * cases and a fixed-seed randomised sweep.  Each ucred operand is staged
 * in two guard buffers pre-filled with 0x7f so any stray write is caught.
 */

#include <climits>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>

import pbsd.sys.security.mac.pimd.b0006;

namespace port = pbsd::sys_security_mac_pimd::b0006;

extern "C" {
extern int ref_pimd_enabled;
extern int ref_pimd_uid;
int ref_pimd_priv_grant_export(port::ucred *cred, int priv);
}

namespace {

constexpr int MAX_REPORT = 10;
constexpr long RANDOM_ITERATIONS = 250000;
constexpr int PRIV_NETINET_MROUTE = 496;
constexpr int EPERM = 1;

constexpr std::size_t GUARD_SIZE = 64;
constexpr std::size_t GUARD_OFF = 16;

struct Stat {
	const char *name;
	long cases;
	long fails;
	int reported;
};

Stat st_pimd_priv_grant{"pimd_priv_grant", 0, 0, 0};

struct GuardCred {
	unsigned char b[GUARD_SIZE];

	void fill()
	{
		std::memset(b, 0x7f, GUARD_SIZE);
	}

	port::ucred *cred()
	{
		return reinterpret_cast<port::ucred *>(b + GUARD_OFF);
	}

	const port::ucred *cred() const
	{
		return reinterpret_cast<const port::ucred *>(b + GUARD_OFF);
	}

	void set_uid(std::uint32_t uid)
	{
		cred()->cr_uid = uid;
	}

	bool intact() const
	{
		for (std::size_t i = 0; i < GUARD_SIZE; ++i) {
			if (b[i] != 0x7f)
				return false;
		}
		return true;
	}
};

std::uint64_t prng_state;

void prng_seed(std::uint64_t seed)
{
	prng_state = seed;
}

std::uint64_t prng_next()
{
	std::uint64_t z = (prng_state += 0x9E3779B97F4A7C15ULL);
	z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
	z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
	return z ^ (z >> 31);
}

std::uint32_t rand_u32()
{
	return static_cast<std::uint32_t>(prng_next());
}

int rand_int()
{
	return static_cast<int>(prng_next());
}

void report_fail(Stat &st, const char *detail)
{
	if (st.reported < MAX_REPORT)
		std::fprintf(stderr, "  FAIL %s: %s\n", st.name, detail);
	++st.reported;
}

void check_case(Stat &st, int enabled, int uid, std::uint32_t cr_uid, int priv)
{
	++st.cases;

	port::pimd_enabled = enabled;
	port::pimd_uid = uid;
	ref_pimd_enabled = enabled;
	ref_pimd_uid = uid;

	GuardCred g_port;
	GuardCred g_ref;
	g_port.fill();
	g_ref.fill();
	g_port.set_uid(cr_uid);
	g_ref.set_uid(cr_uid);

	const int got_port =
	    port::pimd_priv_grant_export(g_port.cred(), priv);
	const int got_ref =
	    ref_pimd_priv_grant_export(g_ref.cred(), priv);

	if (got_port != got_ref) {
		++st.fails;
		char buf[256];
		std::snprintf(buf, sizeof(buf),
		    "enabled=%d uid=%d cr_uid=%u priv=%d -> port=%d ref=%d",
		    enabled, uid, cr_uid, priv, got_port, got_ref);
		report_fail(st, buf);
		return;
	}

	if (!g_port.intact() || !g_ref.intact()) {
		++st.fails;
		report_fail(st, "guard buffer corrupted");
	}
}

void run_edge_cases(Stat &st)
{
	struct Case {
		int enabled;
		int uid;
		std::uint32_t cr_uid;
		int priv;
	};

	static const Case cases[] = {
		/* disabled: short-circuit pimd_enabled */
		{0, 0, 0, PRIV_NETINET_MROUTE},
		{0, 0, 0, 0},
		{0, 0, 0, -1},
		{0, 1, 1, PRIV_NETINET_MROUTE},
		{0, -1, UINT32_MAX, PRIV_NETINET_MROUTE},

		/* enabled but uid mismatch */
		{1, 0, 1, PRIV_NETINET_MROUTE},
		{1, 1, 0, PRIV_NETINET_MROUTE},
		{1, 100, 99, PRIV_NETINET_MROUTE},
		{1, -1, 0, PRIV_NETINET_MROUTE},
		{1, 0, UINT32_MAX, PRIV_NETINET_MROUTE},
		{1, INT_MAX, static_cast<std::uint32_t>(INT_MAX) + 1U,
		    PRIV_NETINET_MROUTE},

		/* enabled, uid match, priv boundary around MROUTE */
		{1, 0, 0, PRIV_NETINET_MROUTE},
		{1, 0, 0, PRIV_NETINET_MROUTE - 1},
		{1, 0, 0, PRIV_NETINET_MROUTE + 1},
		{1, 0, 0, 0},
		{1, 0, 0, -1},
		{1, 0, 0, INT_MAX},
		{1, 0, 0, INT_MIN},

		/* signed pimd_uid vs unsigned cr_uid conversion */
		{1, -1, UINT32_MAX, PRIV_NETINET_MROUTE},
		{1, -1, UINT32_MAX, 0},
		{1, -1, UINT32_MAX - 1, PRIV_NETINET_MROUTE},
		{1, INT_MIN, static_cast<std::uint32_t>(INT_MIN),
		    PRIV_NETINET_MROUTE},

		/* truthy enabled values beyond 1 */
		{2, 5, 5, PRIV_NETINET_MROUTE},
		{-1, 3, 3, PRIV_NETINET_MROUTE},
		{INT_MAX, 7, 7, PRIV_NETINET_MROUTE},

		/* high-bit cr_uid bytes */
		{1, 0x80, 0x80, PRIV_NETINET_MROUTE},
		{1, 0xFF, 0xFF, PRIV_NETINET_MROUTE},
		{1, 0x80000000U, 0x80000000U, PRIV_NETINET_MROUTE},
		{1, UINT32_MAX, UINT32_MAX, PRIV_NETINET_MROUTE},
		{1, UINT32_MAX, UINT32_MAX, PRIV_NETINET_MROUTE - 1},

		/* NUL-heavy is N/A for integers; boundary priv sweep */
		{1, 42, 42, 495},
		{1, 42, 42, 496},
		{1, 42, 42, 497},
	};

	for (const Case &c : cases)
		check_case(st, c.enabled, c.uid, c.cr_uid, c.priv);
}

void run_random_sweep(Stat &st)
{
	for (long i = 0; i < RANDOM_ITERATIONS; ++i) {
		const int enabled = rand_int();
		const int uid = rand_int();
		const std::uint32_t cr_uid = rand_u32();
		int priv = rand_int();

		/* bias toward the switch boundary */
		switch (prng_next() & 7) {
		case 0:
			priv = PRIV_NETINET_MROUTE;
			break;
		case 1:
			priv = PRIV_NETINET_MROUTE - 1;
			break;
		case 2:
			priv = PRIV_NETINET_MROUTE + 1;
			break;
		default:
			break;
		}

		check_case(st, enabled, uid, cr_uid, priv);
	}
}

void print_stat(const Stat &st)
{
	std::printf("%-20s %8ld %8ld\n", st.name, st.cases, st.fails);
}

} // namespace

int main()
{
	prng_seed(0xB0006FACEB00CULL);

	run_edge_cases(st_pimd_priv_grant);
	run_random_sweep(st_pimd_priv_grant);

	std::printf("\n%-20s %8s %8s\n", "function", "cases", "failures");
	print_stat(st_pimd_priv_grant);

	const long total_fails = st_pimd_priv_grant.fails;

	return total_fails == 0 ? 0 : 1;
}
