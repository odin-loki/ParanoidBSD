/*
 * harness.cpp -- differential test for PBSD batch b0006.
 *
 * Drives the C++23 port in port.cppm and the C reference in oracle.c with
 * identical inputs and compares every observable: the return value, the
 * bytes of the credential structure the callee was handed, and the guard
 * bytes surrounding it.
 *
 * mac_pimd.c contains exactly one function, pimd_priv_grant().  It writes to
 * no buffer, returns no pointer and holds no iterator state, so the buffer /
 * offset / iterator protocols degenerate to: place the struct ucred inside a
 * 0x7f-filled arena and require the whole arena to be byte-identical
 * afterwards.  The function's real inputs are four values -- the two
 * sysctl-backed globals pimd_enabled and pimd_uid, the credential's cr_uid,
 * and priv -- and all four are swept.
 */

#include <climits>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <new>
#include <random>

import pbsd.sys.security.mac.pimd.b0006;

namespace port = pbsd::sys_security_mac_pimd::b0006;

/* Layout-identical mirror of the struct ucred defined in oracle.c. */
struct ucred {
	unsigned int	cr_ref;
	std::uint32_t	cr_uid;
	std::uint32_t	cr_ruid;
	std::uint32_t	cr_svuid;
	std::uint32_t	cr_rgid;
	std::uint32_t	cr_svgid;
};

extern "C" {
extern int pimd_enabled;
extern int pimd_uid;
int ref_pimd_priv_grant(struct ucred *cred, int priv);
}

static_assert(sizeof(struct ucred) == sizeof(port::ucred),
    "oracle and port credential layouts must match");

namespace {

constexpr std::size_t ARENA = 64;
constexpr std::size_t OFFSET = 8;
constexpr unsigned char GUARD = 0x7f;

struct Stats {
	const char	*name;
	unsigned long	 cases;
	unsigned long	 failures;
	unsigned long	 granted;	/* observed return 0 */
	unsigned long	 denied;	/* observed return EPERM */
};

Stats g_pg = { "pimd_priv_grant", 0, 0, 0, 0 };

unsigned long g_reported = 0;

/*
 * Fill an arena with the guard byte, materialise a credential at OFFSET whose
 * cr_uid is the requested value and whose other members carry fixed sentinel
 * values, and return a pointer to it.
 */
template <typename Cred>
Cred *
place(unsigned char *arena, std::uint32_t cr_uid, std::uint32_t tag)
{
	std::memset(arena, GUARD, ARENA);
	Cred *c = new (arena + OFFSET) Cred{};
	c->cr_ref = 0xa5a5a5a5u;
	c->cr_uid = cr_uid;
	c->cr_ruid = tag;
	c->cr_svuid = ~tag;
	c->cr_rgid = tag ^ 0x5a5a5a5au;
	c->cr_svgid = 0xdeadbeefu;
	return c;
}

void
report(const char *phase, int enabled, int uidvar, std::uint32_t cr_uid,
    int priv, int got, int want, const char *what)
{
	if (g_reported >= 20) {
		if (g_reported == 20)
			std::printf("  ... further failures suppressed\n");
		g_reported++;
		return;
	}
	g_reported++;
	std::printf("  FAIL [%s] %s: pimd_enabled=%d pimd_uid=%d "
	    "cr_uid=0x%08lx priv=%d  port=%d oracle=%d\n",
	    phase, what, enabled, uidvar,
	    static_cast<unsigned long>(cr_uid), priv, got, want);
}

/*
 * Run one differential case.  set_globals selects whether the two sysctl
 * variables are written before the call; the pristine phase leaves them at
 * their static initialisers so that a mutated initialiser is observable.
 */
void
run_case(const char *phase, bool set_enabled, int enabled, bool set_uidvar,
    int uidvar, std::uint32_t cr_uid, int priv, std::uint32_t tag)
{
	alignas(16) unsigned char parena[ARENA];
	alignas(16) unsigned char oarena[ARENA];

	if (set_enabled) {
		port::pimd_enabled = enabled;
		pimd_enabled = enabled;
	}
	if (set_uidvar) {
		port::pimd_uid = uidvar;
		pimd_uid = uidvar;
	}

	port::ucred *pc = place<port::ucred>(parena, cr_uid, tag);
	struct ucred *oc = place<struct ucred>(oarena, cr_uid, tag);

	int pr = port::pimd_priv_grant(pc, priv);
	int orr = ref_pimd_priv_grant(oc, priv);

	g_pg.cases++;
	bool bad = false;

	if (pr != orr) {
		bad = true;
		report(phase, port::pimd_enabled, port::pimd_uid, cr_uid, priv,
		    pr, orr, "return value");
	}
	if (std::memcmp(parena, oarena, ARENA) != 0) {
		bad = true;
		report(phase, port::pimd_enabled, port::pimd_uid, cr_uid, priv,
		    pr, orr, "arena bytes");
	}
	/* The globals must not have been disturbed by either callee. */
	if (port::pimd_enabled != pimd_enabled ||
	    port::pimd_uid != pimd_uid) {
		bad = true;
		report(phase, port::pimd_enabled, port::pimd_uid, cr_uid, priv,
		    pr, orr, "global state");
	}

	if (bad)
		g_pg.failures++;

	if (orr == 0)
		g_pg.granted++;
	else
		g_pg.denied++;
}

const int ENABLED_POOL[] = {
	0, 1, -1, 2, 0x10000, INT_MIN, INT_MAX
};

const int UIDVAR_POOL[] = {
	0, 1, -1, -2, 2, 1000, INT_MIN, INT_MAX
};

const std::uint32_t CRUID_POOL[] = {
	0u, 1u, 2u, 1000u, 65534u,
	0x7fffffffu, 0x80000000u, 0xfffffffeu, 0xffffffffu
};

const int PRIV_POOL[] = {
	0, 1, -1, 490, 491, 494, 495, 496, 497, 498, 502,
	INT_MIN, INT_MAX
};

std::uint32_t
pick_cruid(std::mt19937_64 &rng)
{
	std::uint64_t r = rng();
	switch (r % 8u) {
	case 0: return 0u;
	case 1: return 1u;
	case 2: return 0xffffffffu;
	case 3: return 0x7fffffffu;
	case 4: return 0x80000000u;
	case 5: return static_cast<std::uint32_t>((r >> 8) % 8u);
	case 6: return 0xfffffff8u + static_cast<std::uint32_t>((r >> 8) % 8u);
	default: return static_cast<std::uint32_t>(r >> 16);
	}
}

int
pick_priv(std::mt19937_64 &rng)
{
	std::uint64_t r = rng();
	switch (r % 8u) {
	case 0: return 496;
	case 1: return 495;
	case 2: return 497;
	case 3: return 0;
	case 4: return -1;
	case 5: return 490 + static_cast<int>((r >> 8) % 13u);
	case 6: return ((r >> 8) & 1u) ? INT_MIN : INT_MAX;
	default: return static_cast<int>(static_cast<std::uint32_t>(r >> 16));
	}
}

int
pick_enabled(std::mt19937_64 &rng)
{
	std::uint64_t r = rng();
	switch (r % 8u) {
	case 0: case 1: case 2: return 0;
	case 3: case 4: case 5: return 1;
	case 6: return -1;
	default: return static_cast<int>(static_cast<std::uint32_t>(r >> 16));
	}
}

int
pick_uidvar(std::mt19937_64 &rng, std::uint32_t cr_uid)
{
	std::uint64_t r = rng();
	switch (r % 8u) {
	/* Bias hard towards the value that makes the comparison true. */
	case 0: case 1: case 2:
		return static_cast<int>(cr_uid);
	case 3: return 0;
	case 4: return 1;
	case 5: return -1;
	case 6: return static_cast<int>(cr_uid) + 1;
	default: return static_cast<int>(static_cast<std::uint32_t>(r >> 16));
	}
}

} /* anonymous namespace */

int
main(void)
{
	std::uint32_t tag = 0x11223344u;

	/*
	 * Phase 1 -- pristine.  Neither global is written, so both sides run
	 * against their static initialisers.  With pimd_enabled == 0 every
	 * call must be denied; a port whose initialiser was perturbed grants.
	 */
	for (std::uint32_t cr_uid : CRUID_POOL)
		for (int priv : PRIV_POOL)
			run_case("pristine", false, 0, false, 0, cr_uid, priv,
			    tag++);

	/*
	 * Phase 2 -- enable only.  pimd_uid keeps its initialiser, so cr_uid
	 * of 0 must match it and grant PRIV_NETINET_MROUTE.
	 */
	for (int enabled : ENABLED_POOL)
		for (std::uint32_t cr_uid : CRUID_POOL)
			for (int priv : PRIV_POOL)
				run_case("enable-only", true, enabled, false, 0,
				    cr_uid, priv, tag++);

	/*
	 * Phase 3 -- exhaustive cross product of the hand-written pools.
	 * Covers both sides of every boundary: pimd_enabled zero/non-zero,
	 * cr_uid equal/adjacent to pimd_uid (including the unsigned wrap that
	 * makes pimd_uid == -1 match cr_uid == 0xffffffff), and priv on, one
	 * below and one above PRIV_NETINET_MROUTE.
	 */
	for (int enabled : ENABLED_POOL)
		for (int uidvar : UIDVAR_POOL)
			for (std::uint32_t cr_uid : CRUID_POOL)
				for (int priv : PRIV_POOL)
					run_case("cross", true, enabled, true,
					    uidvar, cr_uid, priv, tag++);

	/*
	 * Phase 4 -- targeted signedness cases: pimd_uid negative against the
	 * cr_uid that it converts to, and the neighbours on either side.
	 */
	for (int k = 1; k <= 64; k++) {
		std::uint32_t match = static_cast<std::uint32_t>(-k);
		for (int priv : PRIV_POOL) {
			run_case("signedness", true, 1, true, -k, match, priv,
			    tag++);
			run_case("signedness", true, 1, true, -k, match - 1u,
			    priv, tag++);
			run_case("signedness", true, 1, true, -k, match + 1u,
			    priv, tag++);
		}
	}

	/* Phase 5 -- fixed-seed randomised sweep. */
	std::mt19937_64 rng(0x50425344'62303036ull);
	for (long i = 0; i < 200000; i++) {
		int enabled = pick_enabled(rng);
		std::uint32_t cr_uid = pick_cruid(rng);
		int uidvar = pick_uidvar(rng, cr_uid);
		int priv = pick_priv(rng);
		run_case("random", true, enabled, true, uidvar, cr_uid, priv,
		    tag++);
	}

	std::printf("\n%-24s %12s %12s %12s %12s\n", "function", "cases",
	    "failures", "granted", "denied");
	std::printf("%-24s %12lu %12lu %12lu %12lu\n", g_pg.name, g_pg.cases,
	    g_pg.failures, g_pg.granted, g_pg.denied);

	/*
	 * Both outcomes must actually have been produced, otherwise the sweep
	 * never reached the grant path and the comparison proves nothing.
	 */
	int rc = (g_pg.failures == 0) ? 0 : 1;
	if (g_pg.granted == 0 || g_pg.denied == 0) {
		std::printf("\nERROR: sweep failed to exercise both outcomes\n");
		rc = 1;
	}
	std::printf("\n%s\n", rc == 0 ? "PASS" : "FAIL");
	return rc;
}
