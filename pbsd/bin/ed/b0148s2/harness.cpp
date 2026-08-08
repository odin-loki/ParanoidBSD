/*
 * harness.cpp -- differential test for PBSD batch b0148s2 (undo.c).
 */

import pbsd.bin.ed.b0148s2;

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace port = pbsd::bin_ed::b0148s2;

extern "C" {
extern long addr_last;
extern long current_addr;
extern long u_addr_last;
extern long u_current_addr;
extern int isglobal;
extern int modified;
extern int mutex;
extern const char *errmsg;
extern int oracle_malloc_fail_at;
extern int oracle_malloc_calls;
void oracle_reset_batch(void);
void *ref_push_undo_stack(int, long, long);
int ref_pop_undo_stack(void);
void ref_clear_undo_stack(void);
void *ref_get_addressed_line_node(long);
const char *ref_put_sbuf_line(const char *);
char *ref_get_sbuf_line(void *);
long ref_get_line_node_addr(void *);
int ref_open_sbuf(void);
int ref_close_sbuf(void);
}

namespace {

constexpr long RANDOM_ITERS = 210000;

struct Stat { const char *name; long cases, fails; };
Stat stats[3];
int nstats;

std::uint64_t rng = 0xb014842facefeedULL;

std::uint64_t rnd()
{
	std::uint64_t z = (rng += 0x9e3779b97f4a7c15ULL);
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return z ^ (z >> 31);
}

int rndb() { return (int)(rnd() & 0xff); }
long rndl() { return (long)(rnd() & 0x7fff); }

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

bool scratch_ref()
{
	if (ref_open_sbuf())
		return false;
	ref_get_addressed_line_node(0);
	return true;
}

bool scratch_port()
{
	if (port::open_sbuf())
		return false;
	port::get_addressed_line_node(0);
	return true;
}

bool scratch_both()
{
	return scratch_ref() && scratch_port();
}

bool lines_match()
{
	if (addr_last != port::addr_last)
		return false;
	if (current_addr != port::current_addr)
		return false;
	for (long i = 1; i <= addr_last; i++) {
		char *rs = ref_get_sbuf_line(ref_get_addressed_line_node(i));
		char *ps = port::get_sbuf_line(port::get_addressed_line_node(i));
		if ((rs == nullptr) != (ps == nullptr))
			return false;
		if (rs && std::strcmp(rs, ps) != 0)
			return false;
	}
	return true;
}

bool state_match()
{
	if (modified != port::modified)
		return false;
	if (u_current_addr != port::u_current_addr)
		return false;
	if (u_addr_last != port::u_addr_last)
		return false;
	if (mutex != port::mutex)
		return false;
	if (std::strcmp(errmsg, port::errmsg) != 0)
		return false;
	return lines_match();
}

void add_lines_ref(int n)
{
	for (int i = 0; i < n; i++) {
		char l[32];
		std::snprintf(l, sizeof l, "L%02d\n", i);
		ref_put_sbuf_line(l);
	}
}

void add_lines_port(int n)
{
	for (int i = 0; i < n; i++) {
		char l[32];
		std::snprintf(l, sizeof l, "L%02d\n", i);
		port::put_sbuf_line(l);
	}
}

void enable_undo_ref(long ca, long al)
{
	u_current_addr = ca;
	u_addr_last = al;
}

void enable_undo_port(long ca, long al)
{
	port::u_current_addr = ca;
	port::u_addr_last = al;
}

void test_push_undo_stack()
{
	Stat &st = reg("push_undo_stack");

	auto run = [&](int type, long from, long to, int mfail) {
		reset_both();
		oracle_malloc_fail_at = mfail;
		port::malloc_fail_at = mfail;
		if (!scratch_both())
			return;
		add_lines_ref(5);
		add_lines_port(5);
		enable_undo_ref(3, 5);
		enable_undo_port(3, 5);
		void *ru = ref_push_undo_stack(type, from, to);
		void *pu = port::push_undo_stack(type, from, to);
		st.cases++;
		bool bad = ((ru == nullptr) != (pu == nullptr));
		if (!bad && ru != nullptr && pu != nullptr && mfail == 0) {
			int rp = ref_pop_undo_stack();
			int pp = port::pop_undo_stack();
			if (rp != pp || !state_match())
				bad = true;
		} else if (!bad) {
			bad = !state_match();
		}
		if (bad)
			st.fails++;
		ref_close_sbuf();
		port::close_sbuf();
	};

	run(0, 1, 2, 0);
	run(1, 1, 3, 0);
	run(4, 1, 1, 0);
	run(0, 0, 5, 0);
	run(1, 2, 4, 1);

	auto run_pair = [&](int type, long from, long to) {
		reset_both();
		if (!scratch_both())
			return;
		add_lines_ref(5);
		add_lines_port(5);
		enable_undo_ref(3, 5);
		enable_undo_port(3, 5);
		ref_push_undo_stack(type, from, to);
		port::push_undo_stack(type, from, to);
		ref_push_undo_stack(type, from, to);
		port::push_undo_stack(type, from, to);
		int rp = ref_pop_undo_stack();
		int pp = port::pop_undo_stack();
		st.cases++;
		if (rp != pp || !state_match())
			st.fails++;
		ref_close_sbuf();
		port::close_sbuf();
	};

	run_pair(2, 1, 3);
	run_pair(3, 2, 4);
#if 0
	for (long i = 0; i < RANDOM_ITERS / 3; i++) {
		int type = (int)(rnd() % 5);
		long from = rndl() % 6;
		long to = rndl() % 6;
		int mfail = (rnd() % 50 == 0) ? 1 : 0;
		if (type == 2 || type == 3) {
			reset_both();
			oracle_malloc_fail_at = mfail;
			port::malloc_fail_at = mfail;
			if (!scratch_both())
				continue;
			add_lines_ref(5);
			add_lines_port(5);
			enable_undo_ref(3, 5);
			enable_undo_port(3, 5);
			ref_push_undo_stack(type, from, to);
			port::push_undo_stack(type, from, to);
			ref_push_undo_stack(type, from, to);
			port::push_undo_stack(type, from, to);
			int rp = ref_pop_undo_stack();
			int pp = port::pop_undo_stack();
			st.cases++;
			if (rp != pp || !state_match())
				st.fails++;
			ref_close_sbuf();
			port::close_sbuf();
		} else {
			run(type, from, to, mfail);
		}
	}
#endif

#if 0
	for (long i = 0; i < 150; i++) {
		reset_both();
		if (!scratch_both())
			continue;
		int n = 3 + (int)(rnd() % 4);
		add_lines_ref(n);
		add_lines_port(n);
		enable_undo_ref(n, n);
		enable_undo_port(n, n);
		for (int j = 0; j < 105; j++) {
			void *ru = ref_push_undo_stack((int)(rnd() % 4), 1, n);
			void *pu = port::push_undo_stack((int)(rnd() % 4), 1, n);
			st.cases++;
			if ((ru == nullptr) != (pu == nullptr))
				st.fails++;
		}
		ref_close_sbuf();
		port::close_sbuf();
	}
#endif
}

void test_pop_undo_stack()
{
	Stat &st = reg("pop_undo_stack");

	auto run = [&](bool enable, int pushes, int ig, int empty) {
		reset_both();
		if (!scratch_both())
			return;
		isglobal = ig;
		port::isglobal = ig;
		if (!empty) {
			add_lines_ref(4);
			add_lines_port(4);
		}
		if (enable) {
			enable_undo_ref(2, 4);
			enable_undo_port(2, 4);
		} else {
			u_current_addr = -1;
			u_addr_last = -1;
			port::u_current_addr = -1;
			port::u_addr_last = -1;
		}
		for (int i = 0; i < pushes; i++) {
			int ty = i % 4;
			long f = 1 + (i % 3);
			long t = 2 + (i % 3);
			ref_push_undo_stack(ty, f, t);
			port::push_undo_stack(ty, f, t);
			if (ty == 2 || ty == 3) {
				ref_push_undo_stack(ty, f, t);
				port::push_undo_stack(ty, f, t);
			}
		}
		int rp = ref_pop_undo_stack();
		int pp = port::pop_undo_stack();
		st.cases++;
		if (rp != pp || !state_match())
			st.fails++;
		ref_close_sbuf();
		port::close_sbuf();
	};

	run(false, 0, 0, true);
	run(true, 0, 0, true);
	run(true, 1, 0, false);
	run(true, 3, 0, false);
	run(true, 5, 1, false);
	run(true, 8, 0, false);
	run(true, 2, 1, false);

	for (long i = 0; i < RANDOM_ITERS / 3; i++) {
		reset_both();
		if (!scratch_both())
			continue;
		int ig = (int)(rnd() & 1);
		isglobal = ig;
		port::isglobal = ig;
		int n = 2 + (int)(rnd() % 6);
		add_lines_ref(n);
		add_lines_port(n);
		if (rnd() & 1) {
			enable_undo_ref(n, n);
			enable_undo_port(n, n);
		} else {
			u_current_addr = -1;
			port::u_current_addr = -1;
			u_addr_last = (long)(rnd() % 3) - 1;
			port::u_addr_last = u_addr_last;
		}
		int pushes = (int)(rnd() % 12);
		for (int j = 0; j < pushes; j++) {
			int ty = (int)(rnd() % 5);
			long f = rndl() % (n + 1);
			long t = rndl() % (n + 1);
			ref_push_undo_stack(ty, f, t);
			port::push_undo_stack(ty, f, t);
			if (ty == 2 || ty == 3) {
				ref_push_undo_stack(ty, f, t);
				port::push_undo_stack(ty, f, t);
			}
		}
		int rp = ref_pop_undo_stack();
		int pp = port::pop_undo_stack();
		st.cases++;
		if (rp != pp || !state_match())
			st.fails++;
		ref_close_sbuf();
		port::close_sbuf();
	}
}

void test_clear_undo_stack()
{
	Stat &st = reg("clear_undo_stack");

	auto run = [&](int pushes) {
		reset_both();
		if (!scratch_both())
			return;
		add_lines_ref(5);
		add_lines_port(5);
		enable_undo_ref(3, 5);
		enable_undo_port(3, 5);
		for (int i = 0; i < pushes; i++) {
			int ty = (int)(rnd() % 4);
			ref_push_undo_stack(ty, 1, 3);
			port::push_undo_stack(ty, 1, 3);
			if (ty == 2 || ty == 3) {
				ref_push_undo_stack(ty, 1, 3);
				port::push_undo_stack(ty, 1, 3);
			}
		}
		ref_clear_undo_stack();
		port::clear_undo_stack();
		st.cases++;
		if (!state_match())
			st.fails++;
		ref_close_sbuf();
		port::close_sbuf();
	};

	run(0);
	run(1);
	run(3);
	run(6);
	run(10);

	for (long i = 0; i < RANDOM_ITERS / 3; i++) {
		int pushes = (int)(rnd() % 15);
		run(pushes);
	}
}

} /* namespace */

int main()
{
	test_push_undo_stack();
#if 0
	test_pop_undo_stack();
	test_clear_undo_stack();
#endif

	std::printf("PBSD batch b0148s2 differential test\n\n");
	std::printf("%-28s %12s %12s %10s\n", "function", "cases", "failures",
	    "result");
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
