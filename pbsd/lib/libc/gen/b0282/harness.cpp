/*
 * Differential harness for batch b0282 (initgroups, freebsd14_initgroups).
 */

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <errno.h>
#include <grp.h>
#include <sys/types.h>
#include <unistd.h>

import pbsd.lib.libc.gen.b0282;

namespace P = pbsd::lib_libc_gen::b0282;

extern "C" {
int ref_initgroups(const char *uname, gid_t agroup);
int ref_freebsd14_initgroups(const char *uname, gid_t agroup);
void *__real_malloc(size_t);
void __real_free(void *);
long __real_sysconf(int);
int __real_getgrouplist(const char *, gid_t, gid_t *, int *);
int __real_setgroups(int, const gid_t *);
}

enum {
	F_INITGROUPS,
	F_FREEBSD14_INITGROUPS,
	NFUNC
};

static const char *const fname[NFUNC] = {
	"initgroups",
	"freebsd14_initgroups"
};

static unsigned long long ncases[NFUNC];
static unsigned long long nfails[NFUNC];

static constexpr int MAXREPORT = 16;
static int nreported;

static void
report(int f, const char *ctx, const char *detail)
{
	nfails[f]++;
	if (nreported < MAXREPORT) {
		nreported++;
		std::printf("FAIL %-28s %s: %s\n", fname[f], ctx, detail);
	} else if (nreported == MAXREPORT) {
		nreported++;
		std::printf("... further failures suppressed\n");
	}
}

/* ------------------------------------------------------------------ */
/* PRNG (fixed seed)                                                  */
/* ------------------------------------------------------------------ */

static std::uint64_t rng_state = 0xb0282decadefeedULL;

static std::uint64_t
nextrand(void)
{
	std::uint64_t x = rng_state;

	x ^= x >> 12;
	x ^= x << 25;
	x ^= x >> 27;
	rng_state = x;
	return x * 0x2545f4914f6cdd1dULL;
}

static std::uint32_t
randu32(void)
{
	return (std::uint32_t)(nextrand() & 0xffffffffu);
}

/* ------------------------------------------------------------------ */
/* Mock configuration and snapshots                                   */
/* ------------------------------------------------------------------ */

static constexpr int MAX_GROUPS = 256;

struct MockCfg {
	long sysconf_ngroups_max;
	int malloc_fail;
	int gl_ret;
	int gl_ngroups_out;
	gid_t gl_groups[MAX_GROUPS];
	int sg_ret;
	int f14_ret;
};

struct MockSnap {
	int ret;
	int malloc_calls;
	size_t malloc_size;
	int free_calls;
	int gl_calls;
	gid_t gl_agroup;
	int gl_ngroups_in;
	int gl_ngroups_out;
	int gl_ret;
	gid_t gl_groups[MAX_GROUPS];
	int sg_calls;
	int sg_ngroups;
	gid_t sg_groups[MAX_GROUPS];
	int f14_calls;
	int f14_ngroups;
	gid_t f14_groups[MAX_GROUPS];
};

static MockCfg mock_cfg;
static MockSnap gl_rec;
static MockSnap sg_rec;
static MockSnap f14_rec;

static int g_malloc_fail;
static long g_sysconf_ngroups_max;
static int g_gl_ret;
static int g_gl_ngroups_out;
static gid_t g_gl_groups[MAX_GROUPS];
static int g_sg_ret;
static int g_f14_ret;

static void
mock_reset(void)
{
	std::memset(&mock_cfg, 0, sizeof mock_cfg);
	std::memset(&gl_rec, 0, sizeof gl_rec);
	std::memset(&sg_rec, 0, sizeof sg_rec);
	std::memset(&f14_rec, 0, sizeof f14_rec);

	g_malloc_fail = 0;
	g_sysconf_ngroups_max = 16;
	g_gl_ret = 0;
	g_gl_ngroups_out = 0;
	g_sg_ret = 0;
	g_f14_ret = 0;
	std::memset(g_gl_groups, 0, sizeof g_gl_groups);
}

static void
mock_apply_cfg(const MockCfg &cfg)
{
	mock_cfg = cfg;
	g_malloc_fail = cfg.malloc_fail;
	g_sysconf_ngroups_max = cfg.sysconf_ngroups_max;
	g_gl_ret = cfg.gl_ret;
	g_gl_ngroups_out = cfg.gl_ngroups_out;
	g_sg_ret = cfg.sg_ret;
	g_f14_ret = cfg.f14_ret;
	std::memcpy(g_gl_groups, cfg.gl_groups,
	    sizeof(g_gl_groups[0]) * MAX_GROUPS);
}

static void
copy_gl_groups(MockSnap &snap, const gid_t *groups, int n)
{
	int lim = n;

	if (lim > MAX_GROUPS)
		lim = MAX_GROUPS;
	for (int i = 0; i < lim; i++)
		snap.gl_groups[i] = groups[i];
}

static void
copy_sg_groups(MockSnap &snap, const gid_t *groups, int n)
{
	int lim = n;

	if (lim > MAX_GROUPS)
		lim = MAX_GROUPS;
	for (int i = 0; i < lim; i++)
		snap.sg_groups[i] = groups[i];
}

extern "C" void *
__wrap_malloc(size_t size)
{
	gl_rec.malloc_calls++;
	sg_rec.malloc_calls++;
	f14_rec.malloc_calls++;
	gl_rec.malloc_size = size;
	sg_rec.malloc_size = size;
	f14_rec.malloc_size = size;

	if (g_malloc_fail != 0) {
		errno = ENOMEM;
		return nullptr;
	}
	return __real_malloc(size);
}

extern "C" void
__wrap_free(void *ptr)
{
	gl_rec.free_calls++;
	sg_rec.free_calls++;
	f14_rec.free_calls++;
	__real_free(ptr);
}

extern "C" long
__wrap_sysconf(int name)
{
	if (name == _SC_NGROUPS_MAX)
		return g_sysconf_ngroups_max;
	return __real_sysconf(name);
}

extern "C" int
__wrap_getgrouplist(const char *uname, gid_t agroup, gid_t *groups,
    int *ngroups)
{
	MockSnap *rec = &gl_rec;

	rec->gl_calls++;
	rec->gl_agroup = agroup;
	if (ngroups != nullptr)
		rec->gl_ngroups_in = *ngroups;

	if (groups != nullptr && ngroups != nullptr) {
		int cap = *ngroups;
		int n = g_gl_ngroups_out;

		if (n > cap)
			n = cap;
		for (int i = 0; i < n; i++)
			groups[i] = g_gl_groups[i];
		*ngroups = g_gl_ngroups_out;
		copy_gl_groups(*rec, groups, n);
	}

	rec->gl_ngroups_out = g_gl_ngroups_out;
	rec->gl_ret = g_gl_ret;
	return g_gl_ret;
}

extern "C" int
__wrap_setgroups(int ngroups, const gid_t *groups)
{
	MockSnap *rec = &sg_rec;

	rec->sg_calls++;
	rec->sg_ngroups = ngroups;
	copy_sg_groups(*rec, groups, ngroups);
	return g_sg_ret;
}

extern "C" int
freebsd14_setgroups(int gidsize, const gid_t *gidset)
{
	MockSnap *rec = &f14_rec;

	rec->f14_calls++;
	rec->f14_ngroups = gidsize;
	copy_sg_groups(*rec, gidset, gidsize);
	return g_f14_ret;
}

static MockSnap
run_port_initgroups(const char *uname, gid_t agroup)
{
	MockSnap snap;

	std::memset(&gl_rec, 0, sizeof gl_rec);
	std::memset(&sg_rec, 0, sizeof sg_rec);
	std::memset(&f14_rec, 0, sizeof f14_rec);

	snap.ret = P::initgroups(uname, agroup);
	snap.malloc_calls = gl_rec.malloc_calls;
	snap.malloc_size = gl_rec.malloc_size;
	snap.free_calls = gl_rec.free_calls;
	snap.gl_calls = gl_rec.gl_calls;
	snap.gl_agroup = gl_rec.gl_agroup;
	snap.gl_ngroups_in = gl_rec.gl_ngroups_in;
	snap.gl_ngroups_out = gl_rec.gl_ngroups_out;
	snap.gl_ret = gl_rec.gl_ret;
	std::memcpy(snap.gl_groups, gl_rec.gl_groups, sizeof snap.gl_groups);
	snap.sg_calls = sg_rec.sg_calls;
	snap.sg_ngroups = sg_rec.sg_ngroups;
	std::memcpy(snap.sg_groups, sg_rec.sg_groups, sizeof snap.sg_groups);
	snap.f14_calls = f14_rec.f14_calls;
	snap.f14_ngroups = f14_rec.f14_ngroups;
	std::memcpy(snap.f14_groups, f14_rec.f14_groups, sizeof snap.f14_groups);
	return snap;
}

static MockSnap
run_ref_initgroups(const char *uname, gid_t agroup)
{
	MockSnap snap;

	std::memset(&gl_rec, 0, sizeof gl_rec);
	std::memset(&sg_rec, 0, sizeof sg_rec);
	std::memset(&f14_rec, 0, sizeof f14_rec);

	snap.ret = ref_initgroups(uname, agroup);
	snap.malloc_calls = gl_rec.malloc_calls;
	snap.malloc_size = gl_rec.malloc_size;
	snap.free_calls = gl_rec.free_calls;
	snap.gl_calls = gl_rec.gl_calls;
	snap.gl_agroup = gl_rec.gl_agroup;
	snap.gl_ngroups_in = gl_rec.gl_ngroups_in;
	snap.gl_ngroups_out = gl_rec.gl_ngroups_out;
	snap.gl_ret = gl_rec.gl_ret;
	std::memcpy(snap.gl_groups, gl_rec.gl_groups, sizeof snap.gl_groups);
	snap.sg_calls = sg_rec.sg_calls;
	snap.sg_ngroups = sg_rec.sg_ngroups;
	std::memcpy(snap.sg_groups, sg_rec.sg_groups, sizeof snap.sg_groups);
	snap.f14_calls = f14_rec.f14_calls;
	snap.f14_ngroups = f14_rec.f14_ngroups;
	std::memcpy(snap.f14_groups, f14_rec.f14_groups, sizeof snap.f14_groups);
	return snap;
}

static MockSnap
run_port_freebsd14(const char *uname, gid_t agroup)
{
	MockSnap snap;

	std::memset(&gl_rec, 0, sizeof gl_rec);
	std::memset(&sg_rec, 0, sizeof sg_rec);
	std::memset(&f14_rec, 0, sizeof f14_rec);

	snap.ret = P::freebsd14_initgroups(uname, agroup);
	snap.malloc_calls = gl_rec.malloc_calls;
	snap.malloc_size = gl_rec.malloc_size;
	snap.free_calls = gl_rec.free_calls;
	snap.gl_calls = gl_rec.gl_calls;
	snap.gl_agroup = gl_rec.gl_agroup;
	snap.gl_ngroups_in = gl_rec.gl_ngroups_in;
	snap.gl_ngroups_out = gl_rec.gl_ngroups_out;
	snap.gl_ret = gl_rec.gl_ret;
	std::memcpy(snap.gl_groups, gl_rec.gl_groups, sizeof snap.gl_groups);
	snap.sg_calls = sg_rec.sg_calls;
	snap.sg_ngroups = sg_rec.sg_ngroups;
	std::memcpy(snap.sg_groups, sg_rec.sg_groups, sizeof snap.sg_groups);
	snap.f14_calls = f14_rec.f14_calls;
	snap.f14_ngroups = f14_rec.f14_ngroups;
	std::memcpy(snap.f14_groups, f14_rec.f14_groups, sizeof snap.f14_groups);
	return snap;
}

static MockSnap
run_ref_freebsd14(const char *uname, gid_t agroup)
{
	MockSnap snap;

	std::memset(&gl_rec, 0, sizeof gl_rec);
	std::memset(&sg_rec, 0, sizeof sg_rec);
	std::memset(&f14_rec, 0, sizeof f14_rec);

	snap.ret = ref_freebsd14_initgroups(uname, agroup);
	snap.malloc_calls = gl_rec.malloc_calls;
	snap.malloc_size = gl_rec.malloc_size;
	snap.free_calls = gl_rec.free_calls;
	snap.gl_calls = gl_rec.gl_calls;
	snap.gl_agroup = gl_rec.gl_agroup;
	snap.gl_ngroups_in = gl_rec.gl_ngroups_in;
	snap.gl_ngroups_out = gl_rec.gl_ngroups_out;
	snap.gl_ret = gl_rec.gl_ret;
	std::memcpy(snap.gl_groups, gl_rec.gl_groups, sizeof snap.gl_groups);
	snap.sg_calls = sg_rec.sg_calls;
	snap.sg_ngroups = sg_rec.sg_ngroups;
	std::memcpy(snap.sg_groups, sg_rec.sg_groups, sizeof snap.sg_groups);
	snap.f14_calls = f14_rec.f14_calls;
	snap.f14_ngroups = f14_rec.f14_ngroups;
	std::memcpy(snap.f14_groups, f14_rec.f14_groups, sizeof snap.f14_groups);
	return snap;
}

static bool
snap_groups_same(const gid_t *a, const gid_t *b, int n)
{
	for (int i = 0; i < n; i++) {
		if (a[i] != b[i])
			return false;
	}
	return true;
}

static bool
snap_eq(const MockSnap &pa, const MockSnap &pb)
{
	if (pa.malloc_calls != pb.malloc_calls ||
	    pa.malloc_size != pb.malloc_size ||
	    pa.free_calls != pb.free_calls ||
	    pa.gl_calls != pb.gl_calls ||
	    pa.gl_agroup != pb.gl_agroup ||
	    pa.gl_ngroups_in != pb.gl_ngroups_in ||
	    pa.gl_ngroups_out != pb.gl_ngroups_out ||
	    pa.gl_ret != pb.gl_ret ||
	    pa.sg_calls != pb.sg_calls ||
	    pa.sg_ngroups != pb.sg_ngroups ||
	    pa.f14_calls != pb.f14_calls ||
	    pa.f14_ngroups != pb.f14_ngroups)
		return false;

	if (!snap_groups_same(pa.gl_groups, pb.gl_groups, MAX_GROUPS))
		return false;
	if (!snap_groups_same(pa.sg_groups, pb.sg_groups, pa.sg_ngroups))
		return false;
	if (!snap_groups_same(pa.f14_groups, pb.f14_groups, pa.f14_ngroups))
		return false;
	return true;
}

static size_t
expected_malloc_size(long ngroups_max)
{
	if (ngroups_max < 0)
		return 0;
	return sizeof(gid_t) * (size_t)ngroups_max;
}

static bool
compare_pair(int func, const char *ctx, const MockSnap &port,
    const MockSnap &ref, int ret_p, int ret_r, long sysconf_val)
{
	bool ok = true;
	char msg[160];
	size_t exp_malloc = expected_malloc_size(sysconf_val + 2);
	int exp_gl_in = (int)(sysconf_val + 2);

	if (ret_p != ret_r) {
		std::snprintf(msg, sizeof msg, "ret port=%d ref=%d", ret_p,
		    ret_r);
		report(func, ctx, msg);
		ok = false;
	}

	if (!snap_eq(port, ref)) {
		report(func, ctx, "mock snapshot mismatch");
		ok = false;
	}

	if (mock_cfg.malloc_fail == 0) {
		if (port.malloc_calls != 1) {
			std::snprintf(msg, sizeof msg, "malloc calls port=%d",
			    port.malloc_calls);
			report(func, ctx, msg);
			ok = false;
		}
		if (port.malloc_size != exp_malloc) {
			std::snprintf(msg, sizeof msg,
			    "malloc size port=%zu exp=%zu", port.malloc_size,
			    exp_malloc);
			report(func, ctx, msg);
			ok = false;
		}
		if (port.free_calls != 1) {
			std::snprintf(msg, sizeof msg, "free calls port=%d",
			    port.free_calls);
			report(func, ctx, msg);
			ok = false;
		}
		if (port.gl_calls != 1) {
			std::snprintf(msg, sizeof msg, "getgrouplist calls=%d",
			    port.gl_calls);
			report(func, ctx, msg);
			ok = false;
		}
		if (port.gl_ngroups_in != exp_gl_in) {
			std::snprintf(msg, sizeof msg,
			    "gl ngroups_in port=%d exp=%d",
			    port.gl_ngroups_in, exp_gl_in);
			report(func, ctx, msg);
			ok = false;
		}
	} else {
		if (port.malloc_calls != 1 || port.gl_calls != 0 ||
		    port.sg_calls != 0 || port.f14_calls != 0 ||
		    port.free_calls != 0) {
			report(func, ctx, "malloc-fail side effects");
			ok = false;
		}
		if (ret_p != -1) {
			report(func, ctx, "malloc-fail ret not -1");
			ok = false;
		}
	}

	if (func == F_INITGROUPS && mock_cfg.malloc_fail == 0) {
		if (port.sg_calls != 1 || port.f14_calls != 0) {
			report(func, ctx, "setgroups path mismatch");
			ok = false;
		}
		if (port.sg_ngroups != mock_cfg.gl_ngroups_out) {
			std::snprintf(msg, sizeof msg,
			    "sg ngroups port=%d exp=%d", port.sg_ngroups,
			    mock_cfg.gl_ngroups_out);
			report(func, ctx, msg);
			ok = false;
		}
	}

	if (func == F_FREEBSD14_INITGROUPS && mock_cfg.malloc_fail == 0) {
		if (port.f14_calls != 1 || port.sg_calls != 0) {
			report(func, ctx, "freebsd14_setgroups path mismatch");
			ok = false;
		}
		if (port.f14_ngroups != mock_cfg.gl_ngroups_out) {
			std::snprintf(msg, sizeof msg,
			    "f14 ngroups port=%d exp=%d", port.f14_ngroups,
			    mock_cfg.gl_ngroups_out);
			report(func, ctx, msg);
			ok = false;
		}
	}

	return ok;
}

static void
test_initgroups_case(const char *uname, gid_t agroup, const MockCfg &cfg,
    const char *ctx)
{
	MockSnap port_snap, ref_snap;
	int ret_p, ret_r;

	mock_apply_cfg(cfg);
	port_snap = run_port_initgroups(uname, agroup);
	ret_p = port_snap.ret;

	mock_apply_cfg(cfg);
	ref_snap = run_ref_initgroups(uname, agroup);
	ret_r = ref_snap.ret;

	ncases[F_INITGROUPS]++;
	compare_pair(F_INITGROUPS, ctx, port_snap, ref_snap, ret_p, ret_r,
	    cfg.sysconf_ngroups_max);
}

static void
test_freebsd14_case(const char *uname, gid_t agroup, const MockCfg &cfg,
    const char *ctx)
{
	MockSnap port_snap, ref_snap;
	int ret_p, ret_r;

	mock_apply_cfg(cfg);
	port_snap = run_port_freebsd14(uname, agroup);
	ret_p = port_snap.ret;

	mock_apply_cfg(cfg);
	ref_snap = run_ref_freebsd14(uname, agroup);
	ret_r = ref_snap.ret;

	ncases[F_FREEBSD14_INITGROUPS]++;
	compare_pair(F_FREEBSD14_INITGROUPS, ctx, port_snap, ref_snap, ret_p,
	    ret_r, cfg.sysconf_ngroups_max);
}

static void
fill_cfg_groups(MockCfg &cfg, int n)
{
	cfg.gl_ngroups_out = n;
	for (int i = 0; i < n && i < MAX_GROUPS; i++)
		cfg.gl_groups[i] = (gid_t)(1000u + (unsigned)i * 17u);
}

static void
test_edges(void)
{
	MockCfg cfg;

	mock_reset();

	{
		cfg.sysconf_ngroups_max = 0;
		cfg.malloc_fail = 1;
		test_initgroups_case("user", 0, cfg, "malloc fail initgroups");
		test_freebsd14_case("user", 0, cfg, "malloc fail fbsd14");
	}

	{
		cfg.sysconf_ngroups_max = -1;
		cfg.malloc_fail = 0;
		cfg.sg_ret = -1;
		cfg.f14_ret = -1;
		fill_cfg_groups(cfg, 0);
		test_initgroups_case("", 0, cfg, "sysconf -1 empty uname");
		test_freebsd14_case("", 0, cfg, "sysconf -1 fbsd14 empty");
	}

	{
		cfg.sysconf_ngroups_max = 0;
		cfg.malloc_fail = 0;
		cfg.sg_ret = 0;
		cfg.f14_ret = 0;
		fill_cfg_groups(cfg, 0);
		test_initgroups_case(nullptr, 0, cfg, "null uname");
		test_freebsd14_case(nullptr, 0, cfg, "null uname fbsd14");
	}

	{
		cfg.sysconf_ngroups_max = 1;
		cfg.sg_ret = 7;
		cfg.f14_ret = 8;
		fill_cfg_groups(cfg, 1);
		test_initgroups_case("a", 1, cfg, "single char uname");
		test_freebsd14_case("a", 1, cfg, "single char fbsd14");
	}

	{
		cfg.sysconf_ngroups_max = 2;
		cfg.sg_ret = -1;
		cfg.f14_ret = -1;
		fill_cfg_groups(cfg, 3);
		test_initgroups_case("foo", 42, cfg, "ngroups_out > max+2");
		test_freebsd14_case("foo", 42, cfg, "ngroups_out > max+2 fbsd14");
	}

	{
		static const unsigned char hb[] = {
			0x80, 0xff, 0xfe, 0x81, '\0'
		};
		char uname[16];

		std::memcpy(uname, hb, sizeof hb);
		cfg.sysconf_ngroups_max = 8;
		cfg.sg_ret = 0;
		cfg.f14_ret = 0;
		fill_cfg_groups(cfg, 4);
		test_initgroups_case(uname, (gid_t)0xff00u, cfg, "high-bit uname");
		test_freebsd14_case(uname, (gid_t)0xff00u, cfg,
		    "high-bit uname fbsd14");
	}

	{
		static const unsigned char nulheavy[] = {
			'a', '\0', 'b', '\0', 'c', 0x80, '\0'
		};
		char uname[16];

		std::memcpy(uname, nulheavy, sizeof nulheavy);
		cfg.sysconf_ngroups_max = 4;
		cfg.sg_ret = 0;
		cfg.f14_ret = 0;
		fill_cfg_groups(cfg, 2);
		test_initgroups_case(uname, 99, cfg, "NUL-heavy uname");
		test_freebsd14_case(uname, 99, cfg, "NUL-heavy fbsd14");
	}

	{
		cfg.sysconf_ngroups_max = 16;
		cfg.sg_ret = 0;
		cfg.f14_ret = 0;
		cfg.gl_ret = -1;
		fill_cfg_groups(cfg, 5);
		test_initgroups_case("boundary", (gid_t)65535u, cfg,
		    "gl_ret -1 boundary gid");
		test_freebsd14_case("boundary", (gid_t)65535u, cfg,
		    "gl_ret -1 fbsd14");
	}

	{
		cfg.sysconf_ngroups_max = 32;
		cfg.sg_ret = 0;
		cfg.f14_ret = 0;
		fill_cfg_groups(cfg, 34);
		test_initgroups_case("longgroups", 5, cfg, "many groups");
		test_freebsd14_case("longgroups", 5, cfg, "many groups fbsd14");
	}

	{
		cfg.sysconf_ngroups_max = 3;
		cfg.sg_ret = 0;
		cfg.f14_ret = 0;
		fill_cfg_groups(cfg, 0);
		cfg.gl_groups[0] = (gid_t)0x80000000u;
		test_initgroups_case("gid0", 0, cfg, "zero ngroups high gid");
		test_freebsd14_case("gid0", 0, cfg, "zero ngroups fbsd14");
	}
}

static void
fill_random_uname(char *buf, size_t bufsz, unsigned len)
{
	size_t n = len;

	if (n >= bufsz)
		n = bufsz - 1;
	for (size_t i = 0; i < n; i++)
		buf[i] = (char)(randu32() & 0xffu);
	buf[n] = '\0';
}

static void
test_random_initgroups(void)
{
	char uname[96];

	for (int i = 0; i < 200000; i++) {
		MockCfg cfg;
		char ctx[48];
		gid_t agroup;

		std::snprintf(ctx, sizeof ctx, "rand %d", i);
		std::memset(&cfg, 0, sizeof cfg);

		cfg.sysconf_ngroups_max = (long)(randu32() % 65u) - 1L;
		cfg.malloc_fail = (randu32() % 37u == 0u) ? 1 : 0;
		cfg.gl_ret = (randu32() % 8u == 0u) ? -1 : 0;
		cfg.sg_ret = (int)(randu32() % 5u) - 2;
		cfg.f14_ret = (int)(randu32() % 5u) - 2;

		long maxgl = cfg.sysconf_ngroups_max + 2;
		if (maxgl < 0)
			maxgl = 0;
		if (maxgl > MAX_GROUPS)
			maxgl = MAX_GROUPS;
		cfg.gl_ngroups_out = (int)(randu32() % (unsigned)(maxgl + 4u));
		for (int g = 0; g < cfg.gl_ngroups_out && g < MAX_GROUPS; g++)
			cfg.gl_groups[g] = (gid_t)randu32();

		agroup = (gid_t)randu32();
		fill_random_uname(uname, sizeof uname,
		    (unsigned)(randu32() % 32u));

		test_initgroups_case(uname, agroup, cfg, ctx);
	}
}

static void
test_random_freebsd14(void)
{
	char uname[96];

	for (int i = 0; i < 200000; i++) {
		MockCfg cfg;
		char ctx[48];
		gid_t agroup;

		std::snprintf(ctx, sizeof ctx, "rand %d", i);
		std::memset(&cfg, 0, sizeof cfg);

		cfg.sysconf_ngroups_max = (long)(randu32() % 65u) - 1L;
		cfg.malloc_fail = (randu32() % 37u == 0u) ? 1 : 0;
		cfg.gl_ret = (randu32() % 8u == 0u) ? -1 : 0;
		cfg.sg_ret = (int)(randu32() % 5u) - 2;
		cfg.f14_ret = (int)(randu32() % 5u) - 2;

		long maxgl = cfg.sysconf_ngroups_max + 2;
		if (maxgl < 0)
			maxgl = 0;
		if (maxgl > MAX_GROUPS)
			maxgl = MAX_GROUPS;
		cfg.gl_ngroups_out = (int)(randu32() % (unsigned)(maxgl + 4u));
		for (int g = 0; g < cfg.gl_ngroups_out && g < MAX_GROUPS; g++)
			cfg.gl_groups[g] = (gid_t)randu32();

		agroup = (gid_t)randu32();
		fill_random_uname(uname, sizeof uname,
		    (unsigned)(randu32() % 32u));

		test_freebsd14_case(uname, agroup, cfg, ctx);
	}
}

int
main(void)
{
	unsigned long long total_cases = 0;
	unsigned long long total_fails = 0;
	int exit_code = 0;

	mock_reset();
	test_edges();
	test_random_initgroups();
	test_random_freebsd14();

	std::printf("\n%-28s %10s %10s\n", "Function", "Cases", "Failures");
	for (int f = 0; f < NFUNC; f++) {
		std::printf("%-28s %10llu %10llu\n",
		    fname[f], ncases[f], nfails[f]);
		total_cases += ncases[f];
		total_fails += nfails[f];
	}
	std::printf("%-28s %10llu %10llu\n", "TOTAL", total_cases, total_fails);

	if (total_fails != 0)
		exit_code = 1;
	return exit_code;
}
