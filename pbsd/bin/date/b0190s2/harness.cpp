/*
 * harness.cpp -- differential test for PBSD batch b0190s2 (date.c).
 */
#include <sys/time.h>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <clocale>
#include <csetjmp>
#include <fcntl.h>
#include <unistd.h>

import pbsd.bin.date.b0190s2;
namespace P = pbsd::bin_date::b0190s2;

extern "C" {
struct iso8601_fmt { const char *refname; const char *format_string; };
void oracle_err_arm(void);
void oracle_err_disarm(void);
extern jmp_buf oracle_err_jmp;
extern int oracle_err_status;
void ref_test_set_iso8601_selected(const struct iso8601_fmt *);
const struct iso8601_fmt *ref_test_iso8601_fmts(void);
int ref_test_iso8601_fmts_count(void);
void ref_printdate(const char *);
void ref_printisodate(struct tm *, long, long);
void ref_setthetime(const char *, const char *, int, struct timespec *);
size_t ref_strftime_ns(char *, size_t, const char *, const struct tm *, long, long);
void ref_badformat(void);
void ref_iso8601_usage(const char *);
void ref_multipleformats(void);
void ref_usage(void);
}

static jmp_buf g_exit_jmp;
static volatile int g_exit_armed;
static volatile int g_exit_status;
static int cap_out_fd = -1, cap_err_fd = -1, saved_out = -1, saved_err = -1;

static void cap_restore_force()
{
	fflush(nullptr);
	if (saved_out >= 0) {
		dup2(saved_out, 1);
		close(saved_out);
		saved_out = -1;
	}
	if (saved_err >= 0) {
		dup2(saved_err, 2);
		close(saved_err);
		saved_err = -1;
	}
	if (cap_out_fd >= 0) {
		close(cap_out_fd);
		cap_out_fd = -1;
	}
	if (cap_err_fd >= 0) {
		close(cap_err_fd);
		cap_err_fd = -1;
	}
}

extern "C" void __wrap_exit(int status)
{
	if (g_exit_armed) {
		g_exit_status = status;
		g_exit_armed = 0;
		cap_restore_force();
		longjmp(g_exit_jmp, 1);
	}
	fflush(nullptr);
	_exit(status);
}

extern "C" int __wrap_clock_settime(clockid_t, const struct timespec *) { return 0; }
extern "C" struct utmpx *__wrap_pututxline(struct utmpx *u) { return u; }
extern "C" int __wrap_gettimeofday(struct timeval *tv, void *) {
	if (tv) { tv->tv_sec = 1700000000; tv->tv_usec = 0; }
	return 0;
}
extern "C" char *__wrap_getlogin(void) { return const_cast<char*>("harness"); }
extern "C" void __wrap_syslog(int, const char *, ...) {}

constexpr long SWEEP = 200000L;
constexpr int MAX_SHOW = 8;
constexpr unsigned char GUARD = 0x7f;
constexpr int BUF_SZ = 256;

struct Stat { const char *name; long cases, fails; int shown; };
Stat stats[16];
int nstats = 0;

struct Rng {
	std::uint64_t s;
	explicit Rng(std::uint64_t seed) : s(seed) {}
	std::uint64_t next() {
		s += 0x9E3779B97F4A7C15ull;
		std::uint64_t z = s;
		z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
		z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
		return z ^ (z >> 31);
	}
	int bits(int lo, int hi) {
		if (hi <= lo) return lo;
		return lo + (int)(next() % (std::uint64_t)(hi - lo + 1));
	}
	unsigned char byte() { return (unsigned char)(next() & 0xffu); }
	bool coin() { return (next() & 1u) != 0; }
};

Rng rng(0xB01902A0ull);

Stat &reg(const char *name) {
	stats[nstats++] = Stat{name, 0, 0, 0};
	return stats[nstats - 1];
}

void fail(Stat &st, const char *what) {
	st.fails++;
	if (st.shown < MAX_SHOW) {
		st.shown++;
		std::fprintf(stderr, "  FAIL %s: %s\n", st.name, what);
	}
}

void fill_tm(struct tm *t, int sec, int min, int hour, int mday, int mon,
    int year, int wday, int isdst) {
	std::memset(t, 0, sizeof(*t));
	t->tm_sec = sec; t->tm_min = min; t->tm_hour = hour;
	t->tm_mday = mday; t->tm_mon = mon; t->tm_year = year;
	t->tm_wday = wday; t->tm_isdst = isdst;
}

static void cap_begin(char *ob, size_t osz, char *eb, size_t esz) {
	int pout[2], perr[2];
	pipe(pout); pipe(perr);
	saved_out = dup(1); saved_err = dup(2);
	dup2(pout[1], 1); dup2(perr[1], 2);
	close(pout[1]); close(perr[1]);
	cap_out_fd = pout[0]; cap_err_fd = perr[0];
	if (osz) ob[0] = '\0';
	if (esz) eb[0] = '\0';
}

static void cap_end(char *ob, size_t osz, char *eb, size_t esz) {
	fflush(nullptr);
	if (saved_out >= 0) { dup2(saved_out, 1); close(saved_out); saved_out = -1; }
	if (saved_err >= 0) { dup2(saved_err, 2); close(saved_err); saved_err = -1; }
	if (cap_out_fd >= 0) {
		ssize_t n = read(cap_out_fd, ob, osz - 1);
		if (n < 0) n = 0; ob[n] = '\0'; close(cap_out_fd); cap_out_fd = -1;
	}
	if (cap_err_fd >= 0) {
		ssize_t n = read(cap_err_fd, eb, esz - 1);
		if (n < 0) n = 0; eb[n] = '\0'; close(cap_err_fd); cap_err_fd = -1;
	}
}

static void iso8601_select(int idx) {
	const struct iso8601_fmt *fmts = ref_test_iso8601_fmts();
	int n = ref_test_iso8601_fmts_count();
	if (idx < 0) idx = 0;
	if (idx >= n) idx = n - 1;
	ref_test_set_iso8601_selected(&fmts[idx]);
	P::test_set_iso8601_selected(reinterpret_cast<const P::iso8601_fmt*>(&fmts[idx]));
}

void
test_strftime_ns()
{
	Stat &st = reg("strftime_ns");

	struct tm lt;
	fill_tm(&lt, 30, 15, 12, 15, 5, 120, 3, -1);
	mktime(&lt);

	auto run = [&](const char *fmt, long nsec, long res, size_t maxsize) {
		unsigned char rbuf[BUF_SZ + 32], pbuf[BUF_SZ + 32];
		std::memset(rbuf, GUARD, sizeof(rbuf));
		std::memset(pbuf, GUARD, sizeof(pbuf));
		st.cases++;
		size_t rr = ref_strftime_ns((char *)(rbuf + 16), maxsize, fmt, &lt,
		    nsec, res);
		size_t pr = P::strftime_ns((char *)(pbuf + 16), maxsize, fmt, &lt,
		    nsec, res);
		if (rr != pr)
			fail(st, "return");
		else if (std::memcmp(rbuf, pbuf, sizeof(rbuf)) != 0)
			fail(st, "buffer");
	};

	run("", 0, 1, 64);
	run("%Y-%m-%d", 0, 1, 64);
	run("%N", 123456789L, 1, 64);
	run("%%N", 123456789L, 1, 64);
	run("%-N", 123456789L, 1000000L, 64);
	run("%-N", 123456789L, 1L, 64);
	run("%5N", 123456789L, 1, 64);
	run("%09N", 123456789L, 1, 64);
	run("%12N", 123456789L, 1, 64);
	run("%Y-%m-%d %N", 999999999L, 1, 128);
	run("%3N", 123456789L, 1, 64);
	run("%1N", 987654321L, 1, 64);
	run("prefix%Nsuffix", 42L, 1, 128);
	run("%-5N", 100L, 1000L, 64);
	run("%-10N", 555555555L, 1000000000L, 64);
	run("%z %N", 0L, 1, 64);
	run("%", 0L, 1, 64);
	run("%%", 0L, 1, 64);
	run("%0N", 0L, 1, 64);
	run("%99N", 1L, 1, 256);

	for (long i = 0; i < SWEEP / 2; i++) {
		char fmt[64];
		int flen = rng.bits(0, 40);
		int fp = 0;
		for (int j = 0; j < flen; j++) {
			unsigned char c = rng.byte();
			if (c == '\0')
				c = 'x';
			fmt[fp++] = (char)c;
			if (c == '%' && fp < 60 && rng.coin()) {
				static const char *spec = "NY%-0123456789";
				fmt[fp++] = spec[rng.bits(0, 13)];
			}
		}
		fmt[fp] = '\0';
		long nsec = (long)(rng.next() % 1000000000ull);
		long res = (long)rng.bits(1, 9) * 100000000L;
		if (res <= 0)
			res = 1;
		size_t ms = (size_t)rng.bits(0, BUF_SZ);
		run(fmt, nsec, res, ms);
	}
}


void test_setthetime() {
	Stat &st = reg("setthetime");
	auto run_ok = [&](const char *fmt, const char *p, int jflag, time_t base) {
		struct timespec rts = {base, 123}, pts = {base, 123};
		int re = 0, pe = 0, oe = 0;
		st.cases++;
		oracle_err_arm();
		if (setjmp(oracle_err_jmp) == 0) {
			if (setjmp(g_exit_jmp) == 0) {
				g_exit_armed = 1;
				ref_setthetime(fmt, p, jflag, &rts);
				g_exit_armed = 0;
			} else {
				oe = 1;
			}
		} else {
			re = 1;
		}
		oracle_err_disarm();
		P::port_err_arm();
		if (setjmp(P::port_err_jmp) == 0) {
			if (setjmp(g_exit_jmp) == 0) {
				g_exit_armed = 1;
				P::setthetime(fmt, p, jflag, &pts);
				g_exit_armed = 0;
			} else {
				oe = 1;
			}
		} else {
			pe = 1;
		}
		P::port_err_disarm();
		if (oe)
			fail(st, "unexpected exit");
		else if (re != pe)
			fail(st, "errx mismatch");
		else if (!re && (rts.tv_sec != pts.tv_sec || rts.tv_nsec != pts.tv_nsec))
			fail(st, "timespec");
	};
	auto run_err = [&](const char *fmt, const char *p, int jflag, time_t base) {
		struct timespec rts = {base, 0}, pts = {base, 0};
		int re = 0, pe = 0, rs = 0, ps = 0;
		st.cases++;
		oracle_err_arm();
		if (setjmp(oracle_err_jmp) == 0) {
			if (setjmp(g_exit_jmp) == 0) {
				g_exit_armed = 1;
				ref_setthetime(fmt, p, jflag, &rts);
				g_exit_armed = 0;
			} else { re = 1; rs = g_exit_status; }
		} else { re = 2; rs = oracle_err_status; }
		oracle_err_disarm();
		P::port_err_arm();
		if (setjmp(P::port_err_jmp) == 0) {
			if (setjmp(g_exit_jmp) == 0) {
				g_exit_armed = 1;
				P::setthetime(fmt, p, jflag, &pts);
				g_exit_armed = 0;
			} else { pe = 1; ps = g_exit_status; }
		} else { pe = 2; ps = P::port_err_status; }
		P::port_err_disarm();
		if (re != pe || rs != ps) fail(st, "error");
	};
	time_t base = 1704067200;
	run_ok(nullptr, "2312151530", 1, base);
	run_ok(nullptr, "2312151530.30", 1, base);
	run_ok(nullptr, "042312151530", 1, base);
	run_ok(nullptr, "240412121530", 1, base);
	run_ok(nullptr, "202404121530", 1, base);
	run_ok("%Y%m%d%H%M.%S", "202404121530.30", 1, base);
	run_ok(nullptr, "2312151530", 0, base);
	run_err(nullptr, "bad", 1, base);
	run_err(nullptr, "9913010000", 1, base);
	run_err(nullptr, "9912312460", 1, base);
	for (long i = 0; i < SWEEP / 100; i++) {
		char d[24];
		std::snprintf(d, sizeof(d), "%04d%02d%02d%02d%02d",
		    2000 + (int)(i % 24), 1 + (int)(i % 12), 1 + (int)(i % 28),
		    (int)(i % 24), (int)(i % 60));
		run_ok(nullptr, d, 1, (time_t)(1700000000 + i));
	}
}

void test_printdate() {
	Stat &st = reg("printdate");
	auto run = [&](const char *buf) {
		char rout[256], pout[256], rerr[256], perr[256];
		int re = 0, pe = 0, rs = 0, ps = 0;
		st.cases++;
		cap_begin(rout, sizeof(rout), rerr, sizeof(rerr));
		if (setjmp(g_exit_jmp) == 0) { g_exit_armed = 1; ref_printdate(buf); }
		else { re = 1; rs = g_exit_status; }
		g_exit_armed = 0;
		cap_end(rout, sizeof(rout), rerr, sizeof(rerr));
		cap_begin(pout, sizeof(pout), perr, sizeof(perr));
		if (setjmp(g_exit_jmp) == 0) { g_exit_armed = 1; P::printdate(buf); }
		else { pe = 1; ps = g_exit_status; }
		g_exit_armed = 0;
		cap_end(pout, sizeof(pout), perr, sizeof(perr));
		if (re != pe || rs != ps) fail(st, "exit");
		else if (std::strcmp(rout, pout) != 0) fail(st, "stdout");
	};
	run(""); run("hello");
	for (long i = 0; i < SWEEP / 20; i++) {
		char b[128]; int n = rng.bits(0, 120);
		for (int j = 0; j < n; j++) b[j] = (char)(rng.byte() == 0 ? 'a' : rng.byte());
		b[n] = '\0'; run(b);
	}
}

void test_printisodate() {
	Stat &st = reg("printisodate");
	struct tm lt; fill_tm(&lt, 30, 15, 12, 15, 3, 124, 1, 0); mktime(&lt);
	auto run = [&](int sel, long nsec, long res) {
		char rout[512], pout[512], rerr[512], perr[512];
		int re = 0, pe = 0, rs = 0, ps = 0;
		st.cases++;
		iso8601_select(sel);
		cap_begin(rout, sizeof(rout), rerr, sizeof(rerr));
		if (setjmp(g_exit_jmp) == 0) { g_exit_armed = 1; ref_printisodate(&lt, nsec, res); }
		else { re = 1; rs = g_exit_status; }
		g_exit_armed = 0;
		cap_end(rout, sizeof(rout), rerr, sizeof(rerr));
		iso8601_select(sel);
		cap_begin(pout, sizeof(pout), perr, sizeof(perr));
		if (setjmp(g_exit_jmp) == 0) { g_exit_armed = 1; P::printisodate(&lt, nsec, res); }
		else { pe = 1; ps = g_exit_status; }
		g_exit_armed = 0;
		cap_end(pout, sizeof(pout), perr, sizeof(perr));
		if (re != pe || rs != ps) fail(st, "exit");
		else if (std::strcmp(rout, pout) != 0) fail(st, "stdout");
	};
	for (int i = 0; i < ref_test_iso8601_fmts_count(); i++) run(i, 0L, 1L);
	for (long i = 0; i < SWEEP / 10; i++)
		run(rng.bits(0, ref_test_iso8601_fmts_count() - 1),
		    (long)(rng.next() % 1000000000ull), (long)rng.bits(1, 9) * 100000000L);
}

void test_badformat() {
	Stat &st = reg("badformat");
	auto run = [&]() {
		char rerr[1024], perr[1024], rout[64], pout[64];
		int re = 0, pe = 0, rs = 0, ps = 0;
		st.cases++;
		cap_begin(rout, sizeof(rout), rerr, sizeof(rerr));
		if (setjmp(g_exit_jmp) == 0) { g_exit_armed = 1; ref_badformat(); }
		else { re = 1; rs = g_exit_status; }
		g_exit_armed = 0;
		cap_end(rout, sizeof(rout), rerr, sizeof(rerr));
		cap_begin(pout, sizeof(pout), perr, sizeof(perr));
		if (setjmp(g_exit_jmp) == 0) { g_exit_armed = 1; P::badformat(); }
		else { pe = 1; ps = g_exit_status; }
		g_exit_armed = 0;
		cap_end(pout, sizeof(pout), perr, sizeof(perr));
		if (re != pe || rs != ps) fail(st, "exit");
		else if (std::strcmp(rerr, perr) != 0) fail(st, "stderr");
	};
	for (long i = 0; i < SWEEP / 40; i++) run();
}

void test_iso8601_usage() {
	Stat &st = reg("iso8601_usage");
	auto run = [&](const char *arg) {
		char rerr[256], perr[256];
		int re = 0, pe = 0, rs = 0, ps = 0;
		st.cases++;
		cap_begin(rerr, sizeof(rerr), perr, sizeof(perr));
		oracle_err_arm();
		if (setjmp(oracle_err_jmp) == 0) ref_iso8601_usage(arg);
		else { re = 1; rs = oracle_err_status; }
		oracle_err_disarm();
		cap_end(rerr, sizeof(rerr), perr, sizeof(perr));
		char perr2[256], rerr2[256];
		cap_begin(perr2, sizeof(perr2), rerr2, sizeof(rerr2));
		P::port_err_arm();
		if (setjmp(P::port_err_jmp) == 0) P::iso8601_usage(arg);
		else { pe = 1; ps = P::port_err_status; }
		P::port_err_disarm();
		cap_end(perr2, sizeof(perr2), rerr2, sizeof(rerr2));
		if (re != pe || rs != ps) fail(st, "errx");
		else if (std::strcmp(rerr, perr2) != 0) fail(st, "stderr");
	};
	run("bogus");
	for (long i = 0; i < SWEEP / 40; i++) {
		char a[32]; int n = rng.bits(0, 20);
		for (int j = 0; j < n; j++) a[j] = (char)rng.byte();
		a[n] = '\0'; run(a);
	}
}

void test_multipleformats() {
	Stat &st = reg("multipleformats");
	auto run = [&]() {
		char rerr[256], perr[256];
		int re = 0, pe = 0, rs = 0, ps = 0;
		st.cases++;
		cap_begin(rerr, sizeof(rerr), perr, sizeof(perr));
		oracle_err_arm();
		if (setjmp(oracle_err_jmp) == 0) ref_multipleformats();
		else { re = 1; rs = oracle_err_status; }
		oracle_err_disarm();
		cap_end(rerr, sizeof(rerr), perr, sizeof(perr));
		char perr2[256], rerr2[256];
		cap_begin(perr2, sizeof(perr2), rerr2, sizeof(rerr2));
		P::port_err_arm();
		if (setjmp(P::port_err_jmp) == 0) P::multipleformats();
		else { pe = 1; ps = P::port_err_status; }
		P::port_err_disarm();
		cap_end(perr2, sizeof(perr2), rerr2, sizeof(rerr2));
		if (re != pe || rs != ps) fail(st, "errx");
	};
	for (long i = 0; i < SWEEP / 40; i++) run();
}

void test_usage() {
	Stat &st = reg("usage");
	auto run = [&]() {
		char rerr[1024], perr[1024], rout[64], pout[64];
		int re = 0, pe = 0, rs = 0, ps = 0;
		st.cases++;
		cap_begin(rout, sizeof(rout), rerr, sizeof(rerr));
		if (setjmp(g_exit_jmp) == 0) { g_exit_armed = 1; ref_usage(); }
		else { re = 1; rs = g_exit_status; }
		g_exit_armed = 0;
		cap_end(rout, sizeof(rout), rerr, sizeof(rerr));
		cap_begin(pout, sizeof(pout), perr, sizeof(perr));
		if (setjmp(g_exit_jmp) == 0) { g_exit_armed = 1; P::usage(); }
		else { pe = 1; ps = g_exit_status; }
		g_exit_armed = 0;
		cap_end(pout, sizeof(pout), perr, sizeof(perr));
		if (re != pe || rs != ps) fail(st, "exit");
		else if (std::strcmp(rerr, perr) != 0) fail(st, "stderr");
	};
	for (long i = 0; i < SWEEP / 40; i++) run();
}



int main() {
	setenv("TZ", "UTC0", 1);
	tzset();
	(void)setlocale(LC_TIME, "C");
	cap_restore_force();
	test_strftime_ns();
	test_setthetime();
	test_printdate();
	test_printisodate();
	test_badformat();
	test_iso8601_usage();
	test_multipleformats();
	test_usage();
	long tc = 0, tf = 0;
	std::printf("\n%-16s %10s %10s\n", "function", "cases", "failures");
	std::printf("%-16s %10s %10s\n", "--------", "-----", "--------");
	for (int i = 0; i < nstats; i++) {
		std::printf("%-16s %10ld %10ld\n", stats[i].name, stats[i].cases, stats[i].fails);
		tc += stats[i].cases; tf += stats[i].fails;
	}
	std::printf("%-16s %10ld %10ld\n", "TOTAL", tc, tf);
	return tf == 0 ? 0 : 1;
}
