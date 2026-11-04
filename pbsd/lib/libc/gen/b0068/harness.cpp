/*
 * Differential test for batch b0068: pbsd::lib_libc_gen::b0068 against the
 * ref_ oracle in oracle.c.
 *
 * FreeBSD-private helpers (_rtld_error, __xuname, __opendir2) are not present
 * on this host, so minimal stubs below implement behaviour that depends on the
 * exact arguments the ported functions pass (32 for __xuname, DTF_HIDEW |
 * DTF_NODUP for __opendir2).  A planted mutation in any of those constants,
 * comparisons, or flag expressions changes the observable result.
 */

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <dlfcn.h>
#include <errno.h>
#include <stdarg.h>
#include <sys/utsname.h>
#include <dirent.h>
#include <unistd.h>

import pbsd.lib.libc.gen.b0068;

namespace port = pbsd::lib_libc_gen::b0068;

#ifndef DTF_HIDEW
#define	DTF_HIDEW	0x0001
#endif
#ifndef DTF_NODUP
#define	DTF_NODUP	0x0002
#endif

#define	EXPECTED_XUNAME_VER	32
#define	EXPECTED_OPENDIR_FLAGS	(DTF_HIDEW | DTF_NODUP)

/* ------------------------------------------------------------------ */
/* Stubs for FreeBSD-private libc internals                           */
/* ------------------------------------------------------------------ */

extern "C" {

void
_rtld_error(const char *fmt, ...)
{
	(void)fmt;
}

struct stub_dir_slot {
	unsigned char	guard_pre[16];
	char		path[512];
	int		flags;
	unsigned char	guard_post[16];
};

static struct stub_dir_slot stub_dir_pool[32];

static unsigned
stub_hash(const char *s)
{
	unsigned h = 0x811c9dc5u;

	if (s == nullptr)
		return h;
	for (; *s != '\0'; s++)
		h = (h ^ (unsigned char)*s) * 0x01000193u;
	return h;
}

int
__xuname(int ver, void *name)
{
	struct utsname *u = static_cast<struct utsname *>(name);

	if (u == nullptr)
		return EFAULT;
	if (ver != EXPECTED_XUNAME_VER) {
		std::memset(u, 0xA5, sizeof(*u));
		errno = EINVAL;
		return -1;
	}
	std::memset(u, 0, sizeof(*u));
	std::strncpy(u->sysname, "PBSD", sizeof(u->sysname) - 1);
	std::strncpy(u->nodename, "stubhost", sizeof(u->nodename) - 1);
	std::strncpy(u->release, "0.1", sizeof(u->release) - 1);
	std::strncpy(u->version, "test", sizeof(u->version) - 1);
	std::strncpy(u->machine, "x86_64", sizeof(u->machine) - 1);
	return 0;
}

DIR *
__opendir2(const char *name, int flags)
{
	struct stub_dir_slot *slot;
	unsigned idx;

	if (flags != EXPECTED_OPENDIR_FLAGS) {
		errno = EINVAL;
		return nullptr;
	}
	if (name == nullptr) {
		errno = EINVAL;
		return nullptr;
	}
	if (name[0] == '\0') {
		errno = ENOENT;
		return nullptr;
	}

	idx = stub_hash(name) % (sizeof(stub_dir_pool) /
	    sizeof(stub_dir_pool[0]));
	slot = &stub_dir_pool[idx];
	std::memset(slot->guard_pre, 0x7f, sizeof(slot->guard_pre));
	std::memset(slot->guard_post, 0x7f, sizeof(slot->guard_post));
	std::memset(slot->path, 0, sizeof(slot->path));
	std::strncpy(slot->path, name, sizeof(slot->path) - 1);
	slot->flags = flags;
	return reinterpret_cast<DIR *>(slot);
}

void *
ref_libc_dlopen(const char *path, int mode);
void
ref___FreeBSD_libc_enter_restricted_mode(void);
int
ref_uname(struct utsname *name);
DIR *
ref_opendir(const char *name);

} /* extern "C" */

/* ------------------------------------------------------------------ */

struct Stats {
	const char	       *name;
	unsigned long long	cases;
	unsigned long long	fails;
	unsigned long long	printed;
};

static const unsigned MAXPRINT = 8;
static const unsigned char GUARD = 0x7f;

static Stats st_uname = { "uname", 0, 0, 0 };
static Stats st_opendir = { "opendir", 0, 0, 0 };
static Stats st_dlopen = { "libc_dlopen", 0, 0, 0 };
static Stats st_restrict = {
	"__FreeBSD_libc_enter_restricted_mode", 0, 0, 0
};

static std::uint64_t rng = 0x243f6a8885a308d3ULL;

static std::uint64_t
rnd_u64(void)
{
	std::uint64_t z;

	rng += 0x9e3779b97f4a7c15ULL;
	z = rng;
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return z ^ (z >> 31);
}

static int
rnd_i(void)
{
	return static_cast<int>(rnd_u64() & 0x7fffffffu);
}

static void
fill_guard(void *p, size_t n)
{
	std::memset(p, GUARD, n);
}

static bool
guard_ok(const void *p, size_t n)
{
	const unsigned char *b = static_cast<const unsigned char *>(p);

	for (size_t i = 0; i < n; i++)
		if (b[i] != GUARD)
			return false;
	return true;
}

static void
fail_stat(Stats &st, const char *label, const char *detail)
{
	st.fails++;
	if (st.printed++ < MAXPRINT)
		std::printf("  FAIL %-36s %-20s %s\n", st.name, label, detail);
}

struct GuardedUtsname {
	unsigned char	pre[32];
	struct utsname	u;
	unsigned char	post[32];
};

static void
init_guarded_utsname(GuardedUtsname &g)
{
	fill_guard(&g, sizeof(g));
}

static bool
guarded_utsname_equal(const GuardedUtsname &a, const GuardedUtsname &b)
{
	return std::memcmp(&a, &b, sizeof(GuardedUtsname)) == 0;
}

static bool
test_uname_once(Stats &st, const char *label, struct utsname *pu,
    struct utsname *ru)
{
	int pr, rr;

	pr = port::uname(pu);
	rr = ref_uname(ru);
	st.cases++;

	if (pr != rr) {
		fail_stat(st, label, "return value mismatch");
		return false;
	}
	if (!guarded_utsname_equal(
		*reinterpret_cast<GuardedUtsname *>(pu),
		*reinterpret_cast<GuardedUtsname *>(ru))) {
		fail_stat(st, label, "buffer mismatch");
		return false;
	}
	return true;
}

static void
case_uname(const char *label)
{
	GuardedUtsname gp, gr;

	init_guarded_utsname(gp);
	init_guarded_utsname(gr);
	test_uname_once(st_uname, label,
	    reinterpret_cast<struct utsname *>(&gp),
	    reinterpret_cast<struct utsname *>(&gr));
}

static void
case_uname_null(const char *label)
{
	int pr, rr;

	st_uname.cases++;
	pr = port::uname(nullptr);
	rr = ref_uname(nullptr);
	if (pr != rr)
		fail_stat(st_uname, label, "return value mismatch");
}

static bool
dir_ptr_equal(DIR *a, DIR *b)
{
	if (a == nullptr && b == nullptr)
		return true;
	if (a == nullptr || b == nullptr)
		return false;
	return std::strcmp(reinterpret_cast<struct stub_dir_slot *>(a)->path,
	    reinterpret_cast<struct stub_dir_slot *>(b)->path) == 0 &&
	    reinterpret_cast<struct stub_dir_slot *>(a)->flags ==
	    reinterpret_cast<struct stub_dir_slot *>(b)->flags;
}

static void
case_opendir(const char *label, const char *path)
{
	DIR *pd, *rd;

	st_opendir.cases++;
	pd = port::opendir(path);
	rd = ref_opendir(path);
	if (!dir_ptr_equal(pd, rd))
		fail_stat(st_opendir, label, "DIR* mismatch");
}

static void
case_dlopen(const char *label, const char *path, int mode)
{
	void *pv, *rv;

	st_dlopen.cases++;
	pv = port::libc_dlopen(path, mode);
	rv = ref_libc_dlopen(path, mode);
	if ((pv == nullptr) != (rv == nullptr))
		fail_stat(st_dlopen, label, "pointer nullness mismatch");
}

static void
case_restricted_dlopen(const char *label, const char *path, int mode)
{
	void *pv, *rv;

	st_restrict.cases++;
	port::__FreeBSD_libc_enter_restricted_mode();
	ref___FreeBSD_libc_enter_restricted_mode();
	pv = port::libc_dlopen(path, mode);
	rv = ref_libc_dlopen(path, mode);
	if (pv != nullptr || rv != nullptr)
		fail_stat(st_restrict, label, "expected NULL in restricted mode");
	if (pv != rv)
		fail_stat(st_restrict, label, "pointer value mismatch");
}

static void
fill_path_buf(char *buf, size_t cap, std::uint64_t seed, int len)
{
	size_t i;

	if (cap == 0)
		return;
	if (len < 0)
		len = 0;
	if ((size_t)len >= cap)
		len = (int)cap - 1;
	for (i = 0; i < (size_t)len; i++) {
		std::uint64_t v = seed + i * 0x9e3779b97f4a7c15ULL;
		unsigned char c = static_cast<unsigned char>(
		    (v ^ (v >> 8) ^ (v >> 16) ^ (v >> 24)) & 0xffu);
		if (c == '\0' && i + 1 < (size_t)len)
			c = 0x80;
		buf[i] = static_cast<char>(c);
	}
	buf[len] = '\0';
}

static void
handwritten_cases(void)
{
	static const char *paths[] = {
		"", ".", "/", "/tmp", "/nonexistent_pbsd_b0068",
		"a", "\x80", "\xff", "\x80\xff\xfe",
		"foo\x00bar", "nul-heavy\x00\x00\x00tail",
	};
	char longbuf[300];

	case_uname("valid struct");
	case_uname_null("NULL pointer");

	for (size_t i = 0; i < sizeof(paths) / sizeof(paths[0]); i++)
		case_opendir("edge path", paths[i]);

	std::memset(longbuf, 'x', sizeof(longbuf) - 1);
	longbuf[sizeof(longbuf) - 1] = '\0';
	case_opendir("long path", longbuf);

	static const int modes[] = { RTLD_LAZY, RTLD_NOW, RTLD_LAZY | RTLD_LOCAL };

	for (size_t mi = 0; mi < sizeof(modes) / sizeof(modes[0]); mi++) {
		int m = modes[mi];
		case_dlopen("mode sweep", "/lib/libc.so.6", m);
		case_dlopen("mode sweep invalid", "/no_such_lib_pbsd", m);
	}
	case_dlopen("NULL path", nullptr, RTLD_LAZY);
	case_dlopen("empty path", "", RTLD_LAZY);

	case_restricted_dlopen("restricted NULL", nullptr, RTLD_LAZY);
	case_restricted_dlopen("restricted path", "/lib/libc.so.6", RTLD_LAZY);
}

static void
random_sweep(void)
{
	char pathbuf[256];
	GuardedUtsname gpu, gru;
	const unsigned long long ITERS = 200000u;
	unsigned long long i;

	for (i = 0; i < ITERS; i++) {
		int kind = static_cast<int>(rnd_u64() % 3u);

		switch (kind) {
		case 0:
			init_guarded_utsname(gpu);
			init_guarded_utsname(gru);
			test_uname_once(st_uname, "random",
			    reinterpret_cast<struct utsname *>(&gpu),
			    reinterpret_cast<struct utsname *>(&gru));
			break;
		case 1:
			fill_path_buf(pathbuf, sizeof(pathbuf), rnd_u64(),
			    static_cast<int>(rnd_u64() % 200u));
			case_opendir("random", pathbuf);
			break;
		default:
			fill_path_buf(pathbuf, sizeof(pathbuf), rnd_u64(),
			    static_cast<int>(rnd_u64() % 120u));
			case_dlopen("random", pathbuf,
			    RTLD_LAZY | (rnd_i() & (RTLD_NOW | RTLD_LOCAL)));
			break;
		}
	}
}

static void
report(void)
{
	Stats all[] = { st_uname, st_opendir, st_dlopen, st_restrict };
	unsigned long long tc = 0, tf = 0;

	std::printf("\n%-36s %12s %12s\n", "function", "cases", "failures");
	std::printf("--------------------------------------------------------\n");
	for (Stats &st : all) {
		std::printf("%-36s %12llu %12llu\n", st.name, st.cases,
		    st.fails);
		tc += st.cases;
		tf += st.fails;
	}
	std::printf("--------------------------------------------------------\n");
	std::printf("%-36s %12llu %12llu\n", "TOTAL", tc, tf);
	std::printf("\n%s\n", tf == 0 ? "PASS" : "FAIL");
	std::fflush(stdout);
}

int
main(void)
{
	handwritten_cases();
	random_sweep();
	report();
	return (st_uname.fails + st_opendir.fails + st_dlopen.fails +
	    st_restrict.fails) == 0 ? 0 : 1;
}
