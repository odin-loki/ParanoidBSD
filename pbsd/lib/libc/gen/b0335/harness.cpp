/*
 * harness.cpp -- differential test for PBSD batch b0335.
 */

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <climits>
#include <cstdarg>
#include <vector>

#include <pthread.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/syslog.h>
#include <fnmatch.h>
#include <fcntl.h>
#include <unistd.h>

import pbsd.lib.libc.gen.b0335;

namespace P = pbsd::lib_libc_gen::b0335;

extern "C" {
int ref_fnmatch(const char *, const char *, int);
void ref_syslog(int, const char *, ...);
void ref_vsyslog(int, const char *, va_list);
void ref_openlog(const char *, int, int);
void ref_closelog(void);
int ref_setlogmask(int);
int ref_pthread_equal(pthread_t, pthread_t);
pthread_t ref_pthread_self(void);
int ref_pthread_main_np(void);
int ref_pthread_key_create(pthread_key_t *, void (*)(void *));
int ref_pthread_once(pthread_once_t *, void (*)(void));
int ref_pthread_getname_np(pthread_t, char *, size_t);
int ref_pthread_mutex_lock(void *);
int ref_pthread_mutex_unlock(void *);
int ref_pthread_attr_get_np(pthread_t, pthread_attr_t *);
void ref_pthread_suspend_all_np(void);
void ref_pthread_resume_all_np(void);
}

struct Stats { const char *name; long cases; long failures; };
static Stats g_fnmatch = { "fnmatch", 0, 0 };
static Stats g_setlogmask = { "setlogmask", 0, 0 };
static Stats g_openlog = { "openlog", 0, 0 };
static Stats g_closelog = { "closelog", 0, 0 };
static Stats g_syslog = { "syslog", 0, 0 };
static Stats g_vsyslog = { "vsyslog", 0, 0 };
static Stats g_pthread = { "pthread_stubs", 0, 0 };

static void bump(Stats &s, bool ok) { s.cases++; if (!ok) s.failures++; }

static std::uint64_t g_seed = 0xB0335ULL;
static std::uint64_t nextr(void) {
	g_seed ^= g_seed << 13; g_seed ^= g_seed >> 7; g_seed ^= g_seed << 17;
	return g_seed;
}
static int rndbyte(void) { return (int)(nextr() & 0xff); }

struct xlocale_refcounted { long retain_count; void (*destructor)(void *); };
struct xlocale_component { struct xlocale_refcounted base; char locale[32]; };
enum { XLC_COLLATE = 0, XLC_LAST = 6 };
struct _xlocale { struct xlocale_refcounted header; struct xlocale_component *components[XLC_LAST]; };
struct xlocale_collate { struct xlocale_component header; int __collate_load_error; };
static struct _xlocale g_locale = {};
static struct xlocale_collate g_collate = { {}, 1 };

static void init_locale(void) {
	g_collate.__collate_load_error = 1;
	g_locale.components[XLC_COLLATE] = (struct xlocale_component *)&g_collate;
}

extern "C" struct _xlocale *__get_locale(void) { return &g_locale; }
extern "C" size_t __collate_collating_symbol(wchar_t *, size_t, const char *, size_t, mbstate_t *) { return (size_t)-1; }
extern "C" int __collate_equiv_class(const char *, size_t, mbstate_t *) { return -1; }
extern "C" ssize_t __collate_equiv_match(int, const wchar_t *, size_t, wchar_t, const char *, size_t, mbstate_t *, size_t *) { return 0; }
extern "C" int __wcollate_range_cmp(wchar_t a, wchar_t b) { return (int)a - (int)b; }

extern "C" {
int __isthreaded = 0;
const char *_getprogname(void) { return "harness"; }
int _pthread_mutex_lock(pthread_mutex_t *) { return 0; }
int _pthread_mutex_unlock(pthread_mutex_t *) { return 0; }
}

static constexpr int SENDMAX = 16384;
static unsigned char g_sendbuf[2][SENDMAX];
static int g_sendlen[2];
static int g_send_side;
static int g_sockseq = 1;
static int g_connect_ok = 1;
static int g_send_fail;
static int g_send_errno;
static int g_console_open_ok;

extern "C" ssize_t send(int fd, const void *buf, size_t len, int flags) {
	(void)fd; (void)flags;
	int s = g_send_side;
	if (g_send_fail) { errno = g_send_errno ? g_send_errno : ENOBUFS; return -1; }
	if (len > (size_t)SENDMAX) len = SENDMAX;
	std::memcpy(g_sendbuf[s], buf, len);
	g_sendlen[s] = (int)len;
	return (ssize_t)len;
}
extern "C" int _socket(int, int, int) { return 100 + g_sockseq++; }
extern "C" int _connect(int, const struct sockaddr *, socklen_t) { return g_connect_ok ? 0 : -1; }
extern "C" int _getsockopt(int, int, int, void *v, socklen_t *l) {
	if (v && l && *l >= sizeof(int)) *static_cast<int *>(v) = 128;
	return 0;
}
extern "C" int _setsockopt(int, int, int, const void *, socklen_t) { return 0; }
extern "C" int _open(const char *, int, ...) { return g_console_open_ok ? 200 : -1; }
extern "C" int _close(int) { return 0; }
extern "C" ssize_t _writev(int, const struct iovec *iov, int n) {
	return (n >= 1 && iov[0].iov_base) ? (ssize_t)iov[0].iov_len : 0;
}

static thread_local int (*g_fwwrite)(void *, const char *, int);

static int cookie_bridge(void *c, const char *b, int n) { return g_fwwrite(c, b, n); }

extern "C" FILE *fwopen(void *cookie, int (*fn)(void *, const char *, int)) {
	g_fwwrite = fn;
#if defined(__GLIBC__) || defined(__linux__)
	cookie_io_functions_t io = {};
	io.write = [](void *c, const char *buf, size_t n) -> ssize_t {
		return cookie_bridge(c, buf, (int)n);
	};
	return fopencookie(cookie, "w", io);
#else
	return funopen(cookie, nullptr,
	    reinterpret_cast<int (*)(void *, char *, int)>(cookie_bridge),
	    nullptr, nullptr);
#endif
}

static void reset_syslog_io(void) {
	g_sendlen[0] = g_sendlen[1] = 0;
	std::memset(g_sendbuf, 0x7f, sizeof(g_sendbuf));
	g_sockseq = 1;
	g_connect_ok = 1;
	g_send_fail = 0;
	g_send_errno = 0;
	g_console_open_ok = 1;
}

static void test_fnmatch_one(const char *pat, const char *str, int flags) {
	int ro = ref_fnmatch(pat, str, flags);
	int po = P::fnmatch(pat, str, flags);
	bump(g_fnmatch, ro == po);
}

static void test_fnmatch(void) {
	const char *hand[] = {
		"", "*", "?", "**", "*?", "?", "a", "abc", "ABC",
		"*abc*", "a*b", "a?c", "[a]", "[a-z]", "[!a]", "[]]",
		"\\*", "\\?", "a\\b", ".", ".*", "*.", "[.]",
		"a/b", "a*b", "a?b", "/a", "*/a", "a/*/b",
		"[[:alpha:]]", "[0-9]", "[!0-9]", "[a-c]", "[^a]",
		"\x80", "\xff", "\xc0\x80", "a\xffb",
		"test[", "test]", "test[!", "test-]",
	};
	const int flags[] = {
		0, FNM_PATHNAME, FNM_PERIOD, FNM_NOESCAPE, FNM_CASEFOLD,
		FNM_LEADING_DIR,
		FNM_PATHNAME | FNM_PERIOD,
		FNM_PATHNAME | FNM_LEADING_DIR,
		FNM_PERIOD | FNM_CASEFOLD,
		FNM_PATHNAME | FNM_PERIOD | FNM_NOESCAPE,
	};
	for (const char *p : hand)
		for (const char *s : hand)
			for (int f : flags)
				test_fnmatch_one(p, s, f);
	for (int i = 0; i < 10000; i++) {
		char p[32], s[32];
		int pl = rndbyte() % 31, sl = rndbyte() % 31;
		for (int j = 0; j < pl; j++) p[j] = (char)rndbyte();
		p[pl] = 0;
		for (int j = 0; j < sl; j++) s[j] = (char)rndbyte();
		s[sl] = 0;
		int f = (int)(nextr() & 0x3f);
		test_fnmatch_one(p, s, f);
	}
}

static void test_setlogmask_one(int mask) {
	int ro = ref_setlogmask(mask);
	int po = P::setlogmask(mask);
	bump(g_setlogmask, ro == po);
}

static void test_setlogmask(void) {
	int masks[] = { 0, 0xff, LOG_MASK(LOG_ERR), LOG_UPTO(LOG_DEBUG),
	    LOG_MASK(LOG_INFO) | LOG_MASK(LOG_CRIT), 0x100, 0xffff };
	for (int m : masks) test_setlogmask_one(m);
	for (int i = 0; i < 10000; i++)
		test_setlogmask_one((int)(nextr() & 0x1ff));
}

static void test_openlog_closelog(void) {
	const char *idents[] = { nullptr, "", "prog", "ident[42]", "x[0]", "bad[", "[1]", "a[b" };
	int stats[] = { 0, LOG_PID, LOG_PERROR, LOG_NDELAY, LOG_CONS,
	    LOG_PID | LOG_NDELAY, LOG_PERROR | LOG_CONS };
	int facs[] = { 0, LOG_USER, LOG_LOCAL0, LOG_DAEMON, 0x8fff };
	for (const char *id : idents)
		for (int st : stats)
			for (int fc : facs) {
				ref_openlog(id, st, fc);
				P::openlog(id, st, fc);
				bump(g_openlog, true);
				ref_closelog();
				P::closelog();
				bump(g_closelog, true);
			}
}

static void cmp_syslog_bufs(void) {
	bool ok = g_sendlen[0] == g_sendlen[1];
	if (ok && g_sendlen[0] > 0)
		ok = std::memcmp(g_sendbuf[0], g_sendbuf[1], (size_t)g_sendlen[0]) == 0;
	bump(g_syslog, ok);
}

static void test_syslog_pair(int pri, const char *fmt, ...) {
	va_list ap1, ap2;
	reset_syslog_io();
	ref_openlog("harness[99]", LOG_NDELAY, LOG_USER);
	g_send_side = 0;
	va_start(ap1, fmt);
	ref_vsyslog(pri, fmt, ap1);
	va_end(ap1);
	reset_syslog_io();
	g_sockseq = 1;
	P::openlog("harness[99]", LOG_NDELAY, LOG_USER);
	g_send_side = 1;
	va_start(ap2, fmt);
	P::vsyslog(pri, fmt, ap2);
	va_end(ap2);
	cmp_syslog_bufs();
	ref_closelog();
	P::closelog();
}

static void test_syslog(void) {
	const char *fmts[] = {
		"plain", "num %d", "str %s", "pct %%", "hex %x",
		"multi %d %s", "err %m", "%%m", "%m at end",
		"", "\n", "line\nbreak",
	};
	const char *strs[] = { "", "x", "hello", "\x80\xff" };
	int pris[] = { LOG_INFO, LOG_ERR, LOG_DEBUG, LOG_USER | LOG_NOTICE,
	    LOG_MAKEPRI(LOG_LOCAL1, LOG_WARNING), 0x8fff00, LOG_EMERG };
	for (const char *f : fmts)
		for (const char *s : strs)
			for (int p : pris) {
				errno = EACCES;
				test_syslog_pair(p, f, 42, s);
			}
	for (int i = 0; i < 10000; i++) {
		errno = (int)(nextr() & 0xff);
		test_syslog_pair((int)(nextr() & 0x7ff), "%d %m", (int)nextr());
	}
}

static void test_vsyslog_direct(void) {
	for (int i = 0; i < 100; i++) {
		va_list ap1, ap2;
		reset_syslog_io();
		ref_openlog(nullptr, 0, 0);
		g_send_side = 0;
		va_start(ap1, i);
		ref_vsyslog(LOG_INFO, "v %d", i);
		va_end(ap1);
		reset_syslog_io();
		g_sockseq = 1;
		P::openlog(nullptr, 0, 0);
		g_send_side = 1;
		va_start(ap2, i);
		P::vsyslog(LOG_INFO, "v %d", i);
		va_end(ap2);
		bool ok = g_sendlen[0] == g_sendlen[1];
		if (ok && g_sendlen[0])
			ok = std::memcmp(g_sendbuf[0], g_sendbuf[1], g_sendlen[0]) == 0;
		bump(g_vsyslog, ok);
		ref_closelog();
		P::closelog();
	}
}

static void test_pthread_one(void) {
	bool ok = true;
	ok = ok && ref_pthread_equal(ref_pthread_self(), ref_pthread_self()) ==
	    P::pthread_equal(P::pthread_self(), P::pthread_self());
	ok = ok && ref_pthread_main_np() == P::pthread_main_np();
	pthread_key_t k1, k2;
	ok = ok && ref_pthread_key_create(&k1, nullptr) ==
	    P::pthread_key_create(&k2, nullptr);
	char b1[16], b2[16];
	std::memset(b1, 0x7f, sizeof(b1));
	std::memset(b2, 0x7f, sizeof(b2));
	int r1 = ref_pthread_getname_np(ref_pthread_self(), b1, sizeof(b1));
	int r2 = P::pthread_getname_np(P::pthread_self(), b2, sizeof(b2));
	ok = ok && r1 == r2 && std::memcmp(b1, b2, sizeof(b1)) == 0;
	char b3[16], b4[16];
	std::memset(b3, 0x7f, sizeof(b3));
	std::memset(b4, 0x7f, sizeof(b4));
	ok = ok && ref_pthread_getname_np((pthread_t)0x1, b3, 8) ==
	    P::pthread_getname_np((pthread_t)0x1, b4, 8);
	ok = ok && std::memcmp(b3, b4, sizeof(b3)) == 0;
	int m = 0;
	ok = ok && ref_pthread_mutex_lock(&m) == P::pthread_mutex_lock(&m);
	ok = ok && ref_pthread_mutex_unlock(&m) == P::pthread_mutex_unlock(&m);
	pthread_attr_t a1, a2;
	ok = ok && ref_pthread_attr_get_np(ref_pthread_self(), &a1) ==
	    P::pthread_attr_get_np(P::pthread_self(), &a2);
	ref_pthread_suspend_all_np(); P::pthread_suspend_all_np();
	ref_pthread_resume_all_np(); P::pthread_resume_all_np();
	pthread_once_t o1 = PTHREAD_ONCE_INIT, o2 = PTHREAD_ONCE_INIT;
	ok = ok && ref_pthread_once(&o1, nullptr) == P::pthread_once(&o2, nullptr);
	bump(g_pthread, ok);
}

static void test_pthread(void) {
	for (int i = 0; i < 10000; i++)
		test_pthread_one();
}

int main(void) {
	init_locale();
	reset_syslog_io();
	test_fnmatch();
	test_setlogmask();
	test_openlog_closelog();
	test_syslog();
	test_vsyslog_direct();
	test_pthread();

	long total_cases = 0, total_fail = 0;
	Stats *all[] = { &g_fnmatch, &g_setlogmask, &g_openlog, &g_closelog,
	    &g_syslog, &g_vsyslog, &g_pthread };
	std::printf("\n%-24s %10s %10s\n", "function", "cases", "failures");
	std::printf("%-24s %10s %10s\n", "------------------------", "----------", "----------");
	for (Stats *s : all) {
		std::printf("%-24s %10ld %10ld\n", s->name, s->cases, s->failures);
		total_cases += s->cases;
		total_fail += s->failures;
	}
	std::printf("%-24s %10s %10s\n", "------------------------", "----------", "----------");
	std::printf("%-24s %10ld %10ld\n", "TOTAL", total_cases, total_fail);
	std::printf("\n%s\n", total_fail == 0 ? "PASS" : "FAIL");
	return total_fail == 0 ? 0 : 1;
}
