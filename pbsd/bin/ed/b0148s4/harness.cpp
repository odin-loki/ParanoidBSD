/*
 * harness.cpp -- differential test for PBSD batch b0148s4 (buf.c).
 */

import pbsd.bin.ed.b0148s4;

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/wait.h>
#include <unistd.h>

namespace port = pbsd::bin_ed::b0148s4;

extern "C" {
extern long addr_last;
extern long current_addr;
extern const char *errmsg;
extern int oracle_malloc_fail_at;
extern int oracle_malloc_calls;
extern int oracle_quit_called;
extern int oracle_quit_status;
void oracle_reset_batch(void);
void oracle_quit_enter(void);
char *ref_get_sbuf_line(void *);
const char *ref_put_sbuf_line(const char *);
void ref_add_line_node(void *);
long ref_get_line_node_addr(void *);
void *ref_get_addressed_line_node(long);
int ref_open_sbuf(void);
int ref_close_sbuf(void);
void ref_init_buffers(void);
void ref_quit(int);
char *ref_translit_text(char *, int, int, int);
}

namespace {

constexpr unsigned char GUARD = 0x7f;
constexpr long RANDOM_ITERS = 210000;

struct Stat { const char *name; long cases, fails; };
Stat stats[16];
int nstats;

std::uint64_t rng = 0xb01484deadbeefULL;

std::uint64_t rnd()
{
	std::uint64_t z = (rng += 0x9e3779b97f4a7c15ULL);
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return z ^ (z >> 31);
}

int rndb() { return (int)(rnd() & 0xff); }

Stat &reg(const char *n)
{
	stats[nstats++] = {n, 0, 0};
	return stats[nstats - 1];
}

void reset_both()
{
	oracle_reset_batch();
	port::reset_batch();
}

bool scratch_both()
{
	if (ref_open_sbuf() != 0)
		return false;
	if (port::open_sbuf() != 0) {
		ref_close_sbuf();
		return false;
	}
	ref_get_addressed_line_node(0);
	port::get_addressed_line_node(0);
	return true;
}

struct GuardBuf {
	static constexpr std::size_t PRE = 32;
	static constexpr std::size_t USER = 1024;
	static constexpr std::size_t POST = 32;
	static constexpr std::size_t CAP = PRE + USER + POST;
	unsigned char bytes[CAP];

	void fill()
	{
		std::memset(bytes, GUARD, CAP);
	}

	char *user() { return (char *)(bytes + PRE); }

	bool eq(const GuardBuf &o) const
	{
		return std::memcmp(bytes, o.bytes, CAP) == 0;
	}
};

struct PutLine {
	static constexpr std::size_t PRE = 16;
	static constexpr std::size_t USER = 512;
	static constexpr std::size_t POST = 16;
	static constexpr std::size_t CAP = PRE + USER + POST;
	unsigned char bytes[CAP];

	void fill()
	{
		std::memset(bytes, GUARD, CAP);
	}

	char *user() { return (char *)(bytes + PRE); }

	bool eq(const PutLine &o) const
	{
		return std::memcmp(bytes, o.bytes, CAP) == 0;
	}
};

void test_translit_text()
{
	Stat &st = reg("translit_text");

	auto run = [&](const char *in, int len, int from, int to) {
		GuardBuf rg, pg;
		rg.fill();
		pg.fill();
		if (len > 0)
			std::memcpy(rg.user(), in, (std::size_t)len);
		if (len > 0)
			std::memcpy(pg.user(), in, (std::size_t)len);
		char *rr = ref_translit_text(rg.user(), len, from, to);
		char *pr = port::translit_text(pg.user(), len, from, to);
		long ro = rr ? (rr - rg.user()) : -1;
		long po = pr ? (pr - pg.user()) : -1;
		st.cases++;
		if (ro != po || !rg.eq(pg))
			st.fails++;
	};

	run("", 0, 0, 1);
	run("a", 1, 'a', 'A');
	run("\0\0\0", 3, 0, 1);
	run("\x80\xff\xfe", 3, (int)(signed char)0x80, 32);
	run("hello", 5, 'l', 'L');
	run("\xff", 1, 0xff, 0);
	run("\xff", 1, -1, 0);

	for (long i = 0; i < RANDOM_ITERS / 10; i++) {
		char b[256];
		int n = (int)(rnd() % 200);
		for (int j = 0; j < n; j++)
			b[j] = (char)rndb();
		int from = rndb();
		int to = rndb();
		run(b, n, from, to);
	}
}

void test_put_sbuf_line()
{
	Stat &st = reg("put_sbuf_line");

	auto run = [&](const char *payload, std::size_t plen, bool expect_ok) {
		PutLine rl, pl;
		rl.fill();
		pl.fill();
		std::memcpy(rl.user(), payload, plen);
		std::memcpy(pl.user(), payload, plen);
		const char *rr = ref_put_sbuf_line(rl.user());
		long ro = rr ? (rr - rl.user()) : -1;
		const char *pr = port::put_sbuf_line(pl.user());
		long po = pr ? (pr - pl.user()) : -1;
		st.cases++;
		if (ro != po || !rl.eq(pl) ||
		    ((rr == nullptr) != (pr == nullptr)) ||
		    std::strcmp(errmsg, port::errmsg) != 0)
			st.fails++;
		(void)expect_ok;
	};

	run("\n", 1, true);
	run("x\n", 2, true);
	run("hello\n", 6, true);
	run("\x80\xff\x00\n", 4, true);

	for (long i = 0; i < RANDOM_ITERS / 10; i++) {
		char b[400];
		int n = (int)(rnd() % 300);
		for (int j = 0; j < n; j++)
			b[j] = (char)rndb();
		b[n] = '\n';
		b[n + 1] = '\0';
		run(b, (std::size_t)n + 1, true);
	}
}

void test_get_sbuf_line()
{
	Stat &st = reg("get_sbuf_line");

	auto run = [&](int nl, int which, bool head) {
		char lines[32][128];
		reset_both();
		if (!scratch_both())
			return;

		for (int i = 0; i < nl; i++) {
			int n = (int)(rnd() % 80);
			for (int j = 0; j < n; j++)
				lines[i][j] = (char)rndb();
			lines[i][n] = '\n';
			lines[i][n + 1] = '\0';
			ref_put_sbuf_line(lines[i]);
		}

		void *rlp = head ? ref_get_addressed_line_node(0) :
		    ref_get_addressed_line_node(which > nl ? nl + 1 : which);
		char *rs = ref_get_sbuf_line(rlp);

		reset_both();
		if (!scratch_both())
			return;
		for (int i = 0; i < nl; i++)
			port::put_sbuf_line(lines[i]);
		void *plp = head ? port::get_addressed_line_node(0) :
		    port::get_addressed_line_node(which > nl ? nl + 1 : which);
		char *ps = port::get_sbuf_line((port::line_t *)plp);

		st.cases++;
		if (((rs == nullptr) != (ps == nullptr)) ||
		    (rs && ps && std::strcmp(rs, ps)) ||
		    std::strcmp(errmsg, port::errmsg) != 0)
			st.fails++;

		ref_close_sbuf();
		port::close_sbuf();
	};

	run(0, 0, true);
	run(1, 1, false);
	run(5, 3, false);
	run(8, 1, false);
	run(8, 8, false);

	for (long i = 0; i < RANDOM_ITERS / 10; i++) {
		int nl = (int)(rnd() % 12);
		int w = nl ? (int)(rnd() % (nl + 2)) : 0;
		run(nl, w, w == 0);
	}

	/* out-of-order reads force seek branches */
	for (long i = 0; i < RANDOM_ITERS / 20; i++) {
		reset_both();
		if (!scratch_both())
			continue;
		char line[4][64];
		for (int j = 0; j < 4; j++) {
			std::snprintf(line[j], sizeof(line[j]), "L%d\n", j);
			ref_put_sbuf_line(line[j]);
		}
		int order[4];
		for (int j = 0; j < 4; j++)
			order[j] = j;
		for (int j = 3; j > 0; j--) {
			int k = (int)(rnd() % (j + 1));
			int t = order[j];
			order[j] = order[k];
			order[k] = t;
		}
		bool ok = true;
		for (int j = 0; j < 4; j++) {
			char *rs = ref_get_sbuf_line(
			    ref_get_addressed_line_node(order[j] + 1));
			char expect[16];
			std::snprintf(expect, sizeof(expect), "L%d", order[j]);
			if (!rs || std::strcmp(rs, expect) != 0)
				ok = false;
		}
		reset_both();
		if (!scratch_both())
			continue;
		for (int j = 0; j < 4; j++)
			port::put_sbuf_line(line[j]);
		for (int j = 0; j < 4; j++) {
			char *ps = port::get_sbuf_line(
			    (port::line_t *)port::get_addressed_line_node(
				order[j] + 1));
			char expect[16];
			std::snprintf(expect, sizeof(expect), "L%d", order[j]);
			if (!ps || std::strcmp(ps, expect) != 0)
				ok = false;
		}
		st.cases++;
		if (!ok)
			st.fails++;
		ref_close_sbuf();
		port::close_sbuf();
	}
}

void test_get_addressed_line_node()
{
	Stat &st = reg("get_addressed_line_node");

	auto run = [&](long n, int nl) {
		reset_both();
		if (!scratch_both())
			return;
		for (int i = 0; i < nl; i++) {
			char line[16];
			std::snprintf(line, sizeof(line), "%d\n", i);
			ref_put_sbuf_line(line);
		}
		long ra = ref_get_line_node_addr(ref_get_addressed_line_node(n));

		reset_both();
		if (!scratch_both())
			return;
		for (int i = 0; i < nl; i++) {
			char line[16];
			std::snprintf(line, sizeof(line), "%d\n", i);
			port::put_sbuf_line(line);
		}
		long pa = port::get_line_node_addr(
		    (port::line_t *)port::get_addressed_line_node(n));

		st.cases++;
		if (ra != pa)
			st.fails++;
		ref_close_sbuf();
		port::close_sbuf();
	};

	for (long n = 0; n <= 10; n++)
		run(n, 8);
	for (long i = 0; i < RANDOM_ITERS / 10; i++) {
		int nl = (int)(rnd() % 16) + 1;
		long n = rnd() % (nl + 2);
		run(n, nl);
	}
}

void test_get_line_node_addr()
{
	Stat &st = reg("get_line_node_addr");

	auto run = [&](int nl, int target) {
		reset_both();
		if (!scratch_both())
			return;
		for (int i = 0; i < nl; i++) {
			char line[16];
			std::snprintf(line, sizeof(line), "t%d\n", i);
			ref_put_sbuf_line(line);
		}
		long r = ref_get_line_node_addr(
		    ref_get_addressed_line_node(target));

		reset_both();
		if (!scratch_both())
			return;
		for (int i = 0; i < nl; i++) {
			char line[16];
			std::snprintf(line, sizeof(line), "t%d\n", i);
			port::put_sbuf_line(line);
		}
		long p = port::get_line_node_addr(
		    (port::line_t *)port::get_addressed_line_node(target));

		st.cases++;
		if (r != p || std::strcmp(errmsg, port::errmsg) != 0)
			st.fails++;
		ref_close_sbuf();
		port::close_sbuf();
	};

	run(0, 0);
	run(5, 0);
	run(5, 3);
	run(5, 5);
	run(5, 7);
	run(5, 99);

	for (long i = 0; i < RANDOM_ITERS / 10; i++) {
		int nl = (int)(rnd() % 12);
		int t = (int)(rnd() % 20);
		run(nl, t);
	}
}

void test_add_line_node()
{
	Stat &st = reg("add_line_node");

	for (long i = 0; i < 50 + RANDOM_ITERS / 20; i++) {
		reset_both();
		if (!scratch_both())
			continue;
		ref_put_sbuf_line("a\n");
		long ar = addr_last;
		long cr = current_addr;

		reset_both();
		if (!scratch_both())
			continue;
		port::put_sbuf_line("a\n");
		long ap = port::addr_last;
		long cp = port::current_addr;

		st.cases++;
		if (ar != ap || cr != cp)
			st.fails++;
		ref_close_sbuf();
		port::close_sbuf();
	}
}

void test_open_close_sbuf()
{
	Stat &so = reg("open_sbuf");
	Stat &sc = reg("close_sbuf");

	for (long i = 0; i < 20 + RANDOM_ITERS / 20; i++) {
		reset_both();
		int r1 = ref_open_sbuf();
		int r2 = ref_close_sbuf();
		reset_both();
		int p1 = port::open_sbuf();
		int p2 = port::close_sbuf();
		so.cases++;
		sc.cases++;
		if (r1 != p1)
			so.fails++;
		if (r2 != p2)
			sc.fails++;
	}
}

void test_init_buffers()
{
	Stat &st = reg("init_buffers");

	reset_both();
	ref_init_buffers();
	int r = ref_close_sbuf();
	reset_both();
	port::init_buffers();
	int p = port::close_sbuf();
	st.cases = 1;
	if (r != p)
		st.fails = 1;
}

void test_quit()
{
	Stat &st = reg("quit");

	if (fork() == 0) {
		reset_both();
		ref_open_sbuf();
		oracle_quit_enter();
		ref_quit(3);
		_exit(oracle_quit_called && oracle_quit_status == 3 ? 0 : 1);
	}
	int refst = 0;
	waitpid(-1, &refst, 0);

	if (fork() == 0) {
		reset_both();
		port::open_sbuf();
		port::quit_enter();
		port::quit(3);
		_exit(port::quit_called && port::quit_status == 3 ? 0 : 1);
	}
	int portst = 0;
	waitpid(-1, &portst, 0);

	st.cases = 1;
	st.fails = (refst == 0 && portst == 0) ? 0 : 1;
}

} /* namespace */

int main()
{
	test_translit_text();
	test_put_sbuf_line();
	test_get_sbuf_line();
	test_get_addressed_line_node();
	test_get_line_node_addr();
	test_add_line_node();
	test_open_close_sbuf();
	test_init_buffers();
	test_quit();

	std::printf("PBSD batch b0148s4 differential test\n\n");
	std::printf("%-28s %12s %12s %10s\n", "function", "cases",
	    "failures", "result");
	long tc = 0, tf = 0;
	for (int i = 0; i < nstats; i++) {
		tc += stats[i].cases;
		tf += stats[i].fails;
		std::printf("%-28s %12ld %12ld %10s\n", stats[i].name,
		    stats[i].cases, stats[i].fails,
		    stats[i].fails ? "FAIL" : "PASS");
	}
	std::printf("%-28s %12ld %12ld %10s\n", "TOTAL", tc, tf,
	    tf ? "FAIL" : "PASS");
	return tf ? 1 : 0;
}
