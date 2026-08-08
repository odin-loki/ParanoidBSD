// PBSD port of HardenedBSD linuxkpi batch b0209.

module;

#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

export module pbsd.sys.compat.linuxkpi.common.src.b0209;

export namespace pbsd::sys_compat_linuxkpi_common_src::b0209 {

constexpr int MAXMEMDOM = 16;
constexpr int EBADF = 9;

struct domainset {
	int ds_kind;
	int ds_node;
};

struct device;
using device_t = device *;

struct device {
	device_t bsddev;
};

struct page {
	unsigned long pg_cookie;
};

struct folio;

struct folio_batch {
	std::uint8_t nr;
	folio *folios[15];
};

struct thread {
	int dummy;
};

struct cap_rights {
	int dummy;
};

struct file {
	int f_id;
};

struct eventfd_ctx {
	int efd_id;
};

using gfp_t = std::uintptr_t;

constexpr gfp_t __GFP_COMP = 0;

} // namespace pbsd::sys_compat_linuxkpi_common_src::b0209

namespace pbsd::sys_compat_linuxkpi_common_src::b0209::detail {

using namespace pbsd::sys_compat_linuxkpi_common_src::b0209;

#define KASSERT(cond, msg) do { (void)(cond); (void)(msg); } while (0)

inline thread *g_curthread;
inline const cap_rights g_cap_no_rights{};

inline int g_bootverbose;

inline domainset g_ds_rr;
inline domainset g_ds_pref[MAXMEMDOM];

inline int g_bus_get_domain_ret;
inline int g_bus_get_domain_val;

inline page *g_alloc_pages_ret;
inline gfp_t g_alloc_pages_last_gfp;
inline unsigned int g_alloc_pages_last_order;
inline int g_alloc_pages_calls;

constexpr int RELEASE_LOG = 32;
inline folio *g_release_folios[RELEASE_LOG][15];
inline int g_release_counts[RELEASE_LOG];
inline int g_release_log_n;

inline int g_fget_ret;
inline file *g_fget_fp;
inline int g_fget_last_fd;
inline int g_fget_calls;

inline eventfd_ctx *g_eventfd_get_ret;
inline int g_eventfd_get_last_fp;
inline int g_eventfd_get_calls;

inline int g_fdrop_calls;
inline file *g_fdrop_last_fp;

inline int g_eventfd_put_calls;
inline eventfd_ctx *g_eventfd_put_last_ctx;

constexpr int ENV_SLOTS = 64;
inline char g_env_names[ENV_SLOTS][80];
inline const char *g_env_values[ENV_SLOTS];
inline int g_env_count;

inline int g_kern_getenv_calls;
inline char g_kern_getenv_last[80];

inline void *
ERR_PTR(long error)
{
	return (reinterpret_cast<void *>(static_cast<intptr_t>(error)));
}

inline bool
IS_ERR(const void *ptr)
{
	return (reinterpret_cast<std::uintptr_t>(ptr) >=
	    static_cast<std::uintptr_t>(-4095));
}

inline long
PTR_ERR(const void *ptr)
{
	return (static_cast<intptr_t>(reinterpret_cast<std::uintptr_t>(ptr)));
}

inline unsigned int
folio_batch_count(folio_batch *fbatch)
{
	return (fbatch->nr);
}

inline void
folio_batch_reinit(folio_batch *fbatch)
{
	fbatch->nr = 0;
}

inline domainset *
DOMAINSET_RR()
{
	g_ds_rr.ds_kind = 0;
	g_ds_rr.ds_node = -1;
	return (&g_ds_rr);
}

inline domainset *
DOMAINSET_PREF(int node)
{
	g_ds_pref[node].ds_kind = 1;
	g_ds_pref[node].ds_node = node;
	return (&g_ds_pref[node]);
}

inline int
bus_get_domain(device_t dev, int *domain)
{
	(void)dev;
	if (domain != nullptr)
		*domain = g_bus_get_domain_val;
	return (g_bus_get_domain_ret);
}

inline page *
alloc_pages(gfp_t gfp, unsigned int order)
{
	g_alloc_pages_calls++;
	g_alloc_pages_last_gfp = gfp;
	g_alloc_pages_last_order = order;
	return (g_alloc_pages_ret);
}

inline void
release_pages(folio **folios, unsigned int count)
{
	int slot;

	if (g_release_log_n < RELEASE_LOG) {
		slot = g_release_log_n++;
		g_release_counts[slot] = static_cast<int>(count);
		if (count > 15)
			count = 15;
		for (unsigned int i = 0; i < count; i++)
			g_release_folios[slot][i] = folios[i];
	}
}

inline int
fget_unlocked(thread *td, int fd, const cap_rights *rights, file **fpp)
{
	(void)td;
	(void)rights;
	g_fget_calls++;
	g_fget_last_fd = fd;
	if (g_fget_ret != 0)
		return (g_fget_ret);
	if (fpp != nullptr)
		*fpp = g_fget_fp;
	return (0);
}

inline eventfd_ctx *
eventfd_get(file *fp)
{
	g_eventfd_get_calls++;
	g_eventfd_get_last_fp = fp != nullptr ? fp->f_id : -1;
	return (g_eventfd_get_ret);
}

inline void
fdrop(file *fp, thread *td)
{
	(void)td;
	g_fdrop_calls++;
	g_fdrop_last_fp = fp;
}

inline void
eventfd_put(eventfd_ctx *ctx)
{
	g_eventfd_put_calls++;
	g_eventfd_put_last_ctx = ctx;
}

inline const char *
kern_getenv(const char *name)
{
	int i;

	g_kern_getenv_calls++;
	if (name != nullptr) {
		std::strncpy(g_kern_getenv_last, name, sizeof(g_kern_getenv_last) - 1);
		g_kern_getenv_last[sizeof(g_kern_getenv_last) - 1] = '\0';
	} else
		g_kern_getenv_last[0] = '\0';
	for (i = 0; i < g_env_count; i++) {
		if (std::strcmp(g_env_names[i], name) == 0)
			return (g_env_values[i]);
	}
	return (nullptr);
}

inline void
reset() noexcept
{
	int i;

	g_bus_get_domain_ret = 0;
	g_bus_get_domain_val = 0;
	g_alloc_pages_ret = nullptr;
	g_alloc_pages_last_gfp = 0;
	g_alloc_pages_last_order = 0;
	g_alloc_pages_calls = 0;
	g_release_log_n = 0;
	std::memset(g_release_counts, 0, sizeof(g_release_counts));
	std::memset(g_release_folios, 0, sizeof(g_release_folios));
	g_fget_ret = 0;
	g_fget_fp = nullptr;
	g_fget_last_fd = 0;
	g_fget_calls = 0;
	g_eventfd_get_ret = nullptr;
	g_eventfd_get_last_fp = 0;
	g_eventfd_get_calls = 0;
	g_fdrop_calls = 0;
	g_fdrop_last_fp = nullptr;
	g_eventfd_put_calls = 0;
	g_eventfd_put_last_ctx = nullptr;
	g_env_count = 0;
	g_kern_getenv_calls = 0;
	g_kern_getenv_last[0] = '\0';
	g_bootverbose = 0;
	g_curthread = nullptr;
	std::memset(&g_ds_rr, 0, sizeof(g_ds_rr));
	std::memset(g_ds_pref, 0, sizeof(g_ds_pref));
	for (i = 0; i < ENV_SLOTS; i++) {
		g_env_names[i][0] = '\0';
		g_env_values[i] = nullptr;
	}
}

inline void
set_env(const char *name, const char *value)
{
	int i;

	if (name == nullptr)
		return;
	for (i = 0; i < g_env_count; i++) {
		if (std::strcmp(g_env_names[i], name) == 0) {
			g_env_values[i] = value;
			return;
		}
	}
	if (g_env_count >= ENV_SLOTS)
		return;
	std::strncpy(g_env_names[g_env_count], name, sizeof(g_env_names[0]) - 1);
	g_env_names[g_env_count][sizeof(g_env_names[0]) - 1] = '\0';
	g_env_values[g_env_count] = value;
	g_env_count++;
}

} // namespace detail

export namespace pbsd::sys_compat_linuxkpi_common_src::b0209 {

#define KASSERT(cond, msg) do { (void)(cond); (void)(msg); } while (0)

inline thread *curthread = nullptr;
inline int bootverbose = 0;

inline void stub_reset() noexcept { detail::reset(); }

inline void stub_bootverbose(int v) noexcept { detail::g_bootverbose = v; bootverbose = v; }
inline void stub_bus_get_domain(int ret, int val) noexcept
{
	detail::g_bus_get_domain_ret = ret;
	detail::g_bus_get_domain_val = val;
}
inline void stub_alloc_pages_ret(page *p) noexcept { detail::g_alloc_pages_ret = p; }
inline void stub_fget(int ret, file *fp) noexcept
{
	detail::g_fget_ret = ret;
	detail::g_fget_fp = fp;
}
inline void stub_eventfd_get_ret(eventfd_ctx *ctx) noexcept
{
	detail::g_eventfd_get_ret = ctx;
}
inline void stub_set_env(const char *name, const char *value) noexcept
{
	detail::set_env(name, value);
}

inline int alloc_pages_calls() noexcept { return (detail::g_alloc_pages_calls); }
inline gfp_t alloc_pages_last_gfp() noexcept { return (detail::g_alloc_pages_last_gfp); }
inline unsigned int alloc_pages_last_order() noexcept
{
	return (detail::g_alloc_pages_last_order);
}
inline int release_log_n() noexcept { return (detail::g_release_log_n); }
inline int release_count(int i) noexcept { return (detail::g_release_counts[i]); }
inline folio *release_folio(int i, int j) noexcept
{
	return (detail::g_release_folios[i][j]);
}
inline int fget_calls() noexcept { return (detail::g_fget_calls); }
inline int fget_last_fd() noexcept { return (detail::g_fget_last_fd); }
inline int eventfd_get_calls() noexcept { return (detail::g_eventfd_get_calls); }
inline int eventfd_get_last_fp() noexcept { return (detail::g_eventfd_get_last_fp); }
inline int fdrop_calls() noexcept { return (detail::g_fdrop_calls); }
inline int eventfd_put_calls() noexcept { return (detail::g_eventfd_put_calls); }
inline int kern_getenv_calls() noexcept { return (detail::g_kern_getenv_calls); }
inline const char *kern_getenv_last() noexcept { return (detail::g_kern_getenv_last); }

domainset *
linux_get_vm_domain_set(int node)
{
	KASSERT(node < MAXMEMDOM, ("Invalid VM domain %d", node));

	if (node < 0)
		return (detail::DOMAINSET_RR());
	else
		return (detail::DOMAINSET_PREF(node));
}

int
linux_dev_to_node(device *dev)
{
	int numa_domain;

	if (dev == nullptr || dev->bsddev == nullptr ||
	    detail::bus_get_domain(dev->bsddev, &numa_domain) != 0)
		return (-1);
	else
		return (numa_domain);
}

folio *
folio_alloc(gfp_t gfp, unsigned int order)
{
	page *page;
	folio *fol;

	/*
	 * Allocated pages are wired already. There is no need to increase a
	 * refcount here.
	 */
	page = detail::alloc_pages(gfp | __GFP_COMP, order);
	fol = reinterpret_cast<folio *>(page);

	return (fol);
}

void
__folio_batch_release(folio_batch *fbatch)
{
	detail::release_pages(fbatch->folios, detail::folio_batch_count(fbatch));

	detail::folio_batch_reinit(fbatch);
}

eventfd_ctx *
lkpi_eventfd_ctx_fdget(int fd)
{
	file *fp;
	eventfd_ctx *ctx;

	/* Lookup file pointer by file descriptor index. */
	if (detail::fget_unlocked(curthread, fd, &detail::g_cap_no_rights, &fp) != 0)
		return (static_cast<eventfd_ctx *>(detail::ERR_PTR(-EBADF)));

	/*
	 * eventfd_get() bumps the refcount, so we can safely release the
	 * reference on the file itself afterwards.
	 */
	ctx = detail::eventfd_get(fp);
	detail::fdrop(fp, curthread);

	if (ctx == nullptr)
		return (static_cast<eventfd_ctx *>(detail::ERR_PTR(-EBADF)));

	return (ctx);
}

void
lkpi_eventfd_ctx_put(eventfd_ctx *ctx)
{
	detail::eventfd_put(ctx);
}

const char *
video_get_options(const char *connector_name)
{
	char tunable[64];
	const char *options;

	/*
	 * A user may use loader tunables to set a specific mode for the
	 * console. Tunables are read in the following order:
	 *     1. kern.vt.fb.modes.$connector_name
	 *     2. kern.vt.fb.default_mode
	 *
	 * Example of a mode specific to the LVDS connector:
	 *     kern.vt.fb.modes.LVDS="1024x768"
	 *
	 * Example of a mode applied to all connectors not having a
	 * connector-specific mode:
	 *     kern.vt.fb.default_mode="640x480"
	 */
	std::snprintf(tunable, sizeof(tunable), "kern.vt.fb.modes.%s",
	    connector_name);
	if (bootverbose) {
		::printf("[drm] Connector %s: get mode from tunables:\n", connector_name);
		::printf("[drm]  - %s\n", tunable);
		::printf("[drm]  - kern.vt.fb.default_mode\n");
	}
	options = detail::kern_getenv(tunable);
	if (options == nullptr)
		options = detail::kern_getenv("kern.vt.fb.default_mode");

	return (options);
}

} // namespace pbsd::sys_compat_linuxkpi_common_src::b0209
