/*
 * harness.cpp -- differential test for PBSD batch b0297.
 */

#include <climits>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <unistd.h>

#include <fcntl.h>
#include <sys/ioctl.h>

import pbsd.sbin.ipf.libipf.b0297;

namespace P = pbsd::sbin_ipf_libipf::b0297;

#ifndef SIOCFUNCL
#define SIOCFUNCL _IOWR('r', 86, struct ipfunc_resolve)
#endif

#ifndef SIOCIPFINTERROR
#define SIOCIPFINTERROR _IOR('r', 98, int)
#endif

#define OPT_DONTOPEN 0x10000000

extern "C" {
int opts;
void ref_printfraginfo(char *prefix, P::ipfr_t *ifr);
char *ref_kvatoname(P::ipfunc_t func, P::ioctlfunc_t iocfunc);
void ref_printunit(int unit);
void ref_ipf_perror(int err, char *string);
int ref_ipf_perror_fd(int fd, P::ioctlfunc_t iocfunc, char *string);
void ref_ipferror(int fd, char *msg);
}

static constexpr unsigned char GUARD = 0x7f;
static constexpr int MAX_REPORT = 8;
static constexpr long SWEEP = 200000;

struct Stat {
	const char *name;
	long cases;
	long fails;
	int reported;
};

static Stat st_printfraginfo = { "printfraginfo", 0, 0, 0 };
static Stat st_kvatoname = { "kvatoname", 0, 0, 0 };
static Stat st_printunit = { "printunit", 0, 0, 0 };
static Stat st_ipf_perror = { "ipf_perror", 0, 0, 0 };
static Stat st_ipf_perror_fd = { "ipf_perror_fd", 0, 0, 0 };
static Stat st_ipferror = { "ipferror", 0, 0, 0 };

static std::uint64_t rng_state = 0xb0297feedfaceULL;

static int g_open_fail = 0;
static int g_wrapped_ioctl_ret = 0;
static int g_wrapped_ioctl_realerr = 0;
static int g_pfd_realerr = 0;
static int g_pfd_ret = 0;

extern "C" int __real_open(const char *, int, ...);
extern "C" int __real_close(int);
extern "C" int __real_ioctl(int, unsigned long, ...);

extern "C" int
__wrap_open(const char *path, int flags, ...)
{
	(void)path;
	(void)flags;
	if (g_open_fail)
		return -1;
	return 3;
}

extern "C" int
__wrap_close(int fd)
{
	return __real_close(fd);
}

extern "C" int
__wrap_ioctl(int fd, unsigned long req, ...)
{
	va_list ap;

	va_start(ap, req);
	void *argp = va_arg(ap, void *);
	va_end(ap);

	if (req == (unsigned long)SIOCIPFINTERROR) {
		if (argp != nullptr)
			*(int *)argp = g_wrapped_ioctl_realerr;
		return g_wrapped_ioctl_ret;
	}
	return __real_ioctl(fd, req, argp);
}

static inline std::uint64_t
rnd(void)
{
	std::uint64_t z = (rng_state += 0x9e3779b97f4a7c15ULL);

	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return z ^ (z >> 31);
}

static inline std::size_t
rnd_mod(std::size_t m)
{
	if (m == 0)
		return 0;
	return (std::size_t)(rnd() % (std::uint64_t)m);
}

static void
stat_fail(Stat *st, const char *tag, const char *detail)
{
	st->fails++;
	if (st->reported < MAX_REPORT) {
		st->reported++;
		std::printf("  FAIL %s [%s] %s\n", st->name, tag, detail);
	}
}

static void
sync_opts(int v)
{
	P::opts = v;
	opts = v;
}

static void
fill_str(char *buf, std::size_t cap, int pattern, std::size_t len)
{
	static const unsigned char alpha[] = {
	    0x00, 0x01, 0x7e, 0x7f, 0x80, 0xfe, 0xff,
	    'a', 'z', 'A', 'Z', '0', '9', '/', ':', '-'
	};
	const std::size_t na = sizeof(alpha);

	if (cap == 0)
		return;
	if (len >= cap)
		len = cap - 1;
	for (std::size_t i = 0; i < len; i++) {
		switch (pattern) {
		case 0:
			buf[i] = (char)alpha[i % na];
			break;
		case 1:
			buf[i] = (char)(0x80 + (i & 0x7f));
			break;
		case 2:
			buf[i] = (char)((i & 1) ? '\0' : 'x');
			break;
		default:
			buf[i] = (char)(rnd() & 0xff);
			break;
		}
	}
	buf[len] = '\0';
}

static std::string
capture_stdout_frag(void (*fn)(char *, P::ipfr_t *), char *prefix,
    P::ipfr_t *ifr)
{
	std::fflush(stdout);
	int fds[2];
	if (pipe(fds) != 0)
		std::abort();
	int saved = dup(STDOUT_FILENO);
	dup2(fds[1], STDOUT_FILENO);
	close(fds[1]);
	fn(prefix, ifr);
	std::fflush(stdout);
	dup2(saved, STDOUT_FILENO);
	close(saved);
	char out[8192];
	ssize_t n = read(fds[0], out, sizeof(out) - 1);
	close(fds[0]);
	if (n < 0)
		n = 0;
	out[n] = '\0';
	return std::string(out);
}

static std::string
capture_stdout_unit(void (*fn)(int), int unit)
{
	std::fflush(stdout);
	int fds[2];
	if (pipe(fds) != 0)
		std::abort();
	int saved = dup(STDOUT_FILENO);
	dup2(fds[1], STDOUT_FILENO);
	close(fds[1]);
	fn(unit);
	std::fflush(stdout);
	dup2(saved, STDOUT_FILENO);
	close(saved);
	char out[256];
	ssize_t n = read(fds[0], out, sizeof(out) - 1);
	close(fds[0]);
	if (n < 0)
		n = 0;
	out[n] = '\0';
	return std::string(out);
}

static std::string
capture_stderr_perror(void (*fn)(int, char *), int err, char *msg)
{
	std::fflush(stderr);
	int fds[2];
	if (pipe(fds) != 0)
		std::abort();
	int saved = dup(STDERR_FILENO);
	dup2(fds[1], STDERR_FILENO);
	close(fds[1]);
	fn(err, msg);
	std::fflush(stderr);
	dup2(saved, STDERR_FILENO);
	close(saved);
	char out[8192];
	ssize_t n = read(fds[0], out, sizeof(out) - 1);
	close(fds[0]);
	if (n < 0)
		n = 0;
	out[n] = '\0';
	return std::string(out);
}

struct PfdResult {
	std::string err;
	int ret;
};

static PfdResult
capture_stderr_pfd(int (*fn)(int, P::ioctlfunc_t, char *), int fd,
    P::ioctlfunc_t ioc, char *msg)
{
	PfdResult r;

	std::fflush(stderr);
	int fds[2];
	if (pipe(fds) != 0)
		std::abort();
	int saved = dup(STDERR_FILENO);
	dup2(fds[1], STDERR_FILENO);
	close(fds[1]);
	r.ret = fn(fd, ioc, msg);
	std::fflush(stderr);
	dup2(saved, STDERR_FILENO);
	close(saved);
	char out[8192];
	ssize_t n = read(fds[0], out, sizeof(out) - 1);
	close(fds[0]);
	if (n < 0)
		n = 0;
	out[n] = '\0';
	r.err = std::string(out);
	return r;
}

static std::string
capture_stderr_ferror(void (*fn)(int, char *), int fd, char *msg)
{
	std::fflush(stderr);
	int fds[2];
	if (pipe(fds) != 0)
		std::abort();
	int saved = dup(STDERR_FILENO);
	dup2(fds[1], STDERR_FILENO);
	close(fds[1]);
	fn(fd, msg);
	std::fflush(stderr);
	dup2(saved, STDERR_FILENO);
	close(saved);
	char out[8192];
	ssize_t n = read(fds[0], out, sizeof(out) - 1);
	close(fds[0]);
	if (n < 0)
		n = 0;
	out[n] = '\0';
	return std::string(out);
}

static void
call_port_printfraginfo(char *prefix, P::ipfr_t *ifr)
{
	P::printfraginfo(prefix, ifr);
}

static void
call_ref_printfraginfo(char *prefix, P::ipfr_t *ifr)
{
	ref_printfraginfo(prefix, ifr);
}

static void
call_port_printunit(int unit)
{
	P::printunit(unit);
}

static void
call_ref_printunit(int unit)
{
	ref_printunit(unit);
}

static void
call_port_ipf_perror(int err, char *msg)
{
	P::ipf_perror(err, msg);
}

static void
call_ref_ipf_perror(int err, char *msg)
{
	ref_ipf_perror(err, msg);
}

static int
mock_kv_ioctl(int fd, unsigned long cmd, ...)
{
	va_list ap;
	void *arg;

	va_start(ap, cmd);
	arg = va_arg(ap, void *);
	va_end(ap);
	(void)fd;
	if (cmd != (unsigned long)SIOCFUNCL || arg == nullptr)
		return -1;

	P::ipfunc_resolve_t *res = (P::ipfunc_resolve_t *)arg;
	std::uintptr_t tag = (std::uintptr_t)res->ipfu_addr;
	std::snprintf(res->ipfu_name, sizeof(res->ipfu_name), "fn%zx",
	    (std::size_t)tag);
	return 0;
}

static int
mock_kv_ioctl_long(int fd, unsigned long cmd, ...)
{
	va_list ap;
	void *arg;

	va_start(ap, cmd);
	arg = va_arg(ap, void *);
	va_end(ap);
	(void)fd;
	if (cmd != (unsigned long)SIOCFUNCL || arg == nullptr)
		return -1;

	P::ipfunc_resolve_t *res = (P::ipfunc_resolve_t *)arg;
	std::memset(res->ipfu_name, 'A', sizeof(res->ipfu_name));
	return 0;
}

static int
mock_kv_ioctl_empty(int fd, unsigned long cmd, ...)
{
	va_list ap;
	void *arg;

	va_start(ap, cmd);
	arg = va_arg(ap, void *);
	va_end(ap);
	(void)fd;
	if (cmd != (unsigned long)SIOCFUNCL || arg == nullptr)
		return -1;

	P::ipfunc_resolve_t *res = (P::ipfunc_resolve_t *)arg;
	res->ipfu_name[0] = '\0';
	return 0;
}

static int
mock_kv_ioctl_hibyte(int fd, unsigned long cmd, ...)
{
	va_list ap;
	void *arg;

	va_start(ap, cmd);
	arg = va_arg(ap, void *);
	va_end(ap);
	(void)fd;
	if (cmd != (unsigned long)SIOCFUNCL || arg == nullptr)
		return -1;

	P::ipfunc_resolve_t *res = (P::ipfunc_resolve_t *)arg;
	for (std::size_t i = 0; i < sizeof(res->ipfu_name) - 1; i++)
		res->ipfu_name[i] = (char)(0x80 + (i & 0x7f));
	res->ipfu_name[sizeof(res->ipfu_name) - 1] = '\0';
	return 0;
}

static int
mock_pfd_ioctl_global(int fd, unsigned long cmd, ...)
{
	va_list ap;
	void *arg;

	(void)fd;
	va_start(ap, cmd);
	arg = va_arg(ap, void *);
	va_end(ap);
	if (cmd != (unsigned long)SIOCIPFINTERROR || arg == nullptr)
		return -1;
	*(int *)arg = g_pfd_realerr;
	return g_pfd_ret;
}

static void
init_ipfr(P::ipfr_t *ifr, int v, std::uint32_t id, unsigned long ttl,
    unsigned char pr, unsigned int pkts, unsigned int bytes, unsigned char seen0,
    int ref, std::uint32_t src, std::uint32_t dst)
{
	std::memset(ifr, 0, sizeof(*ifr));
	ifr->ipfr_v = v;
	ifr->ipfr_id = id;
	ifr->ipfr_ttl = ttl;
	ifr->ipfr_p = pr;
	ifr->ipfr_pkts = pkts;
	ifr->ipfr_bytes = bytes;
	ifr->ipfr_seen0 = seen0;
	ifr->ipfr_ref = ref;
	ifr->ipfr_src.s_addr = src;
	ifr->ipfr_dst.s_addr = dst;
}

static void
printfraginfo_case(const char *tag, const char *prefix_in, int v,
    std::uint32_t id, unsigned long ttl, unsigned char pr, unsigned int pkts,
    unsigned int bytes, unsigned char seen0, int refv, std::uint32_t src,
    std::uint32_t dst)
{
	unsigned char arena_p[sizeof(P::ipfr_t) + 256];
	unsigned char arena_r[sizeof(P::ipfr_t) + 256];
	unsigned char pref_p[128];
	unsigned char pref_r[128];

	std::memset(arena_p, GUARD, sizeof(arena_p));
	std::memset(arena_r, GUARD, sizeof(arena_r));
	std::memset(pref_p, GUARD, sizeof(pref_p));
	std::memset(pref_r, GUARD, sizeof(pref_r));

	if (prefix_in != nullptr)
		std::snprintf((char *)pref_p + 16, sizeof(pref_p) - 16, "%s",
		    prefix_in);
	else
		fill_str((char *)pref_p + 16, sizeof(pref_p) - 16,
		    (int)(rnd() % 4), rnd_mod(48));

	std::memcpy(pref_r + 16, pref_p + 16, sizeof(pref_p) - 16);

	P::ipfr_t *ip_p = (P::ipfr_t *)(arena_p + 32);
	P::ipfr_t *ip_r = (P::ipfr_t *)(arena_r + 32);
	init_ipfr(ip_p, v, id, ttl, pr, pkts, bytes, seen0, refv, src, dst);
	init_ipfr(ip_r, v, id, ttl, pr, pkts, bytes, seen0, refv, src, dst);

	std::string got = capture_stdout_frag(call_port_printfraginfo,
	    (char *)pref_p + 16, ip_p);
	std::string ref = capture_stdout_frag(call_ref_printfraginfo,
	    (char *)pref_r + 16, ip_r);

	st_printfraginfo.cases++;
	int bad_out = (got != ref);
	int bad_buf = (std::memcmp(arena_p, arena_r, sizeof(arena_p)) != 0) ||
	    (std::memcmp(pref_p, pref_r, sizeof(pref_p)) != 0);
	if (bad_out || bad_buf) {
		stat_fail(&st_printfraginfo, tag, bad_out ? "stdout" : "buf");
		if (st_printfraginfo.reported <= MAX_REPORT) {
			std::printf("      port=%.*s\n", (int)got.size(),
			    got.c_str());
			std::printf("      ref =%.*s\n", (int)ref.size(),
			    ref.c_str());
		}
	}
}

static void
test_printfraginfo_edges(void)
{
	static const char *prefixes[] = {
	    "",
	    "pfx",
	    "\x80\xff",
	    "a\xffb",
	    "long-prefix-value",
	};

	for (std::size_t i = 0; i < sizeof(prefixes) / sizeof(prefixes[0]);
	     i++) {
		char tag[48];
		std::snprintf(tag, sizeof(tag), "pfx%zu", i);
		printfraginfo_case(tag, prefixes[i], 4, 0x1234, 64, 6, 1, 100,
		    0, 1, 0x0a000001, 0x0a000002);
		printfraginfo_case(tag, prefixes[i], 6, 0xabcd, 128, 17, 9,
		    0xffff, 1, -1, 0x20010db8, 0xdeadbeef);
	}

	printfraginfo_case("v0", "", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	printfraginfo_case("v5", "x", 5, 1, 1, 1, 1, 1, 0xff, 7, 0x80808080,
	    0x7f7f7f7f);
	printfraginfo_case("v7", "", 7, 0xffffffffU, ULONG_MAX, 255, UINT_MAX,
	    UINT_MAX, 0x80, INT_MAX, 0xffffffffU, 0);
}

static void
test_printfraginfo_sweep(void)
{
	for (long i = 0; i < SWEEP; i++) {
		int v = (int)(rnd() % 8);
		if ((rnd() & 7) == 0)
			v = 6;
		if ((rnd() & 15) == 0)
			v = (int)(rnd() % 20);
		char tag[48];
		std::snprintf(tag, sizeof(tag), "rnd%ld", i);
		printfraginfo_case(tag, nullptr, v, (std::uint32_t)rnd(),
		    (unsigned long)rnd(), (unsigned char)(rnd() & 0xff),
		    (unsigned int)rnd(), (unsigned int)rnd(),
		    (unsigned char)(rnd() & 0xff), (int)rnd(),
		    (std::uint32_t)rnd(), (std::uint32_t)rnd());
	}
}

static void
kvatoname_case(const char *tag, int optv, int open_fail, P::ipfunc_t func,
    P::ioctlfunc_t ioc)
{
	sync_opts(optv);
	g_open_fail = open_fail;

	char *got = P::kvatoname(func, ioc);
	char *ref = ref_kvatoname(func, ioc);

	st_kvatoname.cases++;
	int got_null = (got == nullptr);
	int ref_null = (ref == nullptr);
	int bad_null = (got_null != ref_null);
	int bad_str = 0;

	if (!got_null && !ref_null)
		bad_str = (std::strcmp(got, ref) != 0);
	else if (got_null != ref_null)
		bad_str = 1;

	if (bad_null || bad_str) {
		stat_fail(&st_kvatoname, tag, bad_null ? "null" : "str");
		if (st_kvatoname.reported <= MAX_REPORT) {
			std::printf("      port=%s ref=%s\n",
			    got_null ? "(null)" : got,
			    ref_null ? "(null)" : ref);
		}
	}
}

static void
test_kvatoname_edges(void)
{
	P::ipfunc_t funcs[] = {
	    (P::ipfunc_t)0,
	    (P::ipfunc_t)1,
	    (P::ipfunc_t)0x80,
	    (P::ipfunc_t)0xdeadbeef,
	    (P::ipfunc_t)-1,
	};

	g_open_fail = 0;
	for (std::size_t i = 0; i < sizeof(funcs) / sizeof(funcs[0]); i++) {
		char tag[48];
		std::snprintf(tag, sizeof(tag), "open%zu", i);
		kvatoname_case(tag, 0, 0, funcs[i], mock_kv_ioctl);
	}

	g_open_fail = 1;
	kvatoname_case("open-fail", 0, 1, (P::ipfunc_t)0x11, mock_kv_ioctl);

	sync_opts(OPT_DONTOPEN);
	g_open_fail = 0;
	kvatoname_case("dontopen", OPT_DONTOPEN, 0, (P::ipfunc_t)0x22,
	    mock_kv_ioctl);
	kvatoname_case("dontopen-fail", OPT_DONTOPEN, 1, (P::ipfunc_t)0x33,
	    mock_kv_ioctl);

	kvatoname_case("long", 0, 0, (P::ipfunc_t)0x44, mock_kv_ioctl_long);
	kvatoname_case("empty", OPT_DONTOPEN, 0, (P::ipfunc_t)0x55,
	    mock_kv_ioctl_empty);
	kvatoname_case("hibyte", OPT_DONTOPEN, 0, (P::ipfunc_t)0x66,
	    mock_kv_ioctl_hibyte);
}

static void
test_kvatoname_sweep(void)
{
	for (long i = 0; i < SWEEP; i++) {
		int optv = ((rnd() & 3) == 0) ? OPT_DONTOPEN : 0;
		int ofail = ((rnd() & 7) == 0 && optv == 0) ? 1 : 0;
		P::ipfunc_t func = (P::ipfunc_t)(std::uintptr_t)rnd();
		P::ioctlfunc_t ioc = mock_kv_ioctl;
		if ((rnd() & 15) == 0)
			ioc = mock_kv_ioctl_long;
		else if ((rnd() & 31) == 0)
			ioc = mock_kv_ioctl_hibyte;
		else if ((rnd() & 63) == 0)
			ioc = mock_kv_ioctl_empty;
		char tag[48];
		std::snprintf(tag, sizeof(tag), "rnd%ld", i);
		kvatoname_case(tag, optv, ofail, func, ioc);
	}
}

static void
printunit_case(const char *tag, int unit)
{
	std::string got = capture_stdout_unit(call_port_printunit, unit);
	std::string ref = capture_stdout_unit(call_ref_printunit, unit);

	st_printunit.cases++;
	if (got != ref) {
		stat_fail(&st_printunit, tag, "stdout");
		if (st_printunit.reported <= MAX_REPORT) {
			std::printf("      port=%s ref=%s\n", got.c_str(),
			    ref.c_str());
		}
	}
}

static void
test_printunit_edges(void)
{
	static const int units[] = {
	    0, 1, 2, 3, 4, 5, 6, 7, -1, -2, 8, 9, -3, 127, -128, INT_MAX,
	    INT_MIN, 0x7f, 0x80, 0xff, -0x80
	};

	for (std::size_t i = 0; i < sizeof(units) / sizeof(units[0]); i++) {
		char tag[48];
		std::snprintf(tag, sizeof(tag), "edge%zu", i);
		printunit_case(tag, units[i]);
	}
}

static void
test_printunit_sweep(void)
{
	for (long i = 0; i < SWEEP; i++) {
		int unit;
		if ((rnd() & 7) == 0)
			unit = (int)(rnd() % 10) - 3;
		else
			unit = (int)rnd();
		char tag[48];
		std::snprintf(tag, sizeof(tag), "rnd%ld", i);
		printunit_case(tag, unit);
	}
}

static void
ipf_perror_case(const char *tag, int err, const char *msg_in)
{
	unsigned char msg_p[128];
	unsigned char msg_r[128];

	std::memset(msg_p, GUARD, sizeof(msg_p));
	std::memset(msg_r, GUARD, sizeof(msg_r));
	if (msg_in != nullptr) {
		std::snprintf((char *)msg_p + 16, sizeof(msg_p) - 16, "%s",
		    msg_in);
	} else {
		fill_str((char *)msg_p + 16, sizeof(msg_p) - 16,
		    (int)(rnd() % 4), rnd_mod(48));
	}
	std::memcpy(msg_r + 16, msg_p + 16, sizeof(msg_r) - 16);

	std::string got = capture_stderr_perror(call_port_ipf_perror, err,
	    (char *)msg_p + 16);
	std::string ref = capture_stderr_perror(call_ref_ipf_perror, err,
	    (char *)msg_r + 16);

	st_ipf_perror.cases++;
	int bad_out = (got != ref);
	int bad_buf = (std::memcmp(msg_p, msg_r, sizeof(msg_p)) != 0);
	if (bad_out || bad_buf)
		stat_fail(&st_ipf_perror, tag, bad_out ? "stderr" : "buf");
}

static void
test_ipf_perror_edges(void)
{
	static const char *msgs[] = { "", "ok", "fail", "\x80\xff", "a\xffb" };
	static const int errs[] = { 0, 1, -1, 40, 127, 255, INT_MAX, INT_MIN };

	for (std::size_t i = 0; i < sizeof(msgs) / sizeof(msgs[0]); i++) {
		for (std::size_t j = 0; j < sizeof(errs) / sizeof(errs[0]);
		     j++) {
			char tag[64];
			std::snprintf(tag, sizeof(tag), "m%zu e%zu", i, j);
			ipf_perror_case(tag, errs[j], msgs[i]);
		}
	}
}

static void
test_ipf_perror_sweep(void)
{
	for (long i = 0; i < SWEEP; i++) {
		int err = (int)rnd();
		if ((rnd() & 7) == 0)
			err = 0;
		char tag[48];
		std::snprintf(tag, sizeof(tag), "rnd%ld", i);
		ipf_perror_case(tag, err, nullptr);
	}
}

static void
ipf_perror_fd_case(const char *tag, int fd, int realerr, int ioc_ret,
    int save_errno, const char *msg_in)
{
	unsigned char msg_p[128];
	unsigned char msg_r[128];

	g_pfd_realerr = realerr;
	g_pfd_ret = ioc_ret;

	std::memset(msg_p, GUARD, sizeof(msg_p));
	std::memset(msg_r, GUARD, sizeof(msg_r));
	if (msg_in != nullptr) {
		std::snprintf((char *)msg_p + 16, sizeof(msg_p) - 16, "%s",
		    msg_in);
	} else {
		fill_str((char *)msg_p + 16, sizeof(msg_p) - 16,
		    (int)(rnd() % 4), rnd_mod(48));
	}
	std::memcpy(msg_r + 16, msg_p + 16, sizeof(msg_r) - 16);

	errno = save_errno;
	PfdResult got = capture_stderr_pfd(P::ipf_perror_fd, fd,
	    mock_pfd_ioctl_global, (char *)msg_p + 16);
	errno = save_errno;
	PfdResult ref = capture_stderr_pfd(ref_ipf_perror_fd, fd,
	    mock_pfd_ioctl_global, (char *)msg_r + 16);

	st_ipf_perror_fd.cases++;
	int bad_ret = (got.ret != ref.ret);
	int bad_out = (got.err != ref.err);
	int bad_buf = (std::memcmp(msg_p, msg_r, sizeof(msg_p)) != 0);
	if (bad_ret || bad_out || bad_buf) {
		stat_fail(&st_ipf_perror_fd, tag,
		    bad_ret ? "ret" : (bad_out ? "stderr" : "buf"));
	}
}

static void
test_ipf_perror_fd_edges(void)
{
	static const int fds[] = { -1, 0, 3, 99 };
	static const int realerrs[] = { 0, 1, 40, 255 };
	static const int ioc_rets[] = { 0, -1 };
	static const int saves[] = { 0, ENOENT, EIO, 0x80 };

	for (std::size_t a = 0; a < sizeof(fds) / sizeof(fds[0]); a++) {
		for (std::size_t b = 0; b < sizeof(realerrs) / sizeof(realerrs[0]);
		     b++) {
			for (std::size_t c = 0;
			     c < sizeof(ioc_rets) / sizeof(ioc_rets[0]); c++) {
				for (std::size_t d = 0;
				     d < sizeof(saves) / sizeof(saves[0]); d++) {
					char tag[80];
					std::snprintf(tag, sizeof(tag),
					    "f%zu r%zu i%zu s%zu", a, b, c, d);
					ipf_perror_fd_case(tag, fds[a],
					    realerrs[b], ioc_rets[c], saves[d],
					    "msg");
				}
			}
		}
	}
}

static void
test_ipf_perror_fd_sweep(void)
{
	for (long i = 0; i < SWEEP; i++) {
		int fd = (int)rnd_mod(8) - 2;
		int realerr = (int)(rnd() & 0xff);
		int ioc_ret = ((rnd() & 3) == 0) ? -1 : 0;
		int save = (int)rnd();
		char tag[48];
		std::snprintf(tag, sizeof(tag), "rnd%ld", i);
		ipf_perror_fd_case(tag, fd, realerr, ioc_ret, save, nullptr);
	}
}

static void
ipferror_case(const char *tag, int fd, int wrap_ret, int wrap_realerr,
    int save_errno, const char *msg_in)
{
	unsigned char msg_p[128];
	unsigned char msg_r[128];

	g_wrapped_ioctl_ret = wrap_ret;
	g_wrapped_ioctl_realerr = wrap_realerr;

	std::memset(msg_p, GUARD, sizeof(msg_p));
	std::memset(msg_r, GUARD, sizeof(msg_r));
	if (msg_in != nullptr) {
		std::snprintf((char *)msg_p + 16, sizeof(msg_p) - 16, "%s",
		    msg_in);
	} else {
		fill_str((char *)msg_p + 16, sizeof(msg_p) - 16,
		    (int)(rnd() % 4), rnd_mod(48));
	}
	std::memcpy(msg_r + 16, msg_p + 16, sizeof(msg_r) - 16);

	errno = save_errno;
	std::string got = capture_stderr_ferror(P::ipferror, fd,
	    (char *)msg_p + 16);
	errno = save_errno;
	std::string ref = capture_stderr_ferror(ref_ipferror, fd,
	    (char *)msg_r + 16);

	st_ipferror.cases++;
	int bad_out = (got != ref);
	int bad_buf = (std::memcmp(msg_p, msg_r, sizeof(msg_p)) != 0);
	if (bad_out || bad_buf)
		stat_fail(&st_ipferror, tag, bad_out ? "stderr" : "buf");
}

static void
test_ipferror_edges(void)
{
	ipferror_case("neg-fd", -1, 0, 0, ENOENT, "neg");
	ipferror_case("neg-fd0", -1, 0, 0, 0, "");
	ipferror_case("pos-fd", 3, 0, 7, EIO, "pos");
	ipferror_case("pos-fd0", 0, -1, 99, 5, "ioctl-fail");
	ipferror_case("pos-err0", 1, 0, 0, 42, "real0");
	ipferror_case("pos-hi", 5, 0, 0xff, 0x80, "\x80\xff");
}

static void
test_ipferror_sweep(void)
{
	for (long i = 0; i < SWEEP; i++) {
		int fd = ((rnd() & 3) == 0) ? -(int)rnd_mod(5) - 1 :
					      (int)rnd_mod(8);
		int wrap_ret = ((rnd() & 7) == 0) ? -1 : 0;
		int wrap_realerr = (int)(rnd() & 0xff);
		int save = (int)rnd();
		char tag[48];
		std::snprintf(tag, sizeof(tag), "rnd%ld", i);
		ipferror_case(tag, fd, wrap_ret, wrap_realerr, save, nullptr);
	}
}

int
main(void)
{
	test_printfraginfo_edges();
	test_printfraginfo_sweep();
	test_kvatoname_edges();
	test_kvatoname_sweep();
	test_printunit_edges();
	test_printunit_sweep();
	test_ipf_perror_edges();
	test_ipf_perror_sweep();
	test_ipf_perror_fd_edges();
	test_ipf_perror_fd_sweep();
	test_ipferror_edges();
	test_ipferror_sweep();

	std::printf("\n%-16s %8s %8s\n", "function", "cases", "fails");
	std::printf("%-16s %8ld %8ld\n", st_printfraginfo.name,
	    st_printfraginfo.cases, st_printfraginfo.fails);
	std::printf("%-16s %8ld %8ld\n", st_kvatoname.name, st_kvatoname.cases,
	    st_kvatoname.fails);
	std::printf("%-16s %8ld %8ld\n", st_printunit.name, st_printunit.cases,
	    st_printunit.fails);
	std::printf("%-16s %8ld %8ld\n", st_ipf_perror.name,
	    st_ipf_perror.cases, st_ipf_perror.fails);
	std::printf("%-16s %8ld %8ld\n", st_ipf_perror_fd.name,
	    st_ipf_perror_fd.cases, st_ipf_perror_fd.fails);
	std::printf("%-16s %8ld %8ld\n", st_ipferror.name, st_ipferror.cases,
	    st_ipferror.fails);

	long total_fails = st_printfraginfo.fails + st_kvatoname.fails +
	    st_printunit.fails + st_ipf_perror.fails + st_ipf_perror_fd.fails +
	    st_ipferror.fails;
	return total_fails == 0 ? 0 : 1;
}
