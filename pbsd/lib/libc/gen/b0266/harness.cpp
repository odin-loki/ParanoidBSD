/*
 * harness.cpp -- differential test for batch b0266.
 *
 * sched_getaffinity() and __freebsd11_dirname() are driven side by side with
 * the ref_ oracle.  Every observable is compared: return values, errno, buffer
 * contents (guard bytes included), and the arguments recorded by the shared
 * __getosreldate()/cpuset_getaffinity() test doubles.
 */

import pbsd.lib.libc.gen.b0266;

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <errno.h>

namespace port = pbsd::lib_libc_gen::b0266;

extern "C" {
int ref_sched_getaffinity(int pid, size_t cpusetsz, unsigned long *cpuset);
char *ref___freebsd11_dirname(char *path);
}

#define	MAXPATHLEN	1024
#define	P_OSREL_TIDPID	1400079
#define	_PID_MAX	99999

#define	CPU_LEVEL_WHICH	3
#define	CPU_WHICH_TID	1
#define	CPU_WHICH_PID	2
#define	CPU_WHICH_TIDPID 3

enum { F_SCHED, F_DIRNAME, NFUNC };

static const char *const fname[NFUNC] = {
	"sched_getaffinity",
	"__freebsd11_dirname",
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
		std::printf("FAIL %-22s %s: %s\n", fname[f], ctx, detail);
	} else if (nreported == MAXREPORT) {
		nreported++;
		std::printf("... further failures suppressed\n");
	}
}

/* ------------------------------------------------------------------ PRNG */

static std::uint64_t rng_state = 0xc0ffeebaddecaf26ULL;

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

static int
randint(void)
{
	return (int)randu32();
}

/* --------------------------------------------------------- test doubles */

static int g_osrel;

struct cpuset_mock_state {
	int		rv;
	int		errno_in;
	int		level;
	int		which;
	int		id;
	size_t		setsize;
	unsigned long	cpuset_off;
	unsigned char	fill;
};

static cpuset_mock_state g_cpuset;

static void
cpuset_mock_reset(int rv, int errno_in)
{
	std::memset(&g_cpuset, 0, sizeof(g_cpuset));
	g_cpuset.rv = rv;
	g_cpuset.errno_in = errno_in;
	g_cpuset.cpuset_off = -1;
}

static std::uint32_t
fnv_step(std::uint32_t h, std::uint32_t v)
{
	return (h ^ v) * 16777619u;
}

extern "C" int
__getosreldate(void)
{
	return (g_osrel);
}

extern "C" int
cpuset_getaffinity(int level, int which, id_t id, size_t setsize,
    unsigned long *cpuset)
{
	std::uint32_t h;
	size_t i;

	g_cpuset.level = level;
	g_cpuset.which = which;
	g_cpuset.id = (int)id;
	g_cpuset.setsize = setsize;
	g_cpuset.cpuset_off = cpuset == nullptr ? -1 : 0;

	h = 0x811c9dc5u;
	h = fnv_step(h, (std::uint32_t)level);
	h = fnv_step(h, (std::uint32_t)which);
	h = fnv_step(h, (std::uint32_t)id);
	h = fnv_step(h, (std::uint32_t)setsize);

	if (cpuset != nullptr && setsize > 0) {
		g_cpuset.fill = (unsigned char)(h & 0xffu);
		for (i = 0; i < setsize; i++)
			((unsigned char *)cpuset)[i] = (unsigned char)(g_cpuset.fill + (unsigned char)i);
		h = fnv_step(h, g_cpuset.fill);
	}

	if (g_cpuset.rv == -1)
		errno = g_cpuset.errno_in;
	return (g_cpuset.rv);
}

struct affinity_obs {
	int		rv;
	int		errno_out;
	int		level;
	int		which;
	int		id;
	size_t		setsize;
	unsigned char	fill;
};

static affinity_obs
snap_affinity(int rv, const unsigned char *base, size_t bufsz)
{
	affinity_obs o;

	std::memset(&o, 0, sizeof(o));
	o.rv = rv;
	o.errno_out = errno;
	o.level = g_cpuset.level;
	o.which = g_cpuset.which;
	o.id = g_cpuset.id;
	o.setsize = g_cpuset.setsize;
	o.fill = g_cpuset.fill;
	(void)base;
	(void)bufsz;
	return (o);
}

static bool
affinity_obs_eq(const affinity_obs &a, const affinity_obs &b)
{
	return (a.rv == b.rv &&
	    a.errno_out == b.errno_out &&
	    a.level == b.level &&
	    a.which == b.which &&
	    a.id == b.id &&
	    a.setsize == b.setsize &&
	    a.fill == b.fill);
}

static bool
buf_eq(const unsigned char *a, const unsigned char *b, size_t n)
{
	return (std::memcmp(a, b, n) == 0);
}

static void
fill_guard(unsigned char *buf, size_t n, unsigned char guard)
{
	std::memset(buf, guard, n);
}

static bool
check_sched_case(int f, const char *ctx, int osrel, int pid, size_t setsize,
    int mock_rv, int mock_errno)
{
	enum { GUARD = 0x7f, BUFSZ = 64 };
	unsigned char refbuf[BUFSZ];
	unsigned char portbuf[BUFSZ];
	affinity_obs ref_obs, port_obs;
	int ref_rv, port_rv;

	g_osrel = osrel;
	cpuset_mock_reset(mock_rv, mock_errno);

	fill_guard(refbuf, BUFSZ, GUARD);
	fill_guard(portbuf, BUFSZ, GUARD);

	errno = 0;
	ref_rv = ref_sched_getaffinity(pid, setsize,
	    (unsigned long *)(refbuf + 8));
	ref_obs = snap_affinity(ref_rv, refbuf, BUFSZ);

	cpuset_mock_reset(mock_rv, mock_errno);
	errno = 0;
	port_rv = port::sched_getaffinity(pid, setsize,
	    (unsigned long *)(portbuf + 8));
	port_obs = snap_affinity(port_rv, portbuf, BUFSZ);

	ncases[f]++;
	if (!affinity_obs_eq(ref_obs, port_obs)) {
		report(f, ctx, "observable mismatch");
		return (false);
	}
	if (!buf_eq(refbuf, portbuf, BUFSZ)) {
		report(f, ctx, "buffer mismatch");
		return (false);
	}
	return (true);
}

static void
test_sched_handwritten(void)
{
	const int f = F_SCHED;

	(void)check_sched_case(f, "osrel=0 pid=0 ok", 0, 0, 16, 0, 0);
	(void)check_sched_case(f, "osrel=0 pid=1 ok", 0, 1, 16, 0, 0);
	(void)check_sched_case(f, "osrel=0 pid=max ok", 0, _PID_MAX, 8, 0, 0);
	(void)check_sched_case(f, "osrel=0 pid=max+1 ok", 0, _PID_MAX + 1, 8, 0, 0);
	(void)check_sched_case(f, "osrel=0 pid=-1 ok", 0, -1, 8, 0, 0);
	(void)check_sched_case(f, "osrel=tidpid pid=0", P_OSREL_TIDPID, 0, 16, 0, 0);
	(void)check_sched_case(f, "osrel=tidpid pid=5", P_OSREL_TIDPID, 5, 16, 0, 0);
	(void)check_sched_case(f, "osrel=bound-1 pid=2", P_OSREL_TIDPID - 1, 2, 16, 0, 0);
	(void)check_sched_case(f, "osrel=bound pid=2", P_OSREL_TIDPID, 2, 16, 0, 0);
	(void)check_sched_case(f, "erange->einval", 0, 0, 16, -1, ERANGE);
	(void)check_sched_case(f, "eperm preserved", 0, 3, 16, -1, EPERM);
	(void)check_sched_case(f, "setsize=0", 0, 0, 0, 0, 0);
	(void)check_sched_case(f, "setsize=1", 0, 1, 1, 0, 0);
	(void)check_sched_case(f, "setsize=32", P_OSREL_TIDPID, 9, 32, 0, 0);
}

static void
test_sched_random(void)
{
	const int f = F_SCHED;
	int i, osrel, pid, mock_rv, mock_errno;
	size_t setsize;
	static const int errtab[] = { 0, ERANGE, EPERM, EACCES, EINVAL };

	for (i = 0; i < 100000; i++) {
		char ctx[64];

		switch (randu32() % 8) {
		case 0:
			osrel = 0;
			break;
		case 1:
			osrel = P_OSREL_TIDPID;
			break;
		case 2:
			osrel = P_OSREL_TIDPID - 1;
			break;
		case 3:
			osrel = P_OSREL_TIDPID + 1;
			break;
		default:
			osrel = (int)(randu32() % 20000000u);
			break;
		}

		switch (randu32() % 7) {
		case 0:
			pid = 0;
			break;
		case 1:
			pid = 1;
			break;
		case 2:
			pid = _PID_MAX;
			break;
		case 3:
			pid = _PID_MAX + 1;
			break;
		case 4:
			pid = _PID_MAX - 1;
			break;
		case 5:
			pid = -1;
			break;
		default:
			pid = randint();
			break;
		}

		setsize = (size_t)(randu32() % 40u);
		mock_rv = (randu32() & 1u) ? 0 : -1;
		mock_errno = errtab[randu32() % (sizeof(errtab) / sizeof(errtab[0]))];

		std::snprintf(ctx, sizeof(ctx), "rand#%d", i);
		(void)check_sched_case(f, ctx, osrel, pid, setsize, mock_rv, mock_errno);
	}
}

/* ------------------------------------------------------------- dirname */

struct dirname_obs {
	int		is_null;
	int		errno_out;
	char		result[MAXPATHLEN + 4];
};

static dirname_obs
snap_dirname(char *rv)
{
	dirname_obs o;

	std::memset(&o, 0, sizeof(o));
	o.is_null = rv == nullptr;
	o.errno_out = errno;
	if (rv != nullptr)
		std::strncpy(o.result, rv, sizeof(o.result) - 1);
	return (o);
}

static bool
dirname_obs_eq(const dirname_obs &a, const dirname_obs &b)
{
	return (a.is_null == b.is_null &&
	    a.errno_out == b.errno_out &&
	    std::strcmp(a.result, b.result) == 0);
}

static bool
check_dirname_case(int f, const char *ctx, char *refpath, char *portpath)
{
	enum { PATHCAP = 4096, GUARD = 0x7f };
	unsigned char refwrap[PATHCAP];
	unsigned char portwrap[PATHCAP];
	char *ref_rv, *port_rv;
	dirname_obs ref_obs, port_obs;
	size_t replen;

	replen = refpath == nullptr ? 0 : std::strlen(refpath);
	if (replen + 16 > PATHCAP - 8)
		replen = PATHCAP - 24;

	fill_guard(refwrap, PATHCAP, GUARD);
	fill_guard(portwrap, PATHCAP, GUARD);
	if (refpath != nullptr) {
		std::memcpy(refwrap + 8, refpath, replen);
		refwrap[8 + replen] = '\0';
	}
	if (portpath != nullptr) {
		std::memcpy(portwrap + 8, portpath, replen);
		portwrap[8 + replen] = '\0';
	}

	errno = 0;
	ref_rv = ref___freebsd11_dirname(refpath == nullptr ? nullptr :
	    (char *)(refwrap + 8));
	ref_obs = snap_dirname(ref_rv);

	errno = 0;
	port_rv = port::__freebsd11_dirname(portpath == nullptr ? nullptr :
	    (char *)(portwrap + 8));
	port_obs = snap_dirname(port_rv);

	ncases[f]++;
	if (!dirname_obs_eq(ref_obs, port_obs)) {
		report(f, ctx, "observable mismatch");
		return (false);
	}
	if (!buf_eq(refwrap, portwrap, PATHCAP)) {
		report(f, ctx, "input buffer mismatch");
		return (false);
	}
	return (true);
}

static void
fill_path_pattern(char *buf, size_t cap, unsigned pattern)
{
	size_t i, n;
	const char *segs[] = {
		"", ".", "/", "//", "///", "a", "foo", "\x80\xff", "a/b", "/a",
		"/a/b", "a//b", "a///b", "foo/bar/baz", "\xff/\xfe", "a/\x80/b"
	};

	n = 0;
	switch (pattern % 16) {
	case 0:
		buf[0] = '\0';
		return;
	case 1:
		for (i = 0; i < cap - 1 && i < (size_t)(MAXPATHLEN + 8); i++)
			buf[i] = (char)('a' + (i % 26));
		buf[i] = '\0';
		return;
	case 2:
		for (i = 0; i < cap - 1 && i < (size_t)MAXPATHLEN; i++)
			buf[i] = '/';
		buf[i] = '\0';
		return;
	case 3:
		for (i = 0; i < cap - 1 && i < (size_t)(MAXPATHLEN + 4); i++)
			buf[i] = (char)(0x80 + (i & 0x7f));
		buf[i] = '\0';
		return;
	case 4:
		for (i = 0; i < (size_t)MAXPATHLEN - 1 && i < cap - 2; i++)
			buf[i] = 'x';
		buf[i++] = '/';
		buf[i++] = 'y';
		buf[i] = '\0';
		return;
	case 5:
		for (i = 0; i < (size_t)MAXPATHLEN && i < cap - 2; i++)
			buf[i] = 'z';
		buf[i++] = '/';
		buf[i] = 'q';
		buf[i + 1] = '\0';
		return;
	default:
		n = pattern % (sizeof(segs) / sizeof(segs[0]));
		std::strncpy(buf, segs[n], cap - 1);
		buf[cap - 1] = '\0';
		return;
	}
}

static void
test_dirname_handwritten(void)
{
	const int f = F_DIRNAME;
	enum { LOCAL = 4096 };
	char refpath[LOCAL];
	char portpath[LOCAL];

	(void)check_dirname_case(f, "null", nullptr, nullptr);

	refpath[0] = portpath[0] = '\0';
	(void)check_dirname_case(f, "empty", refpath, portpath);

	std::strcpy(refpath, ".");
	std::strcpy(portpath, ".");
	(void)check_dirname_case(f, "dot", refpath, portpath);

	std::strcpy(refpath, "/");
	std::strcpy(portpath, "/");
	(void)check_dirname_case(f, "slash", refpath, portpath);

	std::strcpy(refpath, "//");
	std::strcpy(portpath, "//");
	(void)check_dirname_case(f, "doubleslash", refpath, portpath);

	std::strcpy(refpath, "foo");
	std::strcpy(portpath, "foo");
	(void)check_dirname_case(f, "foo", refpath, portpath);

	std::strcpy(refpath, "foo/bar");
	std::strcpy(portpath, "foo/bar");
	(void)check_dirname_case(f, "foo/bar", refpath, portpath);

	std::strcpy(refpath, "foo/bar/");
	std::strcpy(portpath, "foo/bar/");
	(void)check_dirname_case(f, "foo/bar/", refpath, portpath);

	std::strcpy(refpath, "/foo/bar");
	std::strcpy(portpath, "/foo/bar");
	(void)check_dirname_case(f, "/foo/bar", refpath, portpath);

	std::strcpy(refpath, "a/b");
	std::strcpy(portpath, "a/b");
	(void)check_dirname_case(f, "a/b", refpath, portpath);

	std::strcpy(refpath, "a//b");
	std::strcpy(portpath, "a//b");
	(void)check_dirname_case(f, "a//b", refpath, portpath);

	std::memset(refpath, 'a', MAXPATHLEN);
	refpath[MAXPATHLEN] = '\0';
	std::memset(portpath, 'a', MAXPATHLEN);
	portpath[MAXPATHLEN] = '\0';
	(void)check_dirname_case(f, "long-no-slash", refpath, portpath);

	std::memset(refpath, 'a', MAXPATHLEN);
	refpath[MAXPATHLEN] = '/';
	refpath[MAXPATHLEN + 1] = 'b';
	refpath[MAXPATHLEN + 2] = '\0';
	std::memset(portpath, 'a', MAXPATHLEN);
	portpath[MAXPATHLEN] = '/';
	portpath[MAXPATHLEN + 1] = 'b';
	portpath[MAXPATHLEN + 2] = '\0';
	(void)check_dirname_case(f, "enametoolong", refpath, portpath);

	refpath[0] = (char)0x80;
	refpath[1] = '/';
	refpath[2] = (char)0xff;
	refpath[3] = '\0';
	portpath[0] = (char)0x80;
	portpath[1] = '/';
	portpath[2] = (char)0xff;
	portpath[3] = '\0';
	(void)check_dirname_case(f, "highbit", refpath, portpath);
}

static void
test_dirname_random(void)
{
	const int f = F_DIRNAME;
	enum { LOCAL = 4096 };
	char refpath[LOCAL];
	char portpath[LOCAL];
	int i;

	for (i = 0; i < 100000; i++) {
		char ctx[64];
		unsigned p;
		size_t j, n;

		p = randu32();
		fill_path_pattern(refpath, LOCAL, p);
		fill_path_pattern(portpath, LOCAL, p);

		if ((randu32() & 7u) == 0u) {
			n = randu32() % (LOCAL - 2);
			for (j = 0; j < n; j++)
				refpath[j] = portpath[j] =
				    (char)(randu32() & 0xffu);
			refpath[n] = portpath[n] = '\0';
		}

		std::snprintf(ctx, sizeof(ctx), "rand#%d", i);
		(void)check_dirname_case(f, ctx, refpath, portpath);
	}
}

/* ------------------------------------------------------------------ main */

int
main(void)
{
	int i, failed = 0;

	test_sched_handwritten();
	test_sched_random();
	test_dirname_handwritten();
	test_dirname_random();

	std::printf("\n%-22s %12s %12s\n", "function", "cases", "failures");
	std::printf("%-22s %12s %12s\n", "--------", "-----", "--------");
	for (i = 0; i < NFUNC; i++) {
		std::printf("%-22s %12llu %12llu\n", fname[i],
		    ncases[i], nfails[i]);
		if (nfails[i] != 0)
			failed = 1;
	}

	return (failed ? 1 : 0);
}
