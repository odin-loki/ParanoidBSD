// Differential harness for PBSD batch b0311.

import pbsd.lib.libc.gen.b0311;

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <paths.h>
#include <sys/stat.h>
#include <sys/types.h>

namespace P = pbsd::lib_libc_gen::b0311;

static const unsigned char GUARD = 0x7f;
static const long SWEEP = 50000L;

#ifndef SPECNAMELEN
#define SPECNAMELEN 255
#endif

#ifndef L_ctermid
#define L_ctermid 1024
#endif

#define DEFSZ (sizeof(_PATH_DEV) + SPECNAMELEN)
#define BUFSZ (L_ctermid + 64)

extern "C" {
char *ref_ctermid(char *buf);
char *ref_ctermid_r(char *buf);
}

/* ------------------------------------------------------------------ mocks */

struct mock_cfg {
	int		stat_ret;
	int		stat_errno;
	mode_t		st_mode;
	dev_t		st_rdev;
	int		sysctl_ret;
	int		sysctl_errno;
	char		devname[SPECNAMELEN + 1];
};

static mock_cfg g_cfg;

static void
mock_reset(const mock_cfg *cfg)
{
	g_cfg = *cfg;
}

extern "C" int
test_stat(const char *path, struct stat *sb)
{
	(void)path;
	if (g_cfg.stat_ret != 0) {
		errno = g_cfg.stat_errno;
		return (-1);
	}
	std::memset(sb, 0, sizeof(*sb));
	sb->st_mode = g_cfg.st_mode;
	sb->st_rdev = g_cfg.st_rdev;
	return (0);
}

extern "C" int
test_sysctlbyname(const char *name, void *oldp, size_t *oldlenp, void *newp,
    size_t newlen)
{
	size_t i, max;

	(void)name;
	(void)newp;
	(void)newlen;

	if (g_cfg.sysctl_ret != 0) {
		errno = g_cfg.sysctl_errno;
		return (-1);
	}

	max = oldlenp != nullptr ? *oldlenp : 0;
	for (i = 0; i < max && g_cfg.devname[i] != '\0'; i++)
		((char *)oldp)[i] = g_cfg.devname[i];
	if (oldlenp != nullptr)
		*oldlenp = i;
	return (0);
}

/* ------------------------------------------------------------------ stats */

struct Stat {
	const char *name;
	unsigned long long cases;
	unsigned long long fails;
};

static Stat stats[] = {
	{ "ctermid", 0, 0 },
	{ "ctermid_r", 0, 0 },
};
static const int NSTAT = (int)(sizeof(stats) / sizeof(stats[0]));

static Stat &
S(const char *n)
{
	for (int i = 0; i < NSTAT; i++)
		if (std::strcmp(stats[i].name, n) == 0)
			return stats[i];
	std::fprintf(stderr, "bad stat %s\n", n);
	std::exit(2);
}

static void
fail(Stat &st, const char *msg)
{
	st.fails++;
	if (st.fails <= 8)
		std::printf("  FAIL %s: %s\n", st.name, msg);
}

/* ------------------------------------------------------------------ rng */

struct Rng {
	std::uint64_t s;
	explicit Rng(std::uint64_t seed) : s(seed) {}
	std::uint64_t next()
	{
		s += 0x9E3779B97F4A7C15ull;
		std::uint64_t z = s;
		z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
		z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
		return z ^ (z >> 31);
	}
	std::uint32_t u32() { return (std::uint32_t)next(); }
	int coin() { return (int)(next() & 1); }
};

static long
ptr_off(const char *p, const char *base)
{
	if (p == nullptr)
		return (-1);
	return ((long)(p - base));
}

static void
fill_guard(unsigned char *p, std::size_t n)
{
	std::memset(p, GUARD, n);
}

static void
cfg_defaults(mock_cfg *cfg)
{
	std::memset(cfg, 0, sizeof(*cfg));
	cfg->stat_ret = -1;
	cfg->stat_errno = ENOENT;
	cfg->st_mode = S_IFCHR;
	cfg->st_rdev = 0;
	cfg->sysctl_ret = 0;
	cfg->sysctl_errno = 0;
	cfg->devname[0] = '\0';
}

static void
cfg_stat_ok_chr(mock_cfg *cfg)
{
	cfg->stat_ret = 0;
	cfg->st_mode = S_IFCHR;
}

static void
set_devname(mock_cfg *cfg, const char *s)
{
	std::size_t n;

	n = std::strlen(s);
	if (n >= sizeof(cfg->devname))
		n = sizeof(cfg->devname) - 1;
	std::memcpy(cfg->devname, s, n);
	cfg->devname[n] = '\0';
}

static void
set_devname_bytes(mock_cfg *cfg, const unsigned char *s, std::size_t n)
{
	if (n >= sizeof(cfg->devname))
		n = sizeof(cfg->devname) - 1;
	std::memcpy(cfg->devname, s, n);
	cfg->devname[n] = '\0';
}

struct ctermid_obs {
	char   *ret;
	long	ret_off;
	int	errno_after;
	unsigned char buf[BUFSZ];
};

static ctermid_obs
run_ctermid_side(bool port_side, bool use_null, const mock_cfg *cfg,
    int errno_in, unsigned char *user_buf)
{
	ctermid_obs obs;

	mock_reset(cfg);
	errno = errno_in;
	if (use_null) {
		if (port_side)
			obs.ret = P::ctermid(nullptr);
		else
			obs.ret = ref_ctermid(nullptr);
		obs.ret_off = 0;
	} else {
		fill_guard(user_buf, BUFSZ);
		std::memcpy(obs.buf, user_buf, BUFSZ);
		if (port_side)
			obs.ret = P::ctermid((char *)user_buf);
		else
			obs.ret = ref_ctermid((char *)user_buf);
		std::memcpy(obs.buf, user_buf, BUFSZ);
		obs.ret_off = ptr_off(obs.ret, (const char *)user_buf);
	}
	obs.errno_after = errno;
	return (obs);
}

static void
check_ctermid(bool use_null, const mock_cfg *cfg, int errno_in,
    const char *label)
{
	Stat &st = S("ctermid");
	unsigned char ref_user[BUFSZ];
	unsigned char port_user[BUFSZ];
	ctermid_obs ref_obs, port_obs;

	st.cases++;
	ref_obs = run_ctermid_side(false, use_null, cfg, errno_in, ref_user);
	port_obs = run_ctermid_side(true, use_null, cfg, errno_in, port_user);

	if (use_null) {
		if (ref_obs.ret == nullptr || port_obs.ret == nullptr) {
			fail(st, label);
			return;
		}
		if (std::memcmp(ref_obs.ret, port_obs.ret, DEFSZ) != 0)
			fail(st, label);
	} else {
		if (ref_obs.ret_off != port_obs.ret_off)
			fail(st, label);
		if (std::memcmp(ref_obs.buf, port_obs.buf, BUFSZ) != 0)
			fail(st, label);
	}
	if (ref_obs.errno_after != port_obs.errno_after)
		fail(st, label);
}

struct ctermid_r_obs {
	char   *ret;
	long	ret_off;
	int	errno_after;
	unsigned char buf[BUFSZ];
};

static ctermid_r_obs
run_ctermid_r_side(bool port_side, bool use_null, const mock_cfg *cfg,
    int errno_in, unsigned char *user_buf)
{
	ctermid_r_obs obs;

	mock_reset(cfg);
	errno = errno_in;
	if (use_null) {
		if (port_side)
			obs.ret = P::ctermid_r(nullptr);
		else
			obs.ret = ref_ctermid_r(nullptr);
		obs.ret_off = ptr_off(obs.ret, nullptr);
	} else {
		fill_guard(user_buf, BUFSZ);
		std::memcpy(obs.buf, user_buf, BUFSZ);
		if (port_side)
			obs.ret = P::ctermid_r((char *)user_buf);
		else
			obs.ret = ref_ctermid_r((char *)user_buf);
		std::memcpy(obs.buf, user_buf, BUFSZ);
		obs.ret_off = ptr_off(obs.ret, (const char *)user_buf);
	}
	obs.errno_after = errno;
	return (obs);
}

static void
check_ctermid_r(bool use_null, const mock_cfg *cfg, int errno_in,
    const char *label)
{
	Stat &st = S("ctermid_r");
	unsigned char ref_user[BUFSZ];
	unsigned char port_user[BUFSZ];
	ctermid_r_obs ref_obs, port_obs;

	st.cases++;
	ref_obs = run_ctermid_r_side(false, use_null, cfg, errno_in, ref_user);
	port_obs = run_ctermid_r_side(true, use_null, cfg, errno_in, port_user);

	if ((ref_obs.ret == nullptr) != (port_obs.ret == nullptr))
		fail(st, label);
	if (use_null) {
		if (ref_obs.ret != nullptr || port_obs.ret != nullptr)
			fail(st, label);
	} else {
		if (ref_obs.ret_off != port_obs.ret_off)
			fail(st, label);
		if (std::memcmp(ref_obs.buf, port_obs.buf, BUFSZ) != 0)
			fail(st, label);
	}
	if (ref_obs.errno_after != port_obs.errno_after)
		fail(st, label);
}

/* ----------------------------------------------------------- edge cases */

static void
test_ctermid_edges(void)
{
	mock_cfg cfg;
	unsigned char hi[4] = { 0x80, 0xff, 0xfe, 0x00 };
	char longname[SPECNAMELEN];

	cfg_defaults(&cfg);
	check_ctermid(true, &cfg, 5, "null stat fail errno5");

	cfg_defaults(&cfg);
	cfg_stat_ok_chr(&cfg);
	cfg.st_rdev = 0x1234;
	set_devname(&cfg, "pts/0");
	check_ctermid(true, &cfg, EIO, "null stat chr sysctl pts0");

	cfg_defaults(&cfg);
	cfg_stat_ok_chr(&cfg);
	cfg.sysctl_ret = -1;
	cfg.sysctl_errno = ENOMEM;
	check_ctermid(true, &cfg, 0, "null stat chr sysctl fail");

	cfg_defaults(&cfg);
	cfg.stat_ret = 0;
	cfg.st_mode = S_IFREG;
	set_devname(&cfg, "ignored");
	check_ctermid(true, &cfg, EINTR, "null stat not chr");

	cfg_defaults(&cfg);
	check_ctermid(false, &cfg, ENOENT, "user stat fail");

	cfg_defaults(&cfg);
	cfg_stat_ok_chr(&cfg);
	set_devname(&cfg, "console");
	check_ctermid(false, &cfg, 0, "user stat chr console");

	cfg_defaults(&cfg);
	cfg_stat_ok_chr(&cfg);
	set_devname(&cfg, "");
	check_ctermid(false, &cfg, 0, "user stat chr empty devname");

	cfg_defaults(&cfg);
	cfg_stat_ok_chr(&cfg);
	set_devname_bytes(&cfg, hi, 3);
	check_ctermid(false, &cfg, 0, "user stat chr high-bit devname");

	std::memset(longname, 'a', SPECNAMELEN - 1);
	longname[SPECNAMELEN - 1] = '\0';
	cfg_defaults(&cfg);
	cfg_stat_ok_chr(&cfg);
	set_devname(&cfg, longname);
	check_ctermid(false, &cfg, 0, "user stat chr long devname");

	cfg_defaults(&cfg);
	cfg_stat_ok_chr(&cfg);
	set_devname(&cfg, "x");
	check_ctermid(true, &cfg, EAGAIN, "null stat chr single char");

	cfg_defaults(&cfg);
	cfg.stat_ret = 0;
	cfg.st_mode = S_IFBLK;
	check_ctermid(false, &cfg, 0, "user stat block dev");

	cfg_defaults(&cfg);
	cfg_stat_ok_chr(&cfg);
	cfg.st_rdev = (dev_t)0xffffffff;
	set_devname(&cfg, "ttyU0");
	check_ctermid(false, &cfg, 0x7f, "user stat chr rdev max");

	cfg_defaults(&cfg);
	cfg.stat_errno = EACCES;
	check_ctermid(true, &cfg, 0, "null stat fail errno0");
}

static void
test_ctermid_r_edges(void)
{
	mock_cfg cfg;

	cfg_defaults(&cfg);
	check_ctermid_r(true, &cfg, 7, "null buf");

	cfg_defaults(&cfg);
	check_ctermid_r(false, &cfg, EIO, "user stat fail");

	cfg_defaults(&cfg);
	cfg_stat_ok_chr(&cfg);
	set_devname(&cfg, "pts/1");
	check_ctermid_r(false, &cfg, 0, "user stat chr");

	cfg_defaults(&cfg);
	check_ctermid_r(true, &cfg, 0, "null errno0");
}

/* -------------------------------------------------------------- random */

static void
fill_random_devname(mock_cfg *cfg, Rng &rng)
{
	std::size_t len, i;
	unsigned char b;

	len = (std::size_t)(rng.u32() % SPECNAMELEN);
	if (len >= sizeof(cfg->devname))
		len = sizeof(cfg->devname) - 1;
	for (i = 0; i < len; i++) {
		b = (unsigned char)(rng.u32() & 0xffu);
		if ((rng.u32() % 17u) == 0u)
			b = (unsigned char)(0x80u + (rng.u32() % 128u));
		cfg->devname[i] = (char)b;
	}
	cfg->devname[len] = '\0';
}

static void
test_ctermid_random(Rng &rng)
{
	Stat &st = S("ctermid");
	mock_cfg cfg;

	for (long i = 0; i < SWEEP; i++) {
		bool use_null;
		int errno_in;
		unsigned char ref_user[BUFSZ];
		unsigned char port_user[BUFSZ];
		ctermid_obs ref_obs, port_obs;

		cfg_defaults(&cfg);
		use_null = rng.coin() != 0;
		errno_in = (int)rng.u32();
		if ((rng.u32() % 8u) == 0u)
			errno_in = 0;

		switch (rng.u32() % 4u) {
		case 0:
			cfg.stat_ret = -1;
			cfg.stat_errno = (int)(rng.u32() % 40u) + 1;
			break;
		case 1:
			cfg.stat_ret = 0;
			cfg.st_mode = S_IFCHR;
			cfg.st_rdev = (dev_t)rng.u32();
			fill_random_devname(&cfg, rng);
			break;
		case 2:
			cfg.stat_ret = 0;
			cfg.st_mode = (mode_t)(S_IFREG | (rng.u32() & 0777u));
			break;
		default:
			cfg_stat_ok_chr(&cfg);
			cfg.st_rdev = (dev_t)rng.u32();
			if (rng.coin()) {
				cfg.sysctl_ret = -1;
				cfg.sysctl_errno = (int)(rng.u32() % 40u) + 1;
			}
			fill_random_devname(&cfg, rng);
			break;
		}

		st.cases++;
		ref_obs = run_ctermid_side(false, use_null, &cfg, errno_in,
		    ref_user);
		port_obs = run_ctermid_side(true, use_null, &cfg, errno_in,
		    port_user);

		if (use_null) {
			if (ref_obs.ret == nullptr || port_obs.ret == nullptr ||
			    std::memcmp(ref_obs.ret, port_obs.ret, DEFSZ) != 0)
				fail(st, "random");
		} else {
			if (ref_obs.ret_off != port_obs.ret_off ||
			    std::memcmp(ref_obs.buf, port_obs.buf, BUFSZ) != 0)
				fail(st, "random");
		}
		if (ref_obs.errno_after != port_obs.errno_after)
			fail(st, "random errno");
	}
}

static void
test_ctermid_r_random(Rng &rng)
{
	Stat &st = S("ctermid_r");
	mock_cfg cfg;

	for (long i = 0; i < SWEEP; i++) {
		bool use_null;
		int errno_in;
		unsigned char ref_user[BUFSZ];
		unsigned char port_user[BUFSZ];
		ctermid_r_obs ref_obs, port_obs;

		cfg_defaults(&cfg);
		use_null = rng.coin() != 0;
		errno_in = (int)rng.u32();

		if (!use_null) {
			switch (rng.u32() % 3u) {
			case 0:
				cfg.stat_ret = -1;
				break;
			case 1:
				cfg.stat_ret = 0;
				cfg.st_mode = S_IFIFO;
				break;
			default:
				cfg_stat_ok_chr(&cfg);
				fill_random_devname(&cfg, rng);
				break;
			}
		}

		st.cases++;
		ref_obs = run_ctermid_r_side(false, use_null, &cfg, errno_in,
		    ref_user);
		port_obs = run_ctermid_r_side(true, use_null, &cfg, errno_in,
		    port_user);

		if ((ref_obs.ret == nullptr) != (port_obs.ret == nullptr))
			fail(st, "random");
		if (use_null) {
			if (ref_obs.ret != nullptr || port_obs.ret != nullptr)
				fail(st, "random null");
		} else {
			if (ref_obs.ret_off != port_obs.ret_off ||
			    std::memcmp(ref_obs.buf, port_obs.buf, BUFSZ) != 0)
				fail(st, "random");
		}
		if (ref_obs.errno_after != port_obs.errno_after)
			fail(st, "random errno");
	}
}

/* ------------------------------------------------------------------ main */

int
main()
{
	Rng rng(0xb0311u);

	test_ctermid_edges();
	test_ctermid_r_edges();
	test_ctermid_random(rng);
	test_ctermid_r_random(rng);

	std::printf("\n%-24s %12s %12s\n", "function", "cases", "failures");
	for (int i = 0; i < NSTAT; i++)
		std::printf("%-24s %12llu %12llu\n", stats[i].name,
		    stats[i].cases, stats[i].fails);

	unsigned long long total_fail = 0;
	for (int i = 0; i < NSTAT; i++)
		total_fail += stats[i].fails;

	return (total_fail == 0 ? 0 : 1);
}
