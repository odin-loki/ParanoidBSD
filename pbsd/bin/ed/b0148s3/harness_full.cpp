/*
 * harness.cpp -- differential test for PBSD batch b0148s3 (glbl.c).
 */

import pbsd.bin.ed.b0148s3;

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace port = pbsd::bin_ed::b0148s3;

extern "C" {
extern char *ibuf;
extern char *ibufp;
extern int isbinary;
extern long current_addr;
extern long first_addr;
extern long second_addr;
extern const char *errmsg;
extern int oracle_extract_addr_range_result;
extern int oracle_exec_command_result;
extern int oracle_display_lines_result;
extern char *oracle_extended_line;
extern int oracle_extended_line_len;
extern int oracle_get_tty_line_result;
extern char oracle_tty_line[4096];
extern int oracle_malloc_fail_at;
void oracle_reset_batch(void);
int ref_open_sbuf(void);
int ref_close_sbuf(void);
const char *ref_put_sbuf_line(const char *);
void *ref_get_addressed_line_node(long);
long ref_get_line_node_addr(void *);
int ref_set_active_node(void *);
void *ref_next_active_node(void);
void ref_clear_active_list(void);
void ref_unset_active_nodes(void *, void *);
int ref_build_active_list(int);
long ref_exec_global(int, int);
long oracle_active_last(void);
long oracle_active_ptr(void);
long oracle_active_ndx(void);
long oracle_active_size(void);
void *oracle_active_at(long);
}

namespace {

constexpr long RANDOM_ITERS = 210000;

struct Stat { const char *name; long cases, fails; };
Stat stats[8];
int nstats;
std::uint64_t rng = 0x243f6a8885a308d3ULL;

std::uint64_t rnd()
{
	std::uint64_t z = (rng += 0x9e3779b97f4a7c15ULL);
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return z ^ (z >> 31);
}

Stat &reg(const char *n)
{
	stats[nstats++] = {n, 0, 0};
	return stats[nstats - 1];
}

void line_gen(int i, char *b, int sz) { std::snprintf(b, sz, "L%d\n", i); }

void reset_both()
{
	oracle_reset_batch();
	port::reset_batch();
}

void sync_port_globals()
{
	port::isbinary = isbinary;
	port::current_addr = current_addr;
	port::first_addr = first_addr;
	port::second_addr = second_addr;
	std::memcpy(port::ibuf, ibuf, 65536);
	port::ibufp = port::ibuf + (ibufp - ibuf);
}

void setup_ibuf(const char *s)
{
	std::strcpy(ibuf, s);
	ibufp = ibuf;
	sync_port_globals();
}

void sync_hooks_to_port()
{
	port::extract_addr_range_result = oracle_extract_addr_range_result;
	port::exec_command_result = oracle_exec_command_result;
	port::display_lines_result = oracle_display_lines_result;
	port::extended_line = oracle_extended_line;
	port::extended_line_len = oracle_extended_line_len;
	port::get_tty_line_result = oracle_get_tty_line_result;
	std::strcpy(port::tty_line, oracle_tty_line);
	port::malloc_fail_at = oracle_malloc_fail_at;
}

bool scratch_ref()
{
	reset_both();
	if (ref_open_sbuf())
		return false;
	ref_get_addressed_line_node(0);
	return true;
}

bool scratch_port()
{
	reset_both();
	if (port::open_sbuf())
		return false;
	port::get_addressed_line_node(0);
	return true;
}

void add_lines_ref(int nl)
{
	char line[256];
	for (int i = 0; i < nl; i++) {
		line_gen(i, line, (int)sizeof(line));
		ref_put_sbuf_line(line);
	}
}

void add_lines_port(int nl)
{
	char line[256];
	for (int i = 0; i < nl; i++) {
		line_gen(i, line, (int)sizeof(line));
		port::put_sbuf_line(line);
	}
}

void put_pattern_lines_ref(int nl)
{
	char line[128];
	for (int i = 0; i < nl; i++) {
		if (i % 3 == 0)
			std::snprintf(line, sizeof(line), "match%d\n", i);
		else if (i % 3 == 1)
			std::snprintf(line, sizeof(line), "x%d\n", i);
		else
			std::snprintf(line, sizeof(line), "\x80match%d\n", i);
		ref_put_sbuf_line(line);
	}
}

void put_pattern_lines_port(int nl)
{
	char line[128];
	for (int i = 0; i < nl; i++) {
		if (i % 3 == 0)
			std::snprintf(line, sizeof(line), "match%d\n", i);
		else if (i % 3 == 1)
			std::snprintf(line, sizeof(line), "x%d\n", i);
		else
			std::snprintf(line, sizeof(line), "\x80match%d\n", i);
		port::put_sbuf_line(line);
	}
}

bool active_state_match()
{
	long rl = oracle_active_last();
	long pl = port::port_active_last();
	if (rl != pl)
		return false;
	if (oracle_active_ptr() != port::port_active_ptr())
		return false;
	if (oracle_active_ndx() != port::port_active_ndx())
		return false;
	if (oracle_active_size() != port::port_active_size())
		return false;
	for (long i = 0; i < rl; i++) {
		void *ra = oracle_active_at(i);
		void *pa = port::port_active_at(i);
		if ((ra == nullptr) != (pa == nullptr))
			return false;
		if (ra) {
			long ro = ref_get_line_node_addr(ra);
			long po = port::get_line_node_addr(
			    (port::line_t *)pa);
			if (ro != po)
				return false;
		}
	}
	return true;
}

void test_set_active_node()
{
	Stat &st = reg("set_active_node");

	auto run = [&](int count, int fail_at) {
		int rf = 0, pf = 0;

		if (!scratch_ref())
			return;
		add_lines_ref(count > 0 ? count : 1);
		oracle_malloc_fail_at = fail_at;
		for (int j = 0; j < count; j++) {
			int r = ref_set_active_node(
			    ref_get_addressed_line_node(j + 1));
			if (r < 0)
				rf = r;
		}
		long ral = oracle_active_last();
		long rap = oracle_active_ptr();
		long ras = oracle_active_size();
		ref_close_sbuf();
		oracle_malloc_fail_at = 0;

		if (!scratch_port())
			return;
		add_lines_port(count > 0 ? count : 1);
		port::malloc_fail_at = fail_at;
		for (int j = 0; j < count; j++) {
			int p = port::set_active_node(
			    port::get_addressed_line_node(j + 1));
			if (p < 0)
				pf = p;
		}
		long pal = port::port_active_last();
		long pap = port::port_active_ptr();
		long pas = port::port_active_size();
		port::close_sbuf();
		port::malloc_fail_at = 0;

		st.cases++;
		if (rf != pf || ral != pal || rap != pap || ras != pas)
			st.fails++;
	};

	run(0, 0);
	run(1, 0);
	run(3, 0);
	run(520, 0);
	run(600, 0);
	for (long i = 0; i < RANDOM_ITERS / 6; i++)
		run((int)(rnd() % 40) + 1, 0);
	for (long i = 0; i < 50; i++)
		run((int)(rnd() % 8) + 1, (int)(rnd() % 20) + 1);
}

void test_next_active_node()
{
	Stat &st = reg("next_active_node");

	auto run = [&](int nl, int nullify) {
		if (!scratch_ref())
			return;
		add_lines_ref(nl);
		for (int j = 0; j < nl; j++)
			ref_set_active_node(ref_get_addressed_line_node(j + 1));
		if (nullify > 0 && nullify <= nl)
			ref_unset_active_nodes(
			    ref_get_addressed_line_node(nullify),
			    ref_get_addressed_line_node(nullify + 1));

		long ref_offs[32];
		int ref_n = 0;
		for (int step = 0; step < nl + 3; step++) {
			void *r = ref_next_active_node();
			ref_offs[ref_n++] = r ? ref_get_line_node_addr(r) : -1;
		}
		long ral = oracle_active_last();
		long rap = oracle_active_ptr();
		ref_close_sbuf();

		if (!scratch_port())
			return;
		add_lines_port(nl);
		for (int j = 0; j < nl; j++)
			port::set_active_node(
			    port::get_addressed_line_node(j + 1));
		if (nullify > 0 && nullify <= nl)
			port::unset_active_nodes(
			    port::get_addressed_line_node(nullify),
			    port::get_addressed_line_node(nullify + 1));

		for (int step = 0; step < ref_n; step++) {
			void *p = port::next_active_node();
			long po = p ? port::get_line_node_addr(
			    (port::line_t *)p) : -1;
			st.cases++;
			if (ref_offs[step] != po)
				st.fails++;
		}
		st.cases++;
		if (ral != port::port_active_last() ||
		    rap != port::port_active_ptr())
			st.fails++;
		port::close_sbuf();
	};

	run(0, 0);
	run(1, 0);
	run(4, 0);
	run(4, 2);
	run(8, 5);
	for (long i = 0; i < RANDOM_ITERS / 6; i++) {
		int nl = (int)(rnd() % 12) + 1;
		int nz = (rnd() & 3) ? (int)(rnd() % nl) + 1 : 0;
		run(nl, nz);
	}
}

void test_unset_active_nodes()
{
	Stat &st = reg("unset_active_nodes");

	auto run = [&](int nl, int from, int to) {
		if (from < 1 || to <= from || to > nl + 1)
			return;

		if (!scratch_ref())
			return;
		add_lines_ref(nl);
		for (int j = 0; j < nl; j++)
			ref_set_active_node(ref_get_addressed_line_node(j + 1));
		ref_unset_active_nodes(ref_get_addressed_line_node(from),
		    ref_get_addressed_line_node(to));
		long ral = oracle_active_last();
		long rndx = oracle_active_ndx();
		for (long i = 0; i < ral; i++) {
			void *ra = oracle_active_at(i);
			(void)ra;
		}
		ref_close_sbuf();

		if (!scratch_port())
			return;
		add_lines_port(nl);
		for (int j = 0; j < nl; j++)
			port::set_active_node(
			    port::get_addressed_line_node(j + 1));
		port::unset_active_nodes(
		    port::get_addressed_line_node(from),
		    port::get_addressed_line_node(to));

		st.cases++;
		if (ral != port::port_active_last() ||
		    rndx != port::port_active_ndx())
			st.fails++;
		for (long i = 0; i < ral; i++) {
			void *ra = oracle_active_at(i);
			void *pa = port::port_active_at(i);
			if ((ra == nullptr) != (pa == nullptr)) {
				st.fails++;
				break;
			}
			if (ra) {
				long ro = ref_get_line_node_addr(ra);
				long po = port::get_line_node_addr(
				    (port::line_t *)pa);
				if (ro != po) {
					st.fails++;
					break;
				}
			}
		}
		port::close_sbuf();
	};

	run(3, 1, 2);
	run(5, 2, 4);
	run(6, 1, 7);
	for (long i = 0; i < RANDOM_ITERS / 6; i++) {
		int nl = (int)(rnd() % 10) + 2;
		int from = (int)(rnd() % nl) + 1;
		int to = from + (int)(rnd() % (nl - from + 1)) + 1;
		if (to > nl + 1)
			to = nl + 1;
		run(nl, from, to);
	}
}

void test_clear_active_list()
{
	Stat &st = reg("clear_active_list");

	auto run = [&](int nl) {
		if (!scratch_ref())
			return;
		add_lines_ref(nl);
		for (int j = 0; j < nl; j++)
			ref_set_active_node(ref_get_addressed_line_node(j + 1));
		ref_clear_active_list();
		long ral = oracle_active_last();
		long rap = oracle_active_ptr();
		ref_close_sbuf();

		if (!scratch_port())
			return;
		add_lines_port(nl);
		for (int j = 0; j < nl; j++)
			port::set_active_node(
			    port::get_addressed_line_node(j + 1));
		port::clear_active_list();

		st.cases++;
		if (ral != port::port_active_last() ||
		    rap != port::port_active_ptr() ||
		    ral != 0)
			st.fails++;
		port::close_sbuf();
	};

	run(0);
	run(1);
	run(10);
	for (long i = 0; i < RANDOM_ITERS / 6; i++)
		run((int)(rnd() % 20));
}

void test_build_active_list()
{
	Stat &st = reg("build_active_list");

	auto run = [&](const char *patcmd, int isg, int bin, int fa, int sa,
	    int nl) {
		if (!scratch_ref())
			return;
		put_pattern_lines_ref(nl);
		first_addr = fa;
		second_addr = sa;
		isbinary = bin;
		setup_ibuf(patcmd);
		int r = ref_build_active_list(isg);
		const char *re = errmsg;
		long ral = oracle_active_last();
		ref_close_sbuf();

		if (!scratch_port())
			return;
		put_pattern_lines_port(nl);
		port::first_addr = fa;
		port::second_addr = sa;
		isbinary = bin;
		setup_ibuf(patcmd);
		sync_hooks_to_port();
		int p = port::build_active_list(isg);
		const char *pe = port::errmsg;
		long pal = port::port_active_last();

		st.cases++;
		if (r != p || std::strcmp(re, pe) != 0 || ral != pal)
			st.fails++;
		if (r == 0 && p == 0 && !active_state_match())
			st.fails++;
		port::close_sbuf();
	};

	run(" \n", 0, 0, 1, 1, 1);
	run("\n", 0, 0, 1, 1, 1);
	run("/[/\n", 0, 0, 1, 1, 1);
	run("/match/\n", 0, 0, 1, 3, 3);
	run("/match/\n", 1, 0, 1, 3, 3);
	run("/match/\n", 0, 1, 1, 3, 3);
	run("/match/\n", 1, 1, 1, 3, 3);
	run("/^x/\n", 0, 0, 1, 5, 5);
	run("/match/\n", 0, 0, 2, 4, 5);
	run("/\xff/\n", 0, 0, 1, 2, 2);

	for (long i = 0; i < RANDOM_ITERS / 6; i++) {
		int nl = (int)(rnd() % 8);
		int isg = (int)(rnd() & 1);
		int bin = (int)(rnd() & 1);
		int fa = 1;
		int sa = nl ? (int)(rnd() % nl) + 1 : 1;
		const char *patterns[] = {"/match/\n", "/^x/\n", "/./\n",
		    "/^/\n", "/\x80/\n"};
		run(patterns[rnd() % 5], isg, bin, fa, sa, nl);
	}
}

void test_exec_global()
{
	Stat &st = reg("exec_global");

	auto run = [&](int interact, int gflag, const char *cmd,
	    int nl, int ear, int ecr, int dlr, int tty_mode) {
		static char cmdbuf[256];
		std::strncpy(cmdbuf, cmd, sizeof(cmdbuf) - 1);
		cmdbuf[sizeof(cmdbuf) - 1] = '\0';

		if (!scratch_ref())
			return;
		add_lines_ref(nl > 0 ? nl : 1);
		for (int j = 0; j < (nl > 0 ? nl : 1); j++)
			ref_set_active_node(ref_get_addressed_line_node(j + 1));
		oracle_extended_line = interact ? nullptr : cmdbuf;
		oracle_extended_line_len = interact ? 0 :
		    (int)std::strlen(cmdbuf);
		oracle_extract_addr_range_result = ear;
		oracle_exec_command_result = ecr;
		oracle_display_lines_result = dlr;
		oracle_get_tty_line_result = tty_mode;
		if (interact) {
			if (tty_mode > 0)
				std::strcpy(oracle_tty_line, "d\n");
			else if (tty_mode == 0)
				oracle_tty_line[0] = '\0';
		} else if (tty_mode > 0)
			std::strcpy(oracle_tty_line, "\n");
		else if (tty_mode == 0)
			oracle_tty_line[0] = '\0';
		long r = ref_exec_global(interact, gflag);
		const char *re = errmsg;
		ref_close_sbuf();

		if (!scratch_port())
			return;
		add_lines_port(nl > 0 ? nl : 1);
		for (int j = 0; j < (nl > 0 ? nl : 1); j++)
			port::set_active_node(
			    port::get_addressed_line_node(j + 1));
		port::extended_line = interact ? nullptr : cmdbuf;
		port::extended_line_len = interact ? 0 :
		    (int)std::strlen(cmdbuf);
		port::extract_addr_range_result = ear;
		port::exec_command_result = ecr;
		port::display_lines_result = dlr;
		port::get_tty_line_result = tty_mode;
		if (interact) {
			if (tty_mode > 0)
				std::strcpy(port::tty_line, "d\n");
			else if (tty_mode == 0)
				port::tty_line[0] = '\0';
		} else if (tty_mode > 0)
			std::strcpy(port::tty_line, "\n");
		else if (tty_mode == 0)
			port::tty_line[0] = '\0';
		long p = port::exec_global(interact, gflag);
		const char *pe = port::errmsg;

		st.cases++;
		if (r != p || std::strcmp(re, pe) != 0)
			st.fails++;
		port::close_sbuf();
	};

	run(0, 0, "p\n", 1, 0, 0, 0, 1);
	run(0, 0, "", 1, 0, 0, 0, 1);
	run(0, 0, "p\n", 2, 0, 0, 0, 1);
	run(0, 0, "p\n", 1, -1, 0, 0, 1);
	run(0, 0, "p\n", 1, 0, -1, 0, 1);
	run(0, 0, "p\n", 1, 0, 1, 1, 1);
	run(0, 0, "p\n", 1, 0, 2, -1, 1);
	run(1, 0, "p\n", 1, 0, 0, 0, 1);
	run(1, 0, "p\n", 1, 0, 0, 0, -1);
	run(1, 0, "p\n", 1, 0, 0, 0, 0);

	{
		static char xcmd[] = "d\n";
		if (scratch_ref()) {
			add_lines_ref(1);
			ref_set_active_node(ref_get_addressed_line_node(1));
			oracle_extended_line = xcmd;
			oracle_extended_line_len = 2;
			oracle_extract_addr_range_result = 0;
			oracle_exec_command_result = 0;
			oracle_display_lines_result = 0;
			oracle_get_tty_line_result = 1;
			std::strcpy(oracle_tty_line, "&\n");
			long r = ref_exec_global(1, 0);
			const char *re = errmsg;
			ref_close_sbuf();

			if (scratch_port()) {
				add_lines_port(1);
				port::set_active_node(
				    port::get_addressed_line_node(1));
				port::extended_line = xcmd;
				port::extended_line_len = 2;
				port::extract_addr_range_result = 0;
				port::exec_command_result = 0;
				port::display_lines_result = 0;
				port::get_tty_line_result = 1;
				std::strcpy(port::tty_line, "&\n");
				long p = port::exec_global(1, 0);
				const char *pe = port::errmsg;
				st.cases++;
				if (r != p || std::strcmp(re, pe) != 0)
					st.fails++;
				port::close_sbuf();
			}
		}
	}

	for (long i = 0; i < RANDOM_ITERS / 6; i++) {
		int interact = (int)(rnd() & 1);
		int gflag = (int)(rnd() & 7);
		int nl = (int)(rnd() % 5) + 1;
		int ear = (rnd() & 7) == 0 ? -1 : 0;
		int ecr = (rnd() & 7) == 0 ? -1 : (int)(rnd() & 3);
		int dlr = (rnd() & 7) == 0 ? -1 : (int)(rnd() & 3);
		int tty = interact ? (int)(rnd() % 3) - 1 : 1;
		const char *cmds[] = {"p\n", "1p\n", "d\n", "w\n", ""};
		run(interact, gflag, cmds[rnd() % 5], nl, ear, ecr, dlr, tty);
	}
}

} /* namespace */

int main()
{
	test_set_active_node();
	test_next_active_node();
	test_unset_active_nodes();
	test_clear_active_list();
	test_build_active_list();
	test_exec_global();

	std::printf("PBSD batch b0148s3 differential test\n\n");
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
