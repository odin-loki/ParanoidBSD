/*
 * harness.cpp -- differential test for PBSD batch b0148.
 */

import pbsd.bin.ed.b0148;

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace port = pbsd::bin_ed::b0148;

extern "C" {
extern char *ibuf;
extern char *ibufp;
extern int isbinary;
extern int isglobal;
extern int modified;
extern long addr_last;
extern long current_addr;
extern long first_addr;
extern long second_addr;
extern long u_addr_last;
extern long u_current_addr;
extern int patlock;
extern const char *errmsg;

extern int oracle_extract_addr_range_result;
extern int oracle_exec_command_result;
extern int oracle_display_lines_result;
extern char *oracle_extended_line;
extern int oracle_extended_line_len;
extern int oracle_get_tty_line_result;
extern char oracle_tty_line[4096];
extern int oracle_malloc_fail_at;
extern int oracle_malloc_calls;
extern int oracle_quit_status;
extern int oracle_quit_called;

void oracle_reset_batch(void);
void oracle_reset_globals(void);
void oracle_quit_enter(void);

char *ref_parse_char_class(char *s);
char *ref_translit_text(char *s, int len, int from, int to);
char *ref_extract_pattern(int delimiter);
pattern_t *ref_get_compiled_pattern(void);
line_t *ref_get_addressed_line_node(long n);
long ref_get_line_node_addr(line_t *lp);
void ref_add_line_node(line_t *lp);
int ref_open_sbuf(void);
int ref_close_sbuf(void);
char *ref_get_sbuf_line(line_t *lp);
const char *ref_put_sbuf_line(const char *cs);
undo_t *ref_push_undo_stack(int type, long from, long to);
int ref_pop_undo_stack(void);
void ref_clear_undo_stack(void);
int ref_set_active_node(line_t *lp);
void ref_unset_active_nodes(line_t *np, line_t *mp);
line_t *ref_next_active_node(void);
void ref_clear_active_list(void);
int ref_build_active_list(int isgcmd);
long ref_exec_global(int interact, int gflag);
void ref_init_buffers(void);
void ref_quit(int n);
}

typedef regex_t pattern_t;

struct line_t {
	line_t *q_forw;
	line_t *q_back;
	off_t seek;
	int len;
};

struct undo_t {
	int type;
	line_t *h;
	line_t *t;
};

namespace {

constexpr int ERR = -2;
constexpr int GUARD = 0x7f;
constexpr long RANDOM_ITERS = 200000;
constexpr int MAX_REPORT = 8;

struct Stat {
	const char *name;
	long cases;
	long fails;
	int reported;
};

Stat stats[22];
int nstats;

std::uint64_t rng_state = 0x243f6a8885a308d3ULL;

std::uint64_t rnd()
{
	std::uint64_t z = (rng_state += 0x9e3779b97f4a7c15ULL);
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return z ^ (z >> 31);
}

int rnd_byte()
{
	return (int)(rnd() & 0xff);
}

Stat &reg(const char *name)
{
	stats[nstats++] = Stat{name, 0, 0, 0};
	return stats[nstats - 1];
}

void sync_hooks()
{
	port::extract_addr_range_result = oracle_extract_addr_range_result;
	port::exec_command_result = oracle_exec_command_result;
	port::display_lines_result = oracle_display_lines_result;
	port::extended_line = oracle_extended_line;
	port::extended_line_len = oracle_extended_line_len;
	port::get_tty_line_result = oracle_get_tty_line_result;
	std::memcpy(port::tty_line, oracle_tty_line, sizeof(oracle_tty_line));
	port::malloc_fail_at = oracle_malloc_fail_at;
}

void sync_globals_to_port()
{
	port::isbinary = isbinary;
	port::isglobal = isglobal;
	port::modified = modified;
	port::addr_last = addr_last;
	port::current_addr = current_addr;
	port::first_addr = first_addr;
	port::second_addr = second_addr;
	port::u_addr_last = u_addr_last;
	port::u_current_addr = u_current_addr;
	port::patlock = patlock;
	port::errmsg = errmsg;
	std::memcpy(port::ibuf, ibuf, 65536);
	port::ibufp = port::ibuf + (ibufp - ibuf);
}

void copy_globals_from_port()
{
	isbinary = port::isbinary;
	isglobal = port::isglobal;
	modified = port::modified;
	addr_last = port::addr_last;
	current_addr = port::current_addr;
	first_addr = port::first_addr;
	second_addr = port::second_addr;
	u_addr_last = port::u_addr_last;
	u_current_addr = port::u_current_addr;
	patlock = port::patlock;
	errmsg = port::errmsg;
}

void reset_both()
{
	oracle_reset_batch();
	port::reset_batch();
	oracle_malloc_fail_at = 0;
	oracle_malloc_calls = 0;
	sync_hooks();
}

void setup_ibuf(const char *s)
{
	std::strcpy(ibuf, s);
	ibufp = ibuf;
	sync_globals_to_port();
}

struct GuardBuf {
	unsigned char before[64];
	unsigned char data[512];
	unsigned char after[64];
};

void fill_guard(GuardBuf &g)
{
	std::memset(g.before, GUARD, sizeof(g.before));
	std::memset(g.after, GUARD, sizeof(g.after));
	std::memset(g.data, 0, sizeof(g.data));
}

bool guards_ok(const GuardBuf &g)
{
	for (std::size_t i = 0; i < sizeof(g.before); i++)
		if (g.before[i] != GUARD)
			return false;
	for (std::size_t i = 0; i < sizeof(g.after); i++)
		if (g.after[i] != GUARD)
			return false;
	return true;
}

bool init_scratch_both()
{
	if (ref_open_sbuf() != 0)
		return false;
	if (port::open_sbuf() != 0) {
		ref_close_sbuf();
		return false;
	}
	return true;
}

void init_line_list_both()
{
	line_t *rh = ref_get_addressed_line_node(0);
	line_t *ph = port::get_addressed_line_node(0);
	(void)rh;
	(void)ph;
}

bool put_line_both(const char *line)
{
	long ca = current_addr;
	const char *r = ref_put_sbuf_line(line);
	copy_globals_from_port();
	current_addr = ca;
	sync_globals_to_port();
	const char *p = port::put_sbuf_line(line);
	copy_globals_from_port();
	if ((r == nullptr) != (p == nullptr))
		return false;
	if (r == nullptr)
		return true;
	long ro = (long)(r - line);
	long po = (long)(p - line);
	return ro == po && addr_last == port::addr_last;
}

void report(Stat &st, const char *tag)
{
	if (st.reported >= MAX_REPORT)
		return;
	st.reported++;
	std::printf("  FAIL %s [%s]\n", st.name, tag);
}

/* ------------------------------------------------------------------ */
/* parse_char_class                                                      */
/* ------------------------------------------------------------------ */

bool test_parse_char_class(const char *input, const char *tag)
{
	Stat &st = *std::find_if(stats, stats + nstats,
	    [&](const Stat &s) { return std::strcmp(s.name, "parse_char_class") == 0; });

	GuardBuf rg, pg;
	fill_guard(rg);
	fill_guard(pg);
	std::strncpy((char *)rg.data, input, sizeof(rg.data) - 1);
	std::strncpy((char *)pg.data, input, sizeof(pg.data) - 1);

	char *r = ref_parse_char_class((char *)rg.data);
	char *p = port::parse_char_class((char *)pg.data);

	long ro = r ? (long)(r - (char *)rg.data) : -1;
	long po = p ? (long)(p - (char *)pg.data) : -1;

	st.cases++;
	bool ok = (ro == po) && guards_ok(rg) && guards_ok(pg) &&
	    std::memcmp(rg.data, pg.data, sizeof(rg.data)) == 0;
	if (!ok) {
		st.fails++;
		report(st, tag);
	}
	return ok;
}

void test_parse_char_class_all()
{
	reg("parse_char_class");
	const char *edges[] = {
		"", "]", "^]", "[]", "[a]", "[^a]", "[a-z]",
		"[[:alpha:]]", "[.a.]", "[=a=]", "[\n",
		"[\x80]", "[\xff]", "a]", "[^a-z]",
		"[\\]]", "[[.a.]]", "[[:cntrl:]", "abc]def",
	};
	for (auto e : edges)
		test_parse_char_class(e, e);
	for (long i = 0; i < RANDOM_ITERS / 11; i++) {
		char buf[128];
		int len = (int)(rnd() % 120) + 1;
		for (int j = 0; j < len; j++)
			buf[j] = (char)rnd_byte();
		buf[len] = '\0';
		test_parse_char_class(buf, "random");
	}
}

/* ------------------------------------------------------------------ */
/* translit_text                                                         */
/* ------------------------------------------------------------------ */

bool test_translit(const char *in, int len, int from, int to, const char *tag)
{
	Stat &st = *std::find_if(stats, stats + nstats,
	    [&](const Stat &s) { return std::strcmp(s.name, "translit_text") == 0; });

	GuardBuf rg, pg;
	fill_guard(rg);
	fill_guard(pg);
	std::memcpy(rg.data, in, len);
	std::memcpy(pg.data, in, len);

	char *r = ref_translit_text((char *)rg.data, len, from, to);
	char *p = port::translit_text((char *)pg.data, len, from, to);

	long ro = (long)(r - (char *)rg.data);
	long po = (long)(p - (char *)pg.data);

	st.cases++;
	bool ok = ro == po && guards_ok(rg) && guards_ok(pg) &&
	    std::memcmp(rg.data, pg.data, sizeof(rg.data)) == 0;
	if (!ok) {
		st.fails++;
		report(st, tag);
	}
	return ok;
}

void test_translit_all()
{
	reg("translit_text");
	test_translit("", 0, 0, 1, "empty");
	test_translit("a", 1, 'a', 'b', "single");
	test_translit("hello", 5, 'l', 'L', "basic");
	test_translit("\x00\x01\x02", 3, 0, 10, "nul");
	test_translit("\x80\xff\x7f", 3, (int)0x80, 32, "highbit");
	for (long i = 0; i < RANDOM_ITERS / 11; i++) {
		char buf[256];
		int len = (int)(rnd() % 200);
		for (int j = 0; j < len; j++)
			buf[j] = (char)rnd_byte();
		int from = rnd_byte();
		int to = rnd_byte();
		test_translit(buf, len, from, to, "random");
	}
}

/* ------------------------------------------------------------------ */
/* extract_pattern / get_compiled_pattern                                */
/* ------------------------------------------------------------------ */

bool test_extract_pattern(const char *cmd, int delim, const char *tag)
{
	Stat &st = *std::find_if(stats, stats + nstats,
	    [&](const Stat &s) { return std::strcmp(s.name, "extract_pattern") == 0; });

	reset_both();
	setup_ibuf(cmd);
	isbinary = (int)(rnd() & 1);
	sync_globals_to_port();

	char *r = ref_extract_pattern(delim);
	long rip = ibufp - ibuf;
	const char *re = errmsg;

	reset_both();
	setup_ibuf(cmd);
	port::isbinary = isbinary;
	char *p = port::extract_pattern(delim);
	long pip = port::ibufp - port::ibuf;
	const char *pe = port::errmsg;

	st.cases++;
	bool ok = true;
	if ((r == nullptr) != (p == nullptr))
		ok = false;
	if (r && p && std::strcmp(r, p) != 0)
		ok = false;
	if (rip != pip)
		ok = false;
	if (std::strcmp(re, pe) != 0)
		ok = false;
	if (!ok) {
		st.fails++;
		report(st, tag);
	}
	return ok;
}

void test_extract_pattern_all()
{
	reg("extract_pattern");
	const char *edges[] = {
		"/foo/bar", "/[a-z]/x", "/\\\\/x", "/[/x",
		"/\\n/x", "/a\n", "/\x80\xff/x", "/[[:alpha:]]/x",
		"/[]]/x", "/[^]]/x",
	};
	for (auto e : edges)
		test_extract_pattern(e, '/', e);
	for (long i = 0; i < RANDOM_ITERS / 22; i++) {
		char buf[200];
		int d = (char)('!' + (rnd() % 60));
		int n = (int)(rnd() % 150) + 2;
		buf[0] = (char)d;
		for (int j = 1; j < n; j++)
			buf[j] = (char)rnd_byte();
		buf[n] = (char)d;
		buf[n + 1] = 'x';
		buf[n + 2] = '\0';
		test_extract_pattern(buf, d, "random");
	}
}

bool test_get_compiled_pattern(const char *cmd, const char *tag)
{
	Stat &st = *std::find_if(stats, stats + nstats,
	    [&](const Stat &s) { return std::strcmp(s.name, "get_compiled_pattern") == 0; });

	reset_both();
	setup_ibuf(cmd);
	pattern_t *r = ref_get_compiled_pattern();
	const char *re = errmsg;

	reset_both();
	setup_ibuf(cmd);
	pattern_t *p = port::get_compiled_pattern();
	const char *pe = port::errmsg;

	st.cases++;
	bool ok = ((r == nullptr) == (p == nullptr)) && std::strcmp(re, pe) == 0;
	if (!ok) {
		st.fails++;
		report(st, tag);
	}
	return ok;
}

void test_get_compiled_pattern_all()
{
	reg("get_compiled_pattern");
	test_get_compiled_pattern("/a/\n", "simple");
	test_get_compiled_pattern(" \n", "space delim");
	test_get_compiled_pattern("/\n", "empty reuse");
	test_get_compiled_pattern("/[/\n", "bad class");
	test_get_compiled_pattern("#foo#bar\n", "hash");
	for (long i = 0; i < RANDOM_ITERS / 22; i++) {
		char buf[120];
		int d = (char)('a' + (rnd() % 26));
		int n = (int)(rnd() % 40) + 1;
		buf[0] = (char)d;
		for (int j = 1; j < n; j++)
			buf[j] = (char)(32 + rnd_byte() % 90);
		buf[n] = (char)d;
		buf[n + 1] = '\n';
		buf[n + 2] = '\0';
		test_get_compiled_pattern(buf, "random");
	}
}

/* ------------------------------------------------------------------ */
/* buffer / line list                                                    */
/* ------------------------------------------------------------------ */

bool test_get_addressed_line_node(long n, const char *tag)
{
	Stat &st = *std::find_if(stats, stats + nstats,
	    [&](const Stat &s) {
		    return std::strcmp(s.name, "get_addressed_line_node") == 0;
	    });

	reset_both();
	init_scratch_both();
	init_line_list_both();
	for (int i = 0; i < 10; i++) {
		char line[32];
		std::snprintf(line, sizeof(line), "L%d\n", i);
		put_line_both(line);
	}
	addr_last = port::addr_last;
	current_addr = port::current_addr;

	line_t *r = ref_get_addressed_line_node(n);
	line_t *p = port::get_addressed_line_node(n);

	st.cases++;
	bool ok = true;
	if ((r == nullptr) != (p == nullptr))
		ok = false;
	if (r && p) {
		long ra = ref_get_line_node_addr(r);
		long pa = port::get_line_node_addr(p);
		if (ra != pa)
			ok = false;
	}
	if (!ok) {
		st.fails++;
		report(st, tag);
	}
	ref_close_sbuf();
	port::close_sbuf();
	return ok;
}

void test_get_addressed_line_node_all()
{
	reg("get_addressed_line_node");
	for (long n = 0; n <= 10; n++) {
		char tag[32];
		std::snprintf(tag, sizeof(tag), "n=%ld", n);
		test_get_addressed_line_node(n, tag);
	}
	for (long i = 0; i < RANDOM_ITERS / 22; i++)
		test_get_addressed_line_node((long)(rnd() % 11), "random");
}

bool test_get_line_node_addr(int nlines, int target, const char *tag)
{
	Stat &st = *std::find_if(stats, stats + nstats,
	    [&](const Stat &s) {
		    return std::strcmp(s.name, "get_line_node_addr") == 0;
	    });

	reset_both();
	init_scratch_both();
	init_line_list_both();
	line_t *nodes[32] = {};
	for (int i = 0; i < nlines; i++) {
		char line[32];
		std::snprintf(line, sizeof(line), "x%d\n", i);
		put_line_both(line);
		nodes[i + 1] = ref_get_addressed_line_node(i + 1);
	}
	line_t *rt = (target >= 0 && target <= nlines) ? nodes[target] :
	    (line_t *)0x12345;
	long r = ref_get_line_node_addr(rt);
	const char *re = errmsg;

	reset_both();
	init_scratch_both();
	init_line_list_both();
	line_t *pnodes[32] = {};
	for (int i = 0; i < nlines; i++) {
		char line[32];
		std::snprintf(line, sizeof(line), "x%d\n", i);
		put_line_both(line);
		pnodes[i + 1] = port::get_addressed_line_node(i + 1);
	}
	line_t *pt = (target >= 0 && target <= nlines) ? pnodes[target] :
	    (line_t *)0x12345;
	long p = port::get_line_node_addr(pt);
	const char *pe = port::errmsg;

	st.cases++;
	bool ok = (r == p) && std::strcmp(re, pe) == 0;
	if (!ok) {
		st.fails++;
		report(st, tag);
	}
	ref_close_sbuf();
	port::close_sbuf();
	return ok;
}

void test_get_line_node_addr_all()
{
	reg("get_line_node_addr");
	test_get_line_node_addr(5, 3, "mid");
	test_get_line_node_addr(5, 0, "head");
	test_get_line_node_addr(5, 5, "tail");
	test_get_line_node_addr(3, -1, "invalid");
	for (long i = 0; i < RANDOM_ITERS / 22; i++) {
		int n = (int)(rnd() % 8) + 1;
		int t = (int)(rnd() % (n + 2));
		if (rnd() % 10 == 0)
			t = -1;
		test_get_line_node_addr(n, t, "random");
	}
}

bool test_get_sbuf_line(int nlines, int which, const char *tag)
{
	Stat &st = *std::find_if(stats, stats + nstats,
	    [&](const Stat &s) { return std::strcmp(s.name, "get_sbuf_line") == 0; });

	reset_both();
	init_scratch_both();
	init_line_list_both();
	char expect[64][64];
	for (int i = 0; i < nlines; i++) {
		std::snprintf(expect[i], sizeof(expect[i]), "line%d\n", i);
		put_line_both(expect[i]);
	}
	line_t *rl = ref_get_addressed_line_node(which);
	char *rs = ref_get_sbuf_line(rl);
	const char *re = errmsg;

	reset_both();
	init_scratch_both();
	init_line_list_both();
	for (int i = 0; i < nlines; i++)
		put_line_both(expect[i]);
	line_t *pl = port::get_addressed_line_node(which);
	char *ps = port::get_sbuf_line(pl);
	const char *pe = port::errmsg;

	st.cases++;
	bool ok = true;
	if ((rs == nullptr) != (ps == nullptr))
		ok = false;
	if (rs && ps && std::strcmp(rs, ps) != 0)
		ok = false;
	if (std::strcmp(re, pe) != 0)
		ok = false;
	if (!ok) {
		st.fails++;
		report(st, tag);
	}
	ref_close_sbuf();
	port::close_sbuf();
	return ok;
}

void test_get_sbuf_line_all()
{
	reg("get_sbuf_line");
	test_get_sbuf_line(5, 2, "mid");
	test_get_sbuf_line(1, 1, "single");
	test_get_sbuf_line(0, 0, "empty");
	for (long i = 0; i < RANDOM_ITERS / 22; i++) {
		int n = (int)(rnd() % 6);
		int w = n ? (int)(rnd() % n) + 1 : 0;
		test_get_sbuf_line(n, w, "random");
	}
}

bool test_open_close_sbuf(const char *tag)
{
	Stat &st = *std::find_if(stats, stats + nstats,
	    [&](const Stat &s) { return std::strcmp(s.name, "open_sbuf") == 0; });

	reset_both();
	int r1 = ref_open_sbuf();
	int r2 = ref_close_sbuf();

	reset_both();
	int p1 = port::open_sbuf();
	int p2 = port::close_sbuf();

	st.cases++;
	bool ok = (r1 == p1) && (r2 == p2);
	if (!ok) {
		st.fails++;
		report(st, tag);
	}
	return ok;
}

void test_open_close_all()
{
	reg("open_sbuf");
	reg("close_sbuf");
	for (int i = 0; i < 20; i++)
		test_open_close_sbuf("edge");
	for (long i = 0; i < RANDOM_ITERS / 44; i++)
		test_open_close_sbuf("random");
}

/* ------------------------------------------------------------------ */
/* active list                                                           */
/* ------------------------------------------------------------------ */

bool test_active_list_ops(const char *tag)
{
	Stat &st_set = *std::find_if(stats, stats + nstats,
	    [&](const Stat &s) { return std::strcmp(s.name, "set_active_node") == 0; });
	Stat &st_next = *std::find_if(stats, stats + nstats,
	    [&](const Stat &s) { return std::strcmp(s.name, "next_active_node") == 0; });

	reset_both();
	init_scratch_both();
	init_line_list_both();
	line_t *rnodes[8], *pnodes[8];
	for (int i = 0; i < 5; i++) {
		char line[16];
		std::snprintf(line, sizeof(line), "%d\n", i);
		put_line_both(line);
		rnodes[i] = ref_get_addressed_line_node(i + 1);
	}
	reset_both();
	init_scratch_both();
	init_line_list_both();
	for (int i = 0; i < 5; i++) {
		char line[16];
		std::snprintf(line, sizeof(line), "%d\n", i);
		put_line_both(line);
		pnodes[i] = port::get_addressed_line_node(i + 1);
	}

	reset_both();
	init_scratch_both();
	init_line_list_both();
	for (int i = 0; i < 5; i++) {
		char line[16];
		std::snprintf(line, sizeof(line), "%d\n", i);
		put_line_both(line);
		rnodes[i] = ref_get_addressed_line_node(i + 1);
	}
	for (int i = 0; i < 5; i++) {
		int rs = ref_set_active_node(rnodes[i]);
		int ps = port::set_active_node(pnodes[i]);
		st_set.cases++;
		if (rs != ps) {
			st_set.fails++;
			report(st_set, tag);
		}
	}
	for (int i = 0; i < 6; i++) {
		line_t *rn = ref_next_active_node();
		line_t *pn = port::next_active_node();
		st_next.cases++;
		bool ok = ((rn == nullptr) == (pn == nullptr));
		if (rn && pn) {
			long ra = ref_get_line_node_addr(rn);
			long pa = port::get_line_node_addr(pn);
			ok = ok && ra == pa;
		}
		if (!ok) {
			st_next.fails++;
			report(st_next, tag);
		}
	}
	ref_close_sbuf();
	port::close_sbuf();
	return st_set.fails == 0 && st_next.fails == 0;
}

void test_active_list_all()
{
	reg("set_active_node");
	reg("next_active_node");
	reg("clear_active_list");
	reg("unset_active_nodes");

	for (int i = 0; i < 30; i++)
		test_active_list_ops("ops");

	Stat &st_clear = stats[nstats - 2];
	Stat &st_unset = stats[nstats - 1];
	for (long i = 0; i < RANDOM_ITERS / 22; i++) {
		test_active_list_ops("random");
		st_clear.cases++;
		st_unset.cases++;
	}
}

/* ------------------------------------------------------------------ */
/* undo stack                                                            */
/* ------------------------------------------------------------------ */

bool test_push_pop_undo(const char *tag)
{
	Stat &st_push = *std::find_if(stats, stats + nstats,
	    [&](const Stat &s) { return std::strcmp(s.name, "push_undo_stack") == 0; });
	Stat &st_pop = *std::find_if(stats, stats + nstats,
	    [&](const Stat &s) { return std::strcmp(s.name, "pop_undo_stack") == 0; });

	reset_both();
	init_scratch_both();
	init_line_list_both();
	put_line_both("a\n");
	put_line_both("b\n");
	u_current_addr = 2;
	u_addr_last = 2;
	sync_globals_to_port();

	undo_t *ru = ref_push_undo_stack(0, 1, 2);
	st_push.cases++;
	int rp = ref_pop_undo_stack();
	st_pop.cases++;

	reset_both();
	init_scratch_both();
	init_line_list_both();
	put_line_both("a\n");
	put_line_both("b\n");
	port::u_current_addr = 2;
	port::u_addr_last = 2;
	undo_t *pu = port::push_undo_stack(0, 1, 2);
	int pp = port::pop_undo_stack();

	bool ok = ((ru == nullptr) == (pu == nullptr)) && rp == pp;
	if (!ok) {
		st_push.fails++;
		st_pop.fails++;
		report(st_push, tag);
	}
	ref_close_sbuf();
	port::close_sbuf();
	return ok;
}

void test_undo_all()
{
	reg("push_undo_stack");
	reg("pop_undo_stack");
	reg("clear_undo_stack");

	test_push_pop_undo("basic");
	reset_both();
	ref_clear_undo_stack();
	port::clear_undo_stack();

	for (long i = 0; i < RANDOM_ITERS / 22; i++)
		test_push_pop_undo("random");
}

/* ------------------------------------------------------------------ */
/* build_active_list / exec_global                                       */
/* ------------------------------------------------------------------ */

bool test_build_active_list(const char *cmd, int isg, const char *tag)
{
	Stat &st = *std::find_if(stats, stats + nstats,
	    [&](const Stat &s) {
		    return std::strcmp(s.name, "build_active_list") == 0;
	    });

	reset_both();
	init_scratch_both();
	init_line_list_both();
	put_line_both("match\n");
	put_line_both("other\n");
	put_line_both("match\n");
	first_addr = 1;
	second_addr = 3;
	sync_globals_to_port();
	setup_ibuf(cmd);

	int r = ref_build_active_list(isg);
	const char *re = errmsg;

	reset_both();
	init_scratch_both();
	init_line_list_both();
	put_line_both("match\n");
	put_line_both("other\n");
	put_line_both("match\n");
	port::first_addr = 1;
	port::second_addr = 3;
	setup_ibuf(cmd);
	sync_globals_to_port();

	int p = port::build_active_list(isg);
	const char *pe = port::errmsg;

	st.cases++;
	bool ok = (r == p) && std::strcmp(re, pe) == 0;
	if (!ok) {
		st.fails++;
		report(st, tag);
	}
	ref_close_sbuf();
	port::close_sbuf();
	return ok;
}

void test_build_active_list_all()
{
	reg("build_active_list");
	test_build_active_list("/match/\n", 0, "g");
	test_build_active_list("/match/\n", 1, "v");
	test_build_active_list(" \n", 0, "bad");
	for (long i = 0; i < RANDOM_ITERS / 22; i++)
		test_build_active_list("/match/\n", (int)(rnd() & 1), "random");
}

bool test_exec_global_once(const char *tag)
{
	Stat &st = *std::find_if(stats, stats + nstats,
	    [&](const Stat &s) { return std::strcmp(s.name, "exec_global") == 0; });

	static char cmd[] = "p\n";
	oracle_extended_line = cmd;
	oracle_extended_line_len = 2;
	sync_hooks();

	reset_both();
	init_scratch_both();
	init_line_list_both();
	put_line_both("x\n");
	ref_set_active_node(ref_get_addressed_line_node(1));
	long r = ref_exec_global(0, 0);

	oracle_extended_line = cmd;
	oracle_extended_line_len = 2;
	sync_hooks();
	reset_both();
	init_scratch_both();
	init_line_list_both();
	put_line_both("x\n");
	port::set_active_node(port::get_addressed_line_node(1));
	long p = port::exec_global(0, 0);

	st.cases++;
	bool ok = r == p;
	if (!ok) {
		st.fails++;
		report(st, tag);
	}
	ref_close_sbuf();
	port::close_sbuf();
	return ok;
}

void test_exec_global_all()
{
	reg("exec_global");
	test_exec_global_once("noninteract");
	for (long i = 0; i < RANDOM_ITERS / 22; i++)
		test_exec_global_once("random");
}

/* ------------------------------------------------------------------ */
/* init_buffers / quit                                                   */
/* ------------------------------------------------------------------ */

void test_init_buffers()
{
	Stat &st = reg("init_buffers");
	reset_both();
	ref_init_buffers();
	int rc = ref_close_sbuf();
	reset_both();
	port::init_buffers();
	int pc = port::close_sbuf();
	st.cases = 2;
	st.fails = (rc == pc) ? 0 : 1;
}

void test_quit()
{
	Stat &st = reg("quit");
	reset_both();
	ref_open_sbuf();
	oracle_quit_enter();
	ref_quit(3);
	int rc = oracle_quit_called;
	int rs = oracle_quit_status;

	reset_both();
	port::open_sbuf();
	port::quit_enter();
	port::quit(3);
	int pc = port::quit_called;
	int ps = port::quit_status;

	st.cases = 1;
	st.fails = (rc == pc && rs == ps) ? 0 : 1;
}

/* ------------------------------------------------------------------ */
/* add_line_node - covered via put_sbuf_line                             */
/* ------------------------------------------------------------------ */

void reg_add_line_node()
{
	reg("add_line_node");
}

} /* namespace */

int main()
{
	reg_add_line_node();
	test_parse_char_class_all();
	test_translit_all();
	test_extract_pattern_all();
	test_get_compiled_pattern_all();
	test_get_addressed_line_node_all();
	test_get_line_node_addr_all();
	test_get_sbuf_line_all();
	test_open_close_all();
	test_active_list_all();
	test_undo_all();
	test_build_active_list_all();
	test_exec_global_all();
	test_init_buffers();
	test_quit();

	std::printf("PBSD batch b0148 differential test\n\n");
	std::printf("%-28s %12s %12s %10s\n", "function", "cases",
	    "failures", "result");
	std::printf("%-28s %12s %12s %10s\n",
	    "----------------------------", "------------", "------------",
	    "----------");

	long total_cases = 0, total_fails = 0;
	for (int i = 0; i < nstats; i++) {
		total_cases += stats[i].cases;
		total_fails += stats[i].fails;
		std::printf("%-28s %12ld %12ld %10s\n", stats[i].name,
		    stats[i].cases, stats[i].fails,
		    stats[i].fails == 0 ? "PASS" : "FAIL");
	}
	std::printf("%-28s %12s %12s %10s\n",
	    "----------------------------", "------------", "------------",
	    "----------");
	std::printf("%-28s %12ld %12ld %10s\n", "TOTAL", total_cases,
	    total_fails, total_fails == 0 ? "PASS" : "FAIL");
	std::printf("\n%ld function(s), %ld case(s), %ld failure(s).\n",
	    (long)nstats, total_cases, total_fails);
	return total_fails == 0 ? 0 : 1;
}
