// Differential test harness for PBSD batch b0209.

import pbsd.sys.compat.linuxkpi.common.src.b0209;

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace P = pbsd::sys_compat_linuxkpi_common_src::b0209;

#define SWEEP 200000L
#define MAX_SHOW 8
#define MOCK_RELEASE_LOG 32

struct Stat {
	const char *name;
	unsigned long long cases;
	unsigned long long fails;
	int shown;
};

static Stat st_domain = { "linux_get_vm_domain_set", 0, 0, 0 };
static Stat st_devnode = { "linux_dev_to_node", 0, 0, 0 };
static Stat st_folio_alloc = { "folio_alloc", 0, 0, 0 };
static Stat st_folio_rel = { "__folio_batch_release", 0, 0, 0 };
static Stat st_fdget = { "lkpi_eventfd_ctx_fdget", 0, 0, 0 };
static Stat st_fdput = { "lkpi_eventfd_ctx_put", 0, 0, 0 };
static Stat st_video = { "video_get_options", 0, 0, 0 };

extern "C" {
struct domainset {
	int ds_kind;
	int ds_node;
};
struct device;
typedef struct device *device_t;
struct device {
	device_t bsddev;
};
struct page {
	unsigned long pg_cookie;
};
struct folio;
struct folio_batch {
	std::uint8_t nr;
	struct folio *folios[15];
};
struct file {
	int f_id;
};
struct eventfd_ctx {
	int efd_id;
};

struct domainset *ref_linux_get_vm_domain_set(int node);
int ref_linux_dev_to_node(struct device *dev);
struct folio *ref_folio_alloc(unsigned long gfp, unsigned int order);
void ref___folio_batch_release(struct folio_batch *fbatch);
struct eventfd_ctx *ref_lkpi_eventfd_ctx_fdget(int fd);
void ref_lkpi_eventfd_ctx_put(struct eventfd_ctx *ctx);
const char *ref_video_get_options(const char *connector_name);
void mock_reset_b0209(void);
void mock_set_env(const char *name, const char *value);

extern int bootverbose;
extern int mock_bus_get_domain_ret;
extern int mock_bus_get_domain_val;
extern struct page *mock_alloc_pages_ret;
extern unsigned long mock_alloc_pages_last_gfp;
extern unsigned int mock_alloc_pages_last_order;
extern int mock_alloc_pages_calls;
extern int mock_release_log_n;
extern int mock_release_counts[MOCK_RELEASE_LOG];
extern struct folio *mock_release_folios[MOCK_RELEASE_LOG][15];
extern int mock_fget_ret;
extern struct file *mock_fget_fp;
extern int mock_fget_last_fd;
extern int mock_fget_calls;
extern struct eventfd_ctx *mock_eventfd_get_ret;
extern int mock_eventfd_get_last_fp;
extern int mock_eventfd_get_calls;
extern int mock_fdrop_calls;
extern struct file *mock_fdrop_last_fp;
extern int mock_eventfd_put_calls;
extern struct eventfd_ctx *mock_eventfd_put_last_ctx;
extern int mock_kern_getenv_calls;
extern char mock_kern_getenv_last[80];
}

struct Obs {
	int ds_kind;
	int ds_node;
	int ret;
	unsigned long pg_cookie;
	unsigned long gfp;
	unsigned int order;
	int alloc_calls;
	int release_log_n;
	int release_count;
	unsigned long release_cookie[15];
	std::uint8_t batch_nr;
	bool is_err;
	long err;
	int efd_id;
	int fget_calls;
	int fget_fd;
	int eventfd_get_calls;
	int eventfd_get_fp;
	int fdrop_calls;
	int eventfd_put_calls;
	int kern_getenv_calls;
	char kern_getenv_last[80];
	const char *opt;
	int printf_calls;
};

static int g_wrap_printf_calls;
static int g_wrap_track; /* 0=off, 1=port, -1=ref */

extern "C" int __wrap_printf(const char *fmt, ...)
{
	(void)fmt;
	if (g_wrap_track == 0)
		return (0);
	g_wrap_printf_calls++;
	return (0);
}

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
	std::uint32_t below(std::uint32_t n) { return (std::uint32_t)(next() % n); }
	bool coin() { return (next() & 1) != 0; }
};

static Rng rng(0x00b0209faceULL);

static bool
fail(Stat &st, const char *what)
{
	st.fails++;
	if (st.shown < MAX_SHOW) {
		st.shown++;
		std::fprintf(stderr, "  FAIL %s: %s\n", st.name, what);
	}
	return false;
}

static void
reset_both()
{
	mock_reset_b0209();
	P::stub_reset();
	P::bootverbose = 0;
	bootverbose = 0;
}

static void
sync_mocks_from_port()
{
	mock_bus_get_domain_ret = 0;
	mock_bus_get_domain_val = 0;
	mock_alloc_pages_ret = nullptr;
	mock_fget_ret = 0;
	mock_fget_fp = nullptr;
	mock_eventfd_get_ret = nullptr;
	bootverbose = P::bootverbose;
}

static void
apply_bus(int ret, int val)
{
	mock_bus_get_domain_ret = ret;
	mock_bus_get_domain_val = val;
	P::stub_bus_get_domain(ret, val);
}

static void
apply_alloc(P::page *p)
{
	mock_alloc_pages_ret = reinterpret_cast<struct page *>(p);
	P::stub_alloc_pages_ret(p);
}

static void
apply_fget(int ret, P::file *fp)
{
	mock_fget_ret = ret;
	mock_fget_fp = reinterpret_cast<struct file *>(fp);
	P::stub_fget(ret, fp);
}

static void
apply_eventfd_get(P::eventfd_ctx *ctx)
{
	mock_eventfd_get_ret = reinterpret_cast<struct eventfd_ctx *>(ctx);
	P::stub_eventfd_get_ret(ctx);
}

static void
apply_env(const char *name, const char *value)
{
	mock_set_env(name, value);
	P::stub_set_env(name, value);
}

static void
apply_bootverbose(int v)
{
	bootverbose = v;
	P::stub_bootverbose(v);
}

static int
ds_kind(const P::domainset *ds)
{
	return ds != nullptr ? ds->ds_kind : -1;
}

static int
ds_node(const P::domainset *ds)
{
	return ds != nullptr ? ds->ds_node : -999;
}

static bool
is_err_ptr(const void *p)
{
	return (reinterpret_cast<std::uintptr_t>(p) >=
	    static_cast<std::uintptr_t>(-4095));
}

static long
ptr_err(const void *p)
{
	return (static_cast<long>(reinterpret_cast<intptr_t>(p)));
}

static Obs
capture_domain_ref(int node, P::domainset *ds)
{
	Obs o{};
	o.ds_kind = ds != nullptr ? ds->ds_kind : -1;
	o.ds_node = ds != nullptr ? ds->ds_node : -999;
	(void)node;
	return o;
}

static Obs
capture_domain_port(int node, P::domainset *ds)
{
	Obs o{};
	o.ds_kind = ds_kind(ds);
	o.ds_node = ds_node(ds);
	(void)node;
	return o;
}

static bool
cmp_domain(Stat &st, const Obs &a, const Obs &b)
{
	if (a.ds_kind != b.ds_kind || a.ds_node != b.ds_node)
		return fail(st, "domainset kind/node");
	return true;
}

static bool
run_domain_case(int node)
{
	st_domain.cases++;
	reset_both();
	P::domainset *pr = reinterpret_cast<P::domainset *>(ref_linux_get_vm_domain_set(node));
	P::domainset *pp = P::linux_get_vm_domain_set(node);
	Obs orc = capture_domain_ref(node, reinterpret_cast<P::domainset *>(pr));
	Obs opc = capture_domain_port(node, pp);
	return cmp_domain(st_domain, orc, opc);
}

static bool
run_devnode_case(bool null_dev, bool null_bsd, int bus_ret, int bus_val)
{
	st_devnode.cases++;
	reset_both();
	apply_bus(bus_ret, bus_val);

	struct device dev_r{};
	P::device dev_p{};
	device_t bsd_r = reinterpret_cast<device_t>(&dev_r);
	device_t bsd_p = reinterpret_cast<device_t>(&dev_p);

	dev_r.bsddev = null_bsd ? nullptr : bsd_r;
	dev_p.bsddev = null_bsd ? nullptr : reinterpret_cast<P::device_t>(bsd_p);

	int rr = ref_linux_dev_to_node(null_dev ? nullptr : &dev_r);
	int rp = P::linux_dev_to_node(null_dev ? nullptr : &dev_p);
	if (rr != rp)
		return fail(st_devnode, "return value");
	return true;
}

static bool
run_folio_alloc_case(unsigned long gfp, unsigned int order, unsigned long cookie)
{
	st_folio_alloc.cases++;
	reset_both();

	static P::page pg{};
	pg.pg_cookie = cookie;
	apply_alloc(&pg);

	struct folio *fr = ref_folio_alloc(gfp, order);
	P::folio *fp = P::folio_alloc(static_cast<P::gfp_t>(gfp), order);

	unsigned long cr = fr != nullptr ?
	    reinterpret_cast<P::page *>(fr)->pg_cookie : 0;
	unsigned long cp = fp != nullptr ?
	    reinterpret_cast<P::page *>(fp)->pg_cookie : 0;
	if (cr != cp)
		return fail(st_folio_alloc, "folio pointer/value");
	if (mock_alloc_pages_calls != P::alloc_pages_calls())
		return fail(st_folio_alloc, "alloc_pages calls");
	if (mock_alloc_pages_last_gfp != P::alloc_pages_last_gfp())
		return fail(st_folio_alloc, "alloc_pages gfp");
	if (mock_alloc_pages_last_order != P::alloc_pages_last_order())
		return fail(st_folio_alloc, "alloc_pages order");
	return true;
}

static bool
run_folio_release_case(std::uint8_t nr)
{
	st_folio_rel.cases++;
	reset_both();

	static P::page pages[15];
	static P::folio_batch br{};
	static struct folio_batch bp{};
	P::folio_batch bp2{};

	for (int i = 0; i < 15; i++) {
		pages[i].pg_cookie = 0xB0209000u + (unsigned long)i;
		br.folios[i] = reinterpret_cast<P::folio *>(&pages[i]);
		bp.folios[i] = reinterpret_cast<struct folio *>(&pages[i]);
	}
	br.nr = nr;
	bp.nr = nr;
	bp2.nr = nr;
	for (int i = 0; i < (int)nr; i++)
		bp2.folios[i] = reinterpret_cast<P::folio *>(&pages[i]);

	ref___folio_batch_release(reinterpret_cast<struct folio_batch *>(&br));
	P::__folio_batch_release(&bp2);

	if (br.nr != bp2.nr)
		return fail(st_folio_rel, "batch nr after release");
	if (mock_release_log_n != P::release_log_n())
		return fail(st_folio_rel, "release log count");
	if (mock_release_log_n > 0) {
		if (mock_release_counts[0] != P::release_count(0))
			return fail(st_folio_rel, "release count");
		for (int i = 0; i < (int)nr && i < 15; i++) {
			unsigned long cr = mock_release_folios[0][i] != nullptr ?
			    reinterpret_cast<P::page *>(mock_release_folios[0][i])->pg_cookie : 0;
			unsigned long cp = P::release_folio(0, i) != nullptr ?
			    reinterpret_cast<P::page *>(P::release_folio(0, i))->pg_cookie : 0;
			if (cr != cp)
				return fail(st_folio_rel, "released folio");
		}
	}
	return true;
}

static bool
run_fdget_case(int fd, int fget_ret, int fp_id, bool ctx_null)
{
	st_fdget.cases++;
	reset_both();

	static P::file fp{};
	static P::eventfd_ctx ctx{};
	fp.f_id = fp_id;
	ctx.efd_id = 42;
	apply_fget(fget_ret, fget_ret == 0 ? &fp : nullptr);
	apply_eventfd_get(ctx_null ? nullptr : &ctx);

	struct eventfd_ctx *cr = ref_lkpi_eventfd_ctx_fdget(fd);
	P::eventfd_ctx *cp = P::lkpi_eventfd_ctx_fdget(fd);

	bool er = is_err_ptr(cr);
	bool ep = is_err_ptr(cp);
	if (er != ep)
		return fail(st_fdget, "IS_ERR mismatch");
	if (er) {
		if (ptr_err(cr) != ptr_err(cp))
			return fail(st_fdget, "ERR value");
	} else {
		if (cr == nullptr || cp == nullptr ||
		    reinterpret_cast<P::eventfd_ctx *>(cr)->efd_id != cp->efd_id)
			return fail(st_fdget, "ctx id");
	}
	if (mock_fget_calls != P::fget_calls() || mock_fget_last_fd != P::fget_last_fd())
		return fail(st_fdget, "fget");
	if (mock_eventfd_get_calls != P::eventfd_get_calls())
		return fail(st_fdget, "eventfd_get calls");
	if (fget_ret == 0 && mock_eventfd_get_last_fp != P::eventfd_get_last_fp())
		return fail(st_fdget, "eventfd_get fp");
	if (fget_ret == 0 && mock_fdrop_calls != P::fdrop_calls())
		return fail(st_fdget, "fdrop");
	return true;
}

static bool
run_fdput_case(int ctx_id)
{
	st_fdput.cases++;
	reset_both();

	static P::eventfd_ctx ctx_r{};
	static P::eventfd_ctx ctx_p{};
	ctx_r.efd_id = ctx_id;
	ctx_p.efd_id = ctx_id;

	ref_lkpi_eventfd_ctx_put(reinterpret_cast<struct eventfd_ctx *>(&ctx_r));
	P::lkpi_eventfd_ctx_put(&ctx_p);

	if (mock_eventfd_put_calls != P::eventfd_put_calls())
		return fail(st_fdput, "eventfd_put calls");
	if (mock_eventfd_put_last_ctx != nullptr && P::eventfd_put_calls() > 0) {
		if (reinterpret_cast<P::eventfd_ctx *>(mock_eventfd_put_last_ctx)->efd_id != ctx_id)
			return fail(st_fdput, "eventfd_put ctx");
	}
	return true;
}

static bool
run_video_case(const char *name, int verbose, const char *mode_env,
    const char *default_env)
{
	st_video.cases++;
	reset_both();
	apply_bootverbose(verbose);

	char tun[80];
	std::snprintf(tun, sizeof(tun), "kern.vt.fb.modes.%s", name);
	if (mode_env != nullptr)
		apply_env(tun, mode_env);
	if (default_env != nullptr)
		apply_env("kern.vt.fb.default_mode", default_env);

	g_wrap_track = -1;
	g_wrap_printf_calls = 0;
	const char *ref_opt = ref_video_get_options(name);
	int ref_printf = g_wrap_printf_calls;

	g_wrap_track = 1;
	g_wrap_printf_calls = 0;
	const char *port_opt = P::video_get_options(name);
	int port_printf = g_wrap_printf_calls;
	g_wrap_track = 0;

	const char *er = ref_opt;
	const char *pr = port_opt;
	if (er == nullptr) er = "";
	if (pr == nullptr) pr = "";
	if (std::strcmp(er, pr) != 0)
		return fail(st_video, "options string");
	if (ref_printf != port_printf)
		return fail(st_video, "printf calls");
	if (mock_kern_getenv_calls != P::kern_getenv_calls())
		return fail(st_video, "kern_getenv calls");
	if (std::strcmp(mock_kern_getenv_last, P::kern_getenv_last()) != 0)
		return fail(st_video, "kern_getenv last");
	return true;
}

static void
test_domain_hand()
{
	run_domain_case(-1);
	run_domain_case(0);
	run_domain_case(3);
	run_domain_case(15);
}

static void
test_domain_sweep()
{
	for (long i = 0; i < SWEEP; i++) {
		int node = (int)(rng.next() % 32) - 8;
		run_domain_case(node);
	}
}

static void
test_devnode_hand()
{
	run_devnode_case(true, false, 0, 0);
	run_devnode_case(false, true, 0, 0);
	run_devnode_case(false, false, -1, 0);
	run_devnode_case(false, false, 0, 7);
}

static void
test_devnode_sweep()
{
	for (long i = 0; i < SWEEP; i++) {
		bool null_dev = rng.coin() && (rng.below(8) == 0);
		bool null_bsd = !null_dev && rng.coin();
		int ret = rng.coin() ? (int)(rng.next() & 1) : 0;
		int val = (int)(rng.next() % 32);
		run_devnode_case(null_dev, null_bsd, ret, val);
	}
}

static void
test_folio_alloc_hand()
{
	run_folio_alloc_case(0, 0, 0);
	run_folio_alloc_case(0x10, 3, 0xdead);
	run_folio_alloc_case(0xff, 0, 0);
}

static void
test_folio_alloc_sweep()
{
	for (long i = 0; i < SWEEP; i++) {
		unsigned long gfp = rng.next() & 0xffff;
		unsigned int order = (unsigned int)(rng.next() % 8);
		unsigned long cookie = rng.next();
		run_folio_alloc_case(gfp, order, cookie);
	}
}

static void
test_folio_release_hand()
{
	run_folio_release_case(0);
	run_folio_release_case(1);
	run_folio_release_case(15);
}

static void
test_folio_release_sweep()
{
	for (long i = 0; i < SWEEP; i++) {
		std::uint8_t nr = (std::uint8_t)(rng.next() % 16);
		run_folio_release_case(nr);
	}
}

static void
test_fdget_hand()
{
	run_fdget_case(3, -1, 0, false);
	run_fdget_case(4, 0, 9, true);
	run_fdget_case(5, 0, 2, false);
}

static void
test_fdget_sweep()
{
	for (long i = 0; i < SWEEP; i++) {
		int fd = (int)(rng.next() % 64);
		int fget_ret = rng.coin() ? -1 : 0;
		int fp_id = (int)(rng.next() % 1000);
		bool ctx_null = rng.coin();
		run_fdget_case(fd, fget_ret, fp_id, ctx_null);
	}
}

static void
test_fdput_hand()
{
	run_fdput_case(1);
	run_fdput_case(99);
}

static void
test_fdput_sweep()
{
	for (long i = 0; i < SWEEP; i++)
		run_fdput_case((int)(rng.next() % 500));
}

static const char *names[] = {
	"LVDS", "HDMI-A", "DP-1", "", "X", "connector_7"
};

static void
test_video_hand()
{
	run_video_case("LVDS", 0, "1024x768", nullptr);
	run_video_case("HDMI-A", 1, nullptr, "640x480");
	run_video_case("DP-1", 1, "1920x1080", "640x480");
	run_video_case("", 0, nullptr, nullptr);
}

static void
test_video_sweep()
{
	char buf[32];
	for (long i = 0; i < SWEEP; i++) {
		unsigned pick = rng.below(6);
		const char *name = names[pick];
		if (pick == 4) {
			std::snprintf(buf, sizeof(buf), "C%lu", (unsigned long)i % 1000);
			name = buf;
		}
		int verbose = (int)(rng.next() & 1);
		const char *mode = nullptr;
		const char *def = nullptr;
		char mode_buf[24];
		char def_buf[24];
		if (rng.coin()) {
			std::snprintf(mode_buf, sizeof(mode_buf), "%ux%u",
			    (unsigned)(rng.next() % 4096), (unsigned)(rng.next() % 4096));
			mode = mode_buf;
		}
		if (rng.coin()) {
			std::snprintf(def_buf, sizeof(def_buf), "%ux%u",
			    (unsigned)(rng.next() % 2048), (unsigned)(rng.next() % 2048));
			def = def_buf;
		}
		run_video_case(name, verbose, mode, def);
	}
}

int
main()
{
	reset_both();

	test_domain_hand();
	test_domain_sweep();
	test_devnode_hand();
	test_devnode_sweep();
	test_folio_alloc_hand();
	test_folio_alloc_sweep();
	test_folio_release_hand();
	test_folio_release_sweep();
	test_fdget_hand();
	test_fdget_sweep();
	test_fdput_hand();
	test_fdput_sweep();
	test_video_hand();
	test_video_sweep();

	std::fprintf(stderr, "\n%-28s %12s %12s\n", "function", "cases", "failures");
	std::fprintf(stderr, "%-28s %12llu %12llu\n", st_domain.name, st_domain.cases, st_domain.fails);
	std::fprintf(stderr, "%-28s %12llu %12llu\n", st_devnode.name, st_devnode.cases, st_devnode.fails);
	std::fprintf(stderr, "%-28s %12llu %12llu\n", st_folio_alloc.name, st_folio_alloc.cases, st_folio_alloc.fails);
	std::fprintf(stderr, "%-28s %12llu %12llu\n", st_folio_rel.name, st_folio_rel.cases, st_folio_rel.fails);
	std::fprintf(stderr, "%-28s %12llu %12llu\n", st_fdget.name, st_fdget.cases, st_fdget.fails);
	std::fprintf(stderr, "%-28s %12llu %12llu\n", st_fdput.name, st_fdput.cases, st_fdput.fails);
	std::fprintf(stderr, "%-28s %12llu %12llu\n", st_video.name, st_video.cases, st_video.fails);

	unsigned long long total_fails = st_domain.fails + st_devnode.fails +
	    st_folio_alloc.fails + st_folio_rel.fails + st_fdget.fails +
	    st_fdput.fails + st_video.fails;
	return (total_fails == 0) ? 0 : 1;
}
