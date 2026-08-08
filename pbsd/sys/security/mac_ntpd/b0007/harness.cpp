/*
 * Batch b0007 differential harness.
 *
 * Compares ntpd_priv_grant in pbsd.sys.security.mac.ntpd.b0007 against the
 * ref_ oracle in oracle.c.
 */

import pbsd.sys.security.mac.ntpd.b0007;

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <climits>
#include <initializer_list>

namespace P = pbsd::sys_security_mac_ntpd::b0007;

extern "C" {
struct ucred {
	uint32_t cr_uid;
};

void ref_set_ntpd_enabled(int v);
void ref_set_ntpd_uid(int v);
int ref_ntpd_priv_grant(struct ucred *cred, int priv);
}

static const int PRIV_ADJTIME = 15;
static const int PRIV_CLOCK_SETTIME = 17;
static const int PRIV_NTP_ADJTIME = 16;
static const int PRIV_NETINET_RESERVEDPORT = 490;
static const int PRIV_NETINET_REUSEPORT = 504;
static const int EPERM = 1;

static const int kGrantedPrivs[] = {
	PRIV_ADJTIME,
	PRIV_CLOCK_SETTIME,
	PRIV_NTP_ADJTIME,
	PRIV_NETINET_RESERVEDPORT,
	PRIV_NETINET_REUSEPORT,
};
static const int kNGranted =
    static_cast<int>(sizeof(kGrantedPrivs) / sizeof(kGrantedPrivs[0]));

struct Stat {
	const char *name;
	long cases;
	long failures;
	long reported;
};

static Stat st_grant = { "ntpd_priv_grant", 0, 0, 0 };

static constexpr unsigned char GUARD = 0x7f;
static constexpr std::size_t GUARD_LEN = 64;

struct Ucred {
	uint32_t cr_uid;
};

struct UcredBuf {
	unsigned char pre[GUARD_LEN];
	Ucred cred;
	unsigned char post[GUARD_LEN];
};

static void
fail(Stat &s, const char *fmt, long a, long b, long c, long d, long e,
    long f)
{
	s.failures++;
	if (s.reported < 12) {
		s.reported++;
		std::printf("  FAIL %s: ", s.name);
		std::printf(fmt, a, b, c, d, e, f);
		std::printf("\n");
	}
}

static void
init_ucred_buf(UcredBuf &b, uint32_t uid)
{
	std::memset(&b, GUARD, sizeof(b));
	b.cred.cr_uid = uid;
}

static bool
ucred_buf_unchanged(const UcredBuf &after, const UcredBuf &before)
{
	return (std::memcmp(after.pre, before.pre, GUARD_LEN) == 0 &&
	    after.cred.cr_uid == before.cred.cr_uid &&
	    std::memcmp(after.post, before.post, GUARD_LEN) == 0);
}

static void
set_policy(int enabled, int uid)
{
	P::set_ntpd_enabled(enabled);
	P::set_ntpd_uid(uid);
	ref_set_ntpd_enabled(enabled);
	ref_set_ntpd_uid(uid);
}

static void
check_grant(int enabled, int uid, uint32_t cr_uid, int priv)
{
	st_grant.cases++;

	set_policy(enabled, uid);

	UcredBuf port_buf, ref_buf, port_snap, ref_snap;
	init_ucred_buf(port_buf, cr_uid);
	init_ucred_buf(ref_buf, cr_uid);
	port_snap = port_buf;
	ref_snap = ref_buf;

	int got = P::ntpd_priv_grant(
	    reinterpret_cast<P::ucred *>(&port_buf.cred), priv);
	int want = ref_ntpd_priv_grant(
	    reinterpret_cast<struct ucred *>(&ref_buf.cred), priv);

	bool ok = true;
	if (got != want) {
		ok = false;
		fail(st_grant, "enabled=%ld uid=%ld cr_uid=%lu priv=%ld -> "
		    "port %ld != ref %ld", (long)enabled, (long)uid,
		    (unsigned long)cr_uid, (long)priv, (long)got, (long)want);
	}
	if (!ucred_buf_unchanged(port_buf, port_snap)) {
		if (ok)
			fail(st_grant, "port ucred buffer modified enabled=%ld "
			    "uid=%ld cr_uid=%lu priv=%ld", (long)enabled,
			    (long)uid, (unsigned long)cr_uid, (long)priv, 0L,
			    0L);
		ok = false;
	}
	if (!ucred_buf_unchanged(ref_buf, ref_snap)) {
		if (ok)
			fail(st_grant, "ref ucred buffer modified enabled=%ld "
			    "uid=%ld cr_uid=%lu priv=%ld", (long)enabled,
			    (long)uid, (unsigned long)cr_uid, (long)priv, 0L,
			    0L);
		ok = false;
	}
	if (!ok)
		return;
}

/* ------------------------------------------------------------------ */
/* fixed-seed PRNG (splitmix64)                                       */
/* ------------------------------------------------------------------ */

static uint64_t rng_state;

static void
rng_seed(uint64_t s)
{
	rng_state = s;
}

static uint64_t
next64(void)
{
	uint64_t z = (rng_state += 0x9E3779B97F4A7C15ULL);
	z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
	z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
	return (z ^ (z >> 31));
}

static uint32_t
next32(void)
{
	return (static_cast<uint32_t>(next64() >> 32));
}

static uint32_t
below(uint32_t n)
{
	return (next32() % n);
}

static int
pick_priv(void)
{
	switch (below(8)) {
	case 0:
		return (kGrantedPrivs[below(kNGranted)]);
	case 1:
		return (kGrantedPrivs[below(kNGranted)] + 1);
	case 2:
		return (kGrantedPrivs[below(kNGranted)] - 1);
	case 3:
		return (0);
	case 4:
		return (-1);
	case 5:
		return (INT_MAX);
	case 6:
		return (INT_MIN);
	default:
		return (static_cast<int>(next32()));
	}
}

int
main(void)
{
	/*
	 * Policy disabled (ntpd_enabled == 0): every priv and uid combination
	 * must return EPERM.
	 */
	for (int uid : { 0, 1, 123, -1, INT_MAX, INT_MIN }) {
		for (uint32_t cr : { 0u, 1u, 123u, 0x7fu, 0x80u, 0xffu,
		    0xffffffffu }) {
			for (int priv : { 0, PRIV_ADJTIME, PRIV_NETINET_REUSEPORT,
			    999, INT_MIN, INT_MAX }) {
				check_grant(0, uid, cr, priv);
			}
		}
	}

	/*
	 * enabled=0 is the only value that must disable; verify nonzero
	 * values other than 1 still enable.
	 */
	for (int en : { 1, 2, -1, 127, INT_MAX, INT_MIN }) {
		check_grant(en, 123, 123u, PRIV_ADJTIME);
		check_grant(en, 123, 122u, PRIV_ADJTIME);
	}

	/*
	 * uid match boundary around the default ntpd_uid (123).
	 */
	for (uint32_t cr : { 121u, 122u, 123u, 124u, 125u }) {
		for (int priv : { PRIV_ADJTIME - 1, PRIV_ADJTIME,
		    PRIV_NTP_ADJTIME, PRIV_CLOCK_SETTIME,
		    PRIV_NETINET_RESERVEDPORT - 1, PRIV_NETINET_RESERVEDPORT,
		    PRIV_NETINET_REUSEPORT, PRIV_NETINET_REUSEPORT + 1, 0,
		    -1, 42, 999 }) {
			check_grant(1, 123, cr, priv);
		}
	}

	/* Each granted privilege must return 0 when uid matches. */
	for (int i = 0; i < kNGranted; i++)
		check_grant(1, 123, 123u, kGrantedPrivs[i]);

	/* Adjacent and distant priv values must return EPERM. */
	static const int denied[] = {
		-2, -1, 0, 1, 14, 18, 19, 100, 489, 491, 503, 505, 1000,
		INT_MIN, INT_MAX,
	};
	for (int priv : denied)
		check_grant(1, 123, 123u, priv);

	/*
	 * Signed/unsigned uid comparison: ntpd_uid is int, cr_uid is uid_t.
	 */
	static const int policy_uids[] = { -1, 0, 1, 122, 123, 124, 0x7f,
	    0x80, INT_MAX, INT_MIN };
	static const uint32_t cred_uids[] = {
		0u, 1u, 122u, 123u, 124u, 0x7fu, 0x80u, 0xffu, 0x80000000u,
		0xffffffffu,
	};
	for (int puid : policy_uids) {
		for (uint32_t cuid : cred_uids) {
			for (int priv : { PRIV_ADJTIME, PRIV_CLOCK_SETTIME,
			    PRIV_NETINET_REUSEPORT, 0, 42, 999 }) {
				check_grant(1, puid, cuid, priv);
			}
		}
	}

	/* NUL-heavy / high-bit byte patterns in uid words (as uint32). */
	static const uint32_t bit_uids[] = {
		0x00000000u, 0x0000007fu, 0x00000080u, 0x000000ffu,
		0x7f7f7f7fu, 0x80808080u, 0xffffffffu, 0x00000001u,
	};
	for (uint32_t cuid : bit_uids) {
		for (uint32_t puid_u : bit_uids) {
			int puid = static_cast<int>(puid_u);
			check_grant(1, puid, cuid, PRIV_NTP_ADJTIME);
			check_grant(1, puid, cuid, 0);
		}
	}

	/* ---------------- fixed-seed randomised sweep ---------------- */
	rng_seed(0xB0007ULL);

	const long ITERS = 200000;
	for (long i = 0; i < ITERS; i++) {
		int enabled;
		switch (below(6)) {
		case 0:
			enabled = 0;
			break;
		case 1:
			enabled = 1;
			break;
		case 2:
			enabled = 2;
			break;
		case 3:
			enabled = -1;
			break;
		case 4:
			enabled = INT_MAX;
			break;
		default:
			enabled = static_cast<int>(next32());
			break;
		}

		int puid;
		switch (below(5)) {
		case 0:
			puid = 123;
			break;
		case 1:
			puid = static_cast<int>(next32() & 0xffu);
			break;
		case 2:
			puid = -static_cast<int>(below(4096) + 1);
			break;
		case 3:
			puid = INT_MAX;
			break;
		default:
			puid = static_cast<int>(next32());
			break;
		}

		uint32_t cuid;
		switch (below(5)) {
		case 0:
			cuid = static_cast<uint32_t>(puid);
			break;
		case 1:
			cuid = 123u;
			break;
		case 2:
			cuid = static_cast<uint32_t>(puid) ^ 1u;
			break;
		case 3:
			cuid = (below(2) == 0) ? 0x80u : 0xffu;
			break;
		default:
			cuid = next32();
			break;
		}

		int priv = pick_priv();
		check_grant(enabled, puid, cuid, priv);

		/* Occasionally force exact priv switch labels. */
		if (below(16) == 0)
			check_grant(enabled, puid, cuid,
			    kGrantedPrivs[below(kNGranted)]);
	}

	/* ---------------------------- report ---------------------------- */
	Stat *all[] = { &st_grant };
	long total_cases = 0, total_fails = 0;

	std::printf("\n%-32s %12s %10s %s\n", "function", "cases", "failures",
	    "result");
	std::printf("---------------------------------------------------"
	    "-----------------\n");
	for (Stat *s : all) {
		total_cases += s->cases;
		total_fails += s->failures;
		std::printf("%-32s %12ld %10ld %s\n", s->name, s->cases,
		    s->failures, s->failures == 0 ? "PASS" : "FAIL");
	}
	std::printf("---------------------------------------------------"
	    "-----------------\n");
	std::printf("%-32s %12ld %10ld %s\n", "TOTAL", total_cases,
	    total_fails, total_fails == 0 ? "PASS" : "FAIL");

	return (total_fails == 0 ? 0 : 1);
}
