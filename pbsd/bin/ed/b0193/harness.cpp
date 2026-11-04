/*
 * harness.cpp -- differential test for PBSD batch b0193.
 */

import pbsd.bin.ed.b0193;

#include <cerrno>
#include <climits>
#include <csignal>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <regex.h>
#include <string>
#include <string_view>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

namespace port = pbsd::bin_ed::b0193;

namespace pbsd::bin_ed::b0193 {
char *optarg = nullptr;
}

extern "C" {
typedef struct line_t {
	line_t *q_forw;
	line_t *q_back;
	off_t seek;
	int len;
} line_t;

extern char *ibuf;
extern char *ibufp;
extern int ibufsz;
extern int isbinary;
extern int isglobal;
extern int modified;
extern int mutex;
extern int sigflags;
extern long addr_last;
extern long current_addr;
extern long first_addr;
extern long second_addr;
extern long u_addr_last;
extern long u_current_addr;
extern int lineno;
extern int newline_added;
extern int scripted;
extern int patlock;
extern const char *errmsg;
extern long rows;
extern int cols;

extern int oracle_quit_called;
extern int oracle_quit_status;

void oracle_reset_batch(void);
void oracle_quit_enter(void);

int ref_extract_subst_tail(int *flagp, long *np);
char *ref_extract_subst_template(void);
int ref_search_and_replace(regex_t *pat, int gflag, int kth);
int ref_substitute_matching_text(regex_t *pat, line_t *lp, int gflag, int kth);
int ref_apply_subst_template(const char *boln, regmatch_t *rm, int off, int re_nsub);

long ref_read_file(char *fn, long n);
long ref_read_stream(FILE *fp, long n);
int ref_get_stream_line(FILE *fp);
long ref_write_file(char *fn, const char *mode, long n, long m);
long ref_write_stream(FILE *fp, long n, long m);
int ref_put_stream_line(FILE *fp, const char *s, int len);
char *ref_get_extended_line(int *sizep, int nonl);
int ref_get_tty_line(void);
int ref_put_tty_line(const char *s, int l, long n, int gflag);

int ref_extract_addr_range(void);
long ref_next_addr(void);
int ref_exec_command(void);
int ref_check_addr_range(long n, long m);
long ref_get_matching_node_addr(regex_t *pat, int dir);
char *ref_get_filename(void);
int ref_get_shell_command(void);
int ref_append_lines(long n);
int ref_join_lines(long from, long to);
int ref_move_lines(long addr);
int ref_copy_lines(long addr);
int ref_delete_lines(long from, long to);
int ref_display_lines(long from, long to, int gflag);
int ref_mark_line_node(line_t *lp, int n);
long ref_get_marked_node_addr(int n);
void ref_unmark_line_node(line_t *lp);
line_t *ref_dup_line_node(line_t *lp);
int ref_has_trailing_escape(char *s, char *t);
char *ref_strip_escapes(char *s);
void ref_signal_hup(int signo);
void ref_signal_int(int signo);
void ref_handle_hup(int signo);
void ref_handle_int(int signo);
void ref_handle_winch(int signo);
int ref_is_legal_filename(char *s);

line_t *ref_get_addressed_line_node(long n);
long ref_get_line_node_addr(line_t *lp);
char *ref_get_sbuf_line(line_t *lp);
const char *ref_put_sbuf_line(const char *cs);
int ref_open_sbuf(void);
int ref_close_sbuf(void);
}

namespace {

constexpr int ERR = -2;
constexpr int GUARD = 0x7f;
constexpr int GPR = 002;
constexpr int GLS = 004;
constexpr int GNP = 010;
constexpr int GSG = 020;
constexpr int GLB = 001;
constexpr long RANDOM_TARGET = 200000;
constexpr size_t IBUFSZ = 65536;

struct Stat {
	const char *name;
	long cases;
	long fails;
};

Stat stats[64];
int nstats = 0;
long random_iters = 0;

std::uint64_t rng = 0x9e3779b97f4a7c15ULL;

std::uint64_t rnd()
{
	std::uint64_t z = (rng += 0xbf58476d1ce4e5b9ULL);
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return z ^ (z >> 31);
}

int rndb() { return static_cast<int>(rnd() & 0xffu); }

long rnd_long(long lo, long hi)
{
	if (hi <= lo)
		return lo;
	return lo + static_cast<long>(rnd() % static_cast<std::uint64_t>(hi - lo + 1));
}

Stat &reg(const char *name)
{
	stats[nstats++] = {name, 0, 0};
	return stats[nstats - 1];
}

void reset_both()
{
	oracle_reset_batch();
	port::reset_batch();
}

void mirror_globals_to_port()
{
	port::ibufsz = ibufsz;
	port::isbinary = isbinary;
	port::isglobal = isglobal;
	port::modified = modified;
	port::mutex = mutex;
	port::sigflags = sigflags;
	port::addr_last = addr_last;
	port::current_addr = current_addr;
	port::first_addr = first_addr;
	port::second_addr = second_addr;
	port::u_addr_last = u_addr_last;
	port::u_current_addr = u_current_addr;
	port::lineno = lineno;
	port::newline_added = newline_added;
	port::scripted = scripted;
	port::patlock = patlock;
	port::rows = rows;
	port::cols = cols;
	std::memcpy(port::ibuf, ibuf, IBUFSZ);
	if (ibufp)
		port::ibufp = port::ibuf + (ibufp - ibuf);
	else
		port::ibufp = nullptr;
}

void setup_ibuf(const std::string &s, size_t off = 0)
{
	std::memset(ibuf, 0, IBUFSZ);
	size_t n = s.size();
	if (n >= IBUFSZ)
		n = IBUFSZ - 1;
	if (n)
		std::memcpy(ibuf, s.data(), n);
	ibuf[n] = '\0';
	if (off > n)
		off = n;
	ibufp = ibuf + off;
	mirror_globals_to_port();
}

bool scratch_both(const std::vector<std::string> &lines)
{
	if (ref_open_sbuf() < 0)
		return false;
	if (port::open_sbuf() < 0) {
		ref_close_sbuf();
		return false;
	}
	ref_get_addressed_line_node(0);
	port::get_addressed_line_node(0);
	for (const std::string &line : lines) {
		if (line.empty() || line.back() != '\n')
			return false;
		if (!ref_put_sbuf_line(line.c_str()))
			return false;
		if (!port::put_sbuf_line(line.c_str()))
			return false;
	}
	return true;
}

void close_scratch_both()
{
	ref_close_sbuf();
	port::close_sbuf();
}

long ptr_off(const char *base, size_t n, const char *p)
{
	if (!p)
		return -1;
	if (p < base || p > base + static_cast<long>(n))
		return LONG_MIN / 2;
	return p - base;
}

std::string str_or_empty(const char *p)
{
	if (!p)
		return {};
	return std::string(p);
}

bool same_cstr(const char *a, const char *b)
{
	if (!a && !b)
		return true;
	if (!a || !b)
		return false;
	return std::strcmp(a, b) == 0;
}

bool core_equal()
{
	if (isbinary != port::isbinary) return false;
	if (isglobal != port::isglobal) return false;
	if (modified != port::modified) return false;
	if (mutex != port::mutex) return false;
	if (sigflags != port::sigflags) return false;
	if (addr_last != port::addr_last) return false;
	if (current_addr != port::current_addr) return false;
	if (first_addr != port::first_addr) return false;
	if (second_addr != port::second_addr) return false;
	if (u_addr_last != port::u_addr_last) return false;
	if (u_current_addr != port::u_current_addr) return false;
	if (lineno != port::lineno) return false;
	if (newline_added != port::newline_added) return false;
	if (scripted != port::scripted) return false;
	if (patlock != port::patlock) return false;
	if (rows != port::rows) return false;
	if (cols != port::cols) return false;
	if (ptr_off(ibuf, IBUFSZ, ibufp) != ptr_off(port::ibuf, IBUFSZ, port::ibufp))
		return false;
	if (!same_cstr(errmsg, port::errmsg))
		return false;
	return true;
}

bool sbuf_equal()
{
	if (addr_last != port::addr_last)
		return false;
	for (long i = 1; i <= addr_last; ++i) {
		line_t *rlp = ref_get_addressed_line_node(i);
		port::line_t *plp = port::get_addressed_line_node(i);
		if (!rlp || !plp)
			return false;
		if (rlp->len != plp->len)
			return false;
		char *rs = ref_get_sbuf_line(rlp);
		char *ps = port::get_sbuf_line(plp);
		if (!rs || !ps)
			return false;
		if (std::memcmp(rs, ps, static_cast<size_t>(rlp->len)) != 0)
			return false;
	}
	return true;
}

void note_case(Stat &st, bool ok, bool random_case = false)
{
	st.cases++;
	if (!ok)
		st.fails++;
	if (random_case)
		random_iters++;
}

std::string rand_line(int max_len = 32, bool with_high = true, bool with_nul = true)
{
	int n = static_cast<int>(rnd() % static_cast<std::uint64_t>(max_len + 1));
	std::string s;
	s.reserve(static_cast<size_t>(n + 1));
	for (int i = 0; i < n; ++i) {
		int v = rndb();
		if (!with_nul && v == 0)
			v = 'x';
		if (!with_high && v >= 0x80)
			v = (v % 95) + 32;
		if (v == '\n')
			v = 'N';
		s.push_back(static_cast<char>(v));
	}
	s.push_back('\n');
	return s;
}

std::string mktemp_with_data(const std::string &data)
{
	char tmpl[] = "/tmp/pbsd_b0193_XXXXXX";
	int fd = mkstemp(tmpl);
	if (fd < 0)
		return {};
	const char *p = data.data();
	size_t left = data.size();
	while (left) {
		ssize_t w = write(fd, p, left);
		if (w <= 0)
			break;
		p += w;
		left -= static_cast<size_t>(w);
	}
	close(fd);
	return std::string(tmpl);
}

std::string read_whole_file(const std::string &path)
{
	std::string out;
	int fd = open(path.c_str(), O_RDONLY);
	if (fd < 0)
		return out;
	char buf[4096];
	for (;;) {
		ssize_t n = read(fd, buf, sizeof(buf));
		if (n <= 0)
			break;
		out.append(buf, static_cast<size_t>(n));
	}
	close(fd);
	return out;
}

template <typename F>
auto with_stdin(const std::string &input, F fn) -> decltype(fn())
{
	int p[2];
	pipe(p);
	if (!input.empty())
		(void)write(p[1], input.data(), input.size());
	close(p[1]);
	int saved = dup(STDIN_FILENO);
	dup2(p[0], STDIN_FILENO);
	close(p[0]);
	clearerr(stdin);
	auto r = fn();
	clearerr(stdin);
	dup2(saved, STDIN_FILENO);
	close(saved);
	return r;
}

template <typename F>
auto quiet_stdio(F fn) -> decltype(fn())
{
	int dn = open("/dev/null", O_WRONLY);
	int so = dup(STDOUT_FILENO);
	int se = dup(STDERR_FILENO);
	dup2(dn, STDOUT_FILENO);
	dup2(dn, STDERR_FILENO);
	close(dn);
	auto r = fn();
	fflush(stdout);
	fflush(stderr);
	dup2(so, STDOUT_FILENO);
	dup2(se, STDERR_FILENO);
	close(so);
	close(se);
	return r;
}

std::string capture_stdout(void (*fn)(void *), void *arg)
{
	int p[2];
	pipe(p);
	int so = dup(STDOUT_FILENO);
	dup2(p[1], STDOUT_FILENO);
	close(p[1]);
	fn(arg);
	fflush(stdout);
	dup2(so, STDOUT_FILENO);
	close(so);
	std::string out;
	char buf[1024];
	for (;;) {
		ssize_t n = read(p[0], buf, sizeof(buf));
		if (n <= 0)
			break;
		out.append(buf, static_cast<size_t>(n));
	}
	close(p[0]);
	return out;
}

regex_t *mk_re(const char *pat)
{
	regex_t *re = static_cast<regex_t *>(std::malloc(sizeof(regex_t)));
	if (!re)
		return nullptr;
	if (regcomp(re, pat, 0) != 0) {
		std::free(re);
		return nullptr;
	}
	return re;
}

void free_re(regex_t *re)
{
	if (!re)
		return;
	regfree(re);
	std::free(re);
}

void test_extract_subst_tail()
{
	Stat &st = reg("extract_subst_tail");
	auto run = [&](const std::string &cmd, int g) {
		reset_both();
		isglobal = g;
		setup_ibuf(cmd);
		int rf = 0;
		long rn = 0;
		int rr = ref_extract_subst_tail(&rf, &rn);
		long roff = ptr_off(ibuf, IBUFSZ, ibufp);

		int pf = 0;
		long pn = 0;
		int pr = port::extract_subst_tail(&pf, &pn);
		long poff = ptr_off(port::ibuf, IBUFSZ, port::ibufp);

		bool ok = (rr == pr && rf == pf && rn == pn && roff == poff && core_equal());
		note_case(st, ok);
	};

	run("\n", 0);
	run("/a/\n", 0);
	run("/a/g\n", 0);
	run("/a/1\n", 0);
	run("/\x80\xff/\n", 1);
	run("/x/99999\n", 0);

	for (long i = 0; i < 12000; ++i) {
		char d = "/#|@^"[rnd() % 5];
		std::string rhs = rand_line(20, true, false);
		for (char &c : rhs) {
			if (c == d || c == '\n')
				c = 'Q';
		}
		std::string cmd;
		cmd.push_back(d);
		cmd += rhs;
		cmd.push_back(d);
		if ((rnd() & 1) == 0)
			cmd.push_back('g');
		else if ((rnd() & 1) == 0)
			cmd += std::to_string(static_cast<int>(rnd() % 9) + 1);
		cmd.push_back('\n');
		run(cmd, static_cast<int>(rnd() & 1));
		random_iters++;
	}
}

void test_extract_subst_template()
{
	Stat &st = reg("extract_subst_template");
	auto run = [&](const std::string &cmd, const std::string &tty, int g) {
		reset_both();
		isglobal = g;
		setup_ibuf(cmd);
		char *rr = with_stdin(tty, [] { return ref_extract_subst_template(); });
		long roff = ptr_off(ibuf, IBUFSZ, ibufp);
		std::string rs = str_or_empty(rr);

		char *pr = with_stdin(tty, [] { return port::extract_subst_template(); });
		long poff = ptr_off(port::ibuf, IBUFSZ, port::ibufp);
		std::string ps = str_or_empty(pr);

		bool ok = ((rr == nullptr) == (pr == nullptr) && rs == ps &&
		    roff == poff && core_equal());
		note_case(st, ok);
	};

	run("/\n", "", 1);
	run("/x/\n", "", 0);
	run("/\x80\xff/\n", "", 0);
	run("/ab\\\n", "cd/\n", 0);
	run("/boundaryboundaryboundaryboundary/\n", "", 1);

	/* prime previous template and test % reuse */
	reset_both();
	setup_ibuf("/prev/\n");
	(void)ref_extract_subst_template();
	(void)port::extract_subst_template();
	note_case(st, core_equal());
	reset_both();
	setup_ibuf("/%/\n");
	char *r = ref_extract_subst_template();
	char *p = port::extract_subst_template();
	note_case(st, ((r == nullptr) == (p == nullptr) && str_or_empty(r) == str_or_empty(p) &&
	    core_equal()));

	for (long i = 0; i < 12000; ++i) {
		char d = "/#|@^"[rnd() % 5];
		std::string body = rand_line(24, true, false);
		for (char &c : body) {
			if (c == d || c == '\n')
				c = static_cast<char>('a' + (rnd() % 26));
		}
		std::string cmd;
		cmd.push_back(d);
		cmd += body;
		cmd.push_back(d);
		cmd.push_back('\n');
		run(cmd, "", static_cast<int>(rnd() & 1));
		random_iters++;
	}
}

void test_search_and_replace()
{
	Stat &st = reg("search_and_replace");
	auto run = [&](const std::vector<std::string> &lines, const char *templ, int gflag, int kth) {
		reset_both();
		if (!scratch_both(lines)) {
			note_case(st, false);
			return;
		}
		first_addr = 1;
		second_addr = addr_last;
		setup_ibuf(std::string("/") + templ + "/\n");
		(void)ref_extract_subst_template();
		(void)port::extract_subst_template();
		regex_t *rrx = mk_re("a");
		regex_t *prx = mk_re("a");
		int rr = quiet_stdio([&] { return ref_search_and_replace(rrx, gflag, kth); });
		int pr = quiet_stdio([&] { return port::search_and_replace(prx, gflag, kth); });
		bool ok = (rr == pr && core_equal() && sbuf_equal());
		note_case(st, ok);
		free_re(rrx);
		free_re(prx);
		close_scratch_both();
	};

	run({"a\n", "ba\n", "xyz\n"}, "Z", 0, 0);
	run({"bbb\n", "ccc\n"}, "Z", GLB, 0);
	run({"aaaa\n"}, "&X", GSG, 0);
	run({std::string("a\0a\n", 4)}, "Q", GSG, 0);
	run({"onea\n", "twoa\n"}, "R", GPR, 2);

	for (long i = 0; i < 1500; ++i) {
		std::vector<std::string> lines;
		int n = static_cast<int>(rnd() % 5) + 1;
		for (int j = 0; j < n; ++j)
			lines.push_back(rand_line(18, true, true));
		const char *tmpl = (rnd() & 1) ? "X" : "&Q";
		int gflag = 0;
		if (rnd() & 1) gflag |= GSG;
		if (rnd() & 1) gflag |= GLB;
		run(lines, tmpl, gflag, static_cast<int>(rnd() % 3));
		random_iters++;
	}
}

void test_substitute_matching_text()
{
	Stat &st = reg("substitute_matching_text");
	auto run = [&](const std::string &line, const char *templ, int gflag, int kth) {
		reset_both();
		if (!scratch_both({line})) {
			note_case(st, false);
			return;
		}
		setup_ibuf(std::string("/") + templ + "/\n");
		(void)ref_extract_subst_template();
		(void)port::extract_subst_template();
		regex_t *rrx = mk_re("a");
		regex_t *prx = mk_re("a");
		int rr = ref_substitute_matching_text(rrx, ref_get_addressed_line_node(1), gflag, kth);
		int pr = port::substitute_matching_text(prx, port::get_addressed_line_node(1), gflag, kth);
		bool ok = (rr == pr && core_equal());
		note_case(st, ok);
		free_re(rrx);
		free_re(prx);
		close_scratch_both();
	};

	run("a\n", "Z", 0, 0);
	run("bbb\n", "Z", 0, 0);
	run(std::string("a\0a\n", 4), "Q", GSG, 0);
	run(std::string("\x80", 1) + "a" + std::string("\xff\n", 2), "R", GSG, 2);
	run("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n", "&", GSG, 5);

	for (long i = 0; i < 2500; ++i) {
		int gflag = (rnd() & 1) ? GSG : 0;
		run(rand_line(24, true, true), (rnd() & 1) ? "X" : "&", gflag,
		    static_cast<int>(rnd() % 5));
		random_iters++;
	}
}

void test_apply_subst_template()
{
	Stat &st = reg("apply_subst_template");
	auto run = [&](const char *tmpl, const std::string &boln, int so, int eo, int off, int nsub) {
		reset_both();
		setup_ibuf(std::string("/") + tmpl + "/\n");
		(void)ref_extract_subst_template();
		(void)port::extract_subst_template();
		regmatch_t rrm[10] = {};
		regmatch_t prm[10] = {};
		rrm[0].rm_so = so;
		rrm[0].rm_eo = eo;
		prm[0].rm_so = so;
		prm[0].rm_eo = eo;
		if (nsub > 1) {
			rrm[1].rm_so = 0;
			rrm[1].rm_eo = eo > 0 ? eo - 1 : 0;
			prm[1] = rrm[1];
		}
		int rr = ref_apply_subst_template(boln.c_str(), rrm, off, nsub);
		int pr = port::apply_subst_template(boln.c_str(), prm, off, nsub);
		note_case(st, rr == pr && core_equal());
	};

	run("&", "abc", 0, 2, 0, 0);
	run("\\1", "abc", 0, 3, 1, 1);
	run("x&y", "a", 0, 1, 0, 0);
	run("\x80&\xff", "xyz", 1, 3, 2, 0);
	run("boundaryboundary", "aaaaaaaaaaaaaaaaaaaaaaaaaaaa", 3, 12, 4, 0);

	for (long i = 0; i < 18000; ++i) {
		std::string boln = rand_line(20, true, false);
		if (!boln.empty() && boln.back() == '\n')
			boln.pop_back();
		int len = static_cast<int>(boln.size());
		int so = len ? static_cast<int>(rnd() % len) : 0;
		int eo = len ? static_cast<int>(so + (rnd() % (len - so + 1))) : 0;
		int off = static_cast<int>(rnd() % 8);
		const char *tmpl = (rnd() & 1) ? "&" : "X\\1Y";
		run(tmpl, boln, so, eo, off, 1);
		random_iters++;
	}
}

void test_read_file()
{
	Stat &st = reg("read_file");
	auto run = [&](const std::string &fdata, long at) {
		std::string path = mktemp_with_data(fdata);
		if (path.empty()) {
			note_case(st, false);
			return;
		}
		reset_both();
		scripted = 1;
		mirror_globals_to_port();
		if (!scratch_both({"x\n", "y\n"})) {
			note_case(st, false);
			unlink(path.c_str());
			return;
		}
		char rfnbuf[PATH_MAX];
		char pfnbuf[PATH_MAX];
		std::snprintf(rfnbuf, sizeof(rfnbuf), "%s", path.c_str());
		std::snprintf(pfnbuf, sizeof(pfnbuf), "%s", path.c_str());
		long rr = quiet_stdio([&] { return ref_read_file(rfnbuf, at); });
		long pr = quiet_stdio([&] { return port::read_file(pfnbuf, at); });
		bool ok = (rr == pr && core_equal() && sbuf_equal() &&
		    std::strcmp(rfnbuf, pfnbuf) == 0);
		note_case(st, ok);
		close_scratch_both();
		unlink(path.c_str());
	};

	run("", 0);
	run("a", 0);
	run("a\n", 0);
	run(std::string("\0\x80\xff", 3), 0);
	run(std::string(520, 'B') + "\n", 1);

	for (long i = 0; i < 300; ++i) {
		run(rand_line(48, true, true), rnd_long(0, 1));
		random_iters++;
	}
}

void test_read_stream()
{
	Stat &st = reg("read_stream");
	auto run = [&](const std::string &fdata, long at) {
		std::string path = mktemp_with_data(fdata);
		if (path.empty()) {
			note_case(st, false);
			return;
		}
		FILE *rfp = std::fopen(path.c_str(), "rb");
		FILE *pfp = std::fopen(path.c_str(), "rb");
		if (!rfp || !pfp) {
			if (rfp) std::fclose(rfp);
			if (pfp) std::fclose(pfp);
			unlink(path.c_str());
			note_case(st, false);
			return;
		}
		reset_both();
		if (!scratch_both({"a\n", "b\n"})) {
			std::fclose(rfp);
			std::fclose(pfp);
			unlink(path.c_str());
			note_case(st, false);
			return;
		}
		long rr = ref_read_stream(rfp, at);
		long pr = port::read_stream(pfp, at);
		bool ok = (rr == pr && core_equal() && sbuf_equal());
		note_case(st, ok);
		std::fclose(rfp);
		std::fclose(pfp);
		close_scratch_both();
		unlink(path.c_str());
	};

	run("", 0);
	run("x", 0);
	run("x\n", 1);
	run(std::string("\0A\n", 3), 0);
	run(std::string(600, 'C') + "\n", 0);
	for (long i = 0; i < 500; ++i) {
		run(rand_line(40, true, true), rnd_long(0, 1));
		random_iters++;
	}
}

void test_get_stream_line()
{
	Stat &st = reg("get_stream_line");
	auto run = [&](const std::string &data) {
		std::string path = mktemp_with_data(data);
		if (path.empty()) {
			note_case(st, false);
			return;
		}
		FILE *rfp = std::fopen(path.c_str(), "rb");
		FILE *pfp = std::fopen(path.c_str(), "rb");
		if (!rfp || !pfp) {
			if (rfp) std::fclose(rfp);
			if (pfp) std::fclose(pfp);
			unlink(path.c_str());
			note_case(st, false);
			return;
		}
		reset_both();
		for (;;) {
			int rr = ref_get_stream_line(rfp);
			int pr = port::get_stream_line(pfp);
			if (!(rr == pr && core_equal())) {
				note_case(st, false);
				break;
			}
			if (rr <= 0) {
				note_case(st, true);
				break;
			}
		}
		std::fclose(rfp);
		std::fclose(pfp);
		unlink(path.c_str());
	};

	run("");
	run("a");
	run("a\n");
	run(std::string("\0\x80\n", 3));
	run(std::string(700, 'D') + "\n");
	for (long i = 0; i < 3000; ++i) {
		std::string d;
		int ln = static_cast<int>(rnd() % 4) + 1;
		for (int j = 0; j < ln; ++j)
			d += rand_line(24, true, true);
		run(d);
		random_iters++;
	}
}

void test_write_file()
{
	Stat &st = reg("write_file");
	auto run = [&](const std::vector<std::string> &lines, long n, long m, const char *mode) {
		char rt[] = "/tmp/pbsd_wfr_XXXXXX";
		char pt[] = "/tmp/pbsd_wfp_XXXXXX";
		int rfd = mkstemp(rt);
		int pfd = mkstemp(pt);
		if (rfd < 0 || pfd < 0) {
			if (rfd >= 0) close(rfd);
			if (pfd >= 0) close(pfd);
			note_case(st, false);
			return;
		}
		close(rfd);
		close(pfd);
		reset_both();
		scripted = 1;
		mirror_globals_to_port();
		if (!scratch_both(lines)) {
			note_case(st, false);
			unlink(rt);
			unlink(pt);
			return;
		}
		long rr = quiet_stdio([&] { return ref_write_file(rt, mode, n, m); });
		long pr = quiet_stdio([&] { return port::write_file(pt, mode, n, m); });
		std::string ro = read_whole_file(rt);
		std::string po = read_whole_file(pt);
		bool ok = (rr == pr && ro == po && core_equal() && sbuf_equal());
		note_case(st, ok);
		close_scratch_both();
		unlink(rt);
		unlink(pt);
	};

	run({"a\n", "b\n"}, 1, 2, "w");
	run({std::string("a\0\n", 3)}, 1, 1, "w");
	run({"\x80\xff\n"}, 1, 1, "w");
	run({std::string(512, 'E') + "\n"}, 1, 1, "w");
	run({"x\n"}, 1, 1, "a");
	for (long i = 0; i < 300; ++i) {
		std::vector<std::string> lines;
		int k = static_cast<int>(rnd() % 4) + 1;
		for (int j = 0; j < k; ++j)
			lines.push_back(rand_line(20, true, true));
		run(lines, 1, static_cast<long>(k), (rnd() & 1) ? "w" : "a");
		random_iters++;
	}
}

void test_write_stream()
{
	Stat &st = reg("write_stream");
	auto run = [&](const std::vector<std::string> &lines, long n, long m) {
		FILE *rfp = tmpfile();
		FILE *pfp = tmpfile();
		if (!rfp || !pfp) {
			if (rfp) std::fclose(rfp);
			if (pfp) std::fclose(pfp);
			note_case(st, false);
			return;
		}
		reset_both();
		if (!scratch_both(lines)) {
			std::fclose(rfp);
			std::fclose(pfp);
			note_case(st, false);
			return;
		}
		long rr = ref_write_stream(rfp, n, m);
		long pr = port::write_stream(pfp, n, m);
		std::fflush(rfp);
		std::fflush(pfp);
		std::rewind(rfp);
		std::rewind(pfp);
		std::string ro;
		std::string po;
		char rb[1024];
		char pb[1024];
		for (;;) {
			size_t rn = std::fread(rb, 1, sizeof(rb), rfp);
			size_t pn = std::fread(pb, 1, sizeof(pb), pfp);
			if (rn != pn) {
				ro.push_back('\1');
				break;
			}
			ro.append(rb, rn);
			po.append(pb, pn);
			if (rn == 0)
				break;
		}
		bool ok = (rr == pr && ro == po && core_equal());
		note_case(st, ok);
		std::fclose(rfp);
		std::fclose(pfp);
		close_scratch_both();
	};

	run({"a\n", "b\n"}, 1, 2);
	run({std::string("a\0\n", 3)}, 1, 1);
	run({"\x80\xff\n"}, 1, 1);
	run({std::string(520, 'F') + "\n"}, 1, 1);
	run({}, 0, 0);
	for (long i = 0; i < 500; ++i) {
		std::vector<std::string> lines;
		int k = static_cast<int>(rnd() % 4) + 1;
		for (int j = 0; j < k; ++j)
			lines.push_back(rand_line(22, true, true));
		run(lines, 1, k);
		random_iters++;
	}
}

void test_put_stream_line()
{
	Stat &st = reg("put_stream_line");
	auto run = [&](const std::string &s, int len) {
		FILE *rfp = tmpfile();
		FILE *pfp = tmpfile();
		if (!rfp || !pfp) {
			if (rfp) std::fclose(rfp);
			if (pfp) std::fclose(pfp);
			note_case(st, false);
			return;
		}
		int rr = ref_put_stream_line(rfp, s.data(), len);
		int pr = port::put_stream_line(pfp, s.data(), len);
		std::fflush(rfp);
		std::fflush(pfp);
		std::rewind(rfp);
		std::rewind(pfp);
		std::string ro;
		std::string po;
		char rb[256], pb[256];
		for (;;) {
			size_t rn = std::fread(rb, 1, sizeof(rb), rfp);
			size_t pn = std::fread(pb, 1, sizeof(pb), pfp);
			if (rn != pn) {
				ro.push_back('\1');
				break;
			}
			ro.append(rb, rn);
			po.append(pb, pn);
			if (rn == 0) break;
		}
		bool ok = (rr == pr && ro == po && core_equal());
		note_case(st, ok);
		std::fclose(rfp);
		std::fclose(pfp);
	};

	run("", 0);
	run("a", 1);
	run(std::string("\0", 1), 1);
	run(std::string("\x80\xff", 2), 2);
	run(std::string(512, 'G'), 512);
	for (long i = 0; i < 8000; ++i) {
		std::string s = rand_line(30, true, true);
		if (!s.empty() && s.back() == '\n')
			s.pop_back();
		run(s, static_cast<int>(s.size()));
		random_iters++;
	}
}

void test_get_extended_line()
{
	Stat &st = reg("get_extended_line");
	auto run = [&](const std::string &cmd, const std::string &tty, int nonl) {
		reset_both();
		setup_ibuf(cmd);
		int rsz = -9;
		char *rr = with_stdin(tty, [&] { return ref_get_extended_line(&rsz, nonl); });
		long roff = ptr_off(ibuf, IBUFSZ, rr);
		std::string rtxt = rr ? std::string(rr) : std::string();
		long riboff = ptr_off(ibuf, IBUFSZ, ibufp);

		int psz = -11;
		char *pr = with_stdin(tty, [&] { return port::get_extended_line(&psz, nonl); });
		long poff = ptr_off(port::ibuf, IBUFSZ, pr);
		std::string ptxt = pr ? std::string(pr) : std::string();
		long piboff = ptr_off(port::ibuf, IBUFSZ, port::ibufp);

		bool ok = ((rr == nullptr) == (pr == nullptr) && rsz == psz &&
		    roff == poff && rtxt == ptxt && riboff == piboff && core_equal());
		note_case(st, ok);
	};

	run("x\n", "", 0);
	run("x\\\n", "y\n", 0);
	run("x\\\n", "y\n", 1);
	run(std::string("\x80\\\n", 3), "\xff\n", 0);
	run(std::string(510, 'H') + "\n", "", 0);
	for (long i = 0; i < 9000; ++i) {
		std::string cmd = rand_line(20, true, false);
		if ((rnd() & 1) && cmd.size() >= 2) {
			cmd[cmd.size() - 2] = '\\';
		}
		std::string tty = rand_line(12, true, false);
		run(cmd, tty, static_cast<int>(rnd() & 1));
		random_iters++;
	}
}

void test_get_tty_line()
{
	Stat &st = reg("get_tty_line");
	auto run = [&](const std::string &in) {
		reset_both();
		int rr = with_stdin(in, [] { return ref_get_tty_line(); });
		long roff = ptr_off(ibuf, IBUFSZ, ibufp);
		std::string rib = std::string(ibuf, ibuf + ((rr > 0) ? rr : 0));
		int pr = with_stdin(in, [] { return port::get_tty_line(); });
		long poff = ptr_off(port::ibuf, IBUFSZ, port::ibufp);
		std::string pib = std::string(port::ibuf, port::ibuf + ((pr > 0) ? pr : 0));
		bool ok = (rr == pr && roff == poff && rib == pib && core_equal());
		note_case(st, ok);
	};

	run("");
	run("\n");
	run("a\n");
	run(std::string("\0\n", 2));
	run(std::string("\x80\xff\n", 3));
	run(std::string(700, 'I') + "\n");
	for (long i = 0; i < 4000; ++i) {
		std::string in = rand_line(30, true, true);
		if ((rnd() & 7) == 0 && !in.empty())
			in.pop_back();
		run(in);
		random_iters++;
	}
}

struct PutTTYCall {
	const char *s;
	int l;
	long n;
	int g;
	int ret;
};

void do_ref_put_tty(void *arg)
{
	PutTTYCall *c = static_cast<PutTTYCall *>(arg);
	c->ret = ref_put_tty_line(c->s, c->l, c->n, c->g);
}

void do_port_put_tty(void *arg)
{
	PutTTYCall *c = static_cast<PutTTYCall *>(arg);
	c->ret = port::put_tty_line(c->s, c->l, c->n, c->g);
}

void test_put_tty_line()
{
	Stat &st = reg("put_tty_line");
	auto run = [&](const std::string &txt, int gflag) {
		reset_both();
		scripted = 1;
		rows = 200;
		cols = 72;
		mirror_globals_to_port();
		PutTTYCall rc{txt.data(), static_cast<int>(txt.size()), 7, gflag, 0};
		PutTTYCall pc{txt.data(), static_cast<int>(txt.size()), 7, gflag, 0};
		std::string ro = capture_stdout(do_ref_put_tty, &rc);
		std::string po = capture_stdout(do_port_put_tty, &pc);
		bool ok = (rc.ret == pc.ret && ro == po && core_equal());
		note_case(st, ok);
	};

	run("", 0);
	run("a", 0);
	run(std::string("\0", 1), 0);
	run(std::string("\x80\xff", 2), GLS);
	run(std::string(520, 'J'), GLS | GNP);
	for (long i = 0; i < 3000; ++i) {
		std::string t = rand_line(32, true, true);
		if (!t.empty() && t.back() == '\n')
			t.pop_back();
		int g = 0;
		if (rnd() & 1) g |= GLS;
		if (rnd() & 1) g |= GNP;
		run(t, g);
		random_iters++;
	}
}

void test_extract_addr_range()
{
	Stat &st = reg("extract_addr_range");
	auto run = [&](const std::string &cmd, long ca, long al) {
		reset_both();
		current_addr = ca;
		addr_last = al;
		setup_ibuf(cmd);
		int rr = ref_extract_addr_range();
		int pr = port::extract_addr_range();
		bool ok = (rr == pr && core_equal());
		note_case(st, ok);
	};

	run("\n", 0, 5);
	run("1,3\n", 1, 8);
	run(".,$p\n", 2, 9);
	run("0\n", 1, 5);
	run("999999999999999999\n", 1, 5);

	for (long i = 0; i < 10000; ++i) {
		long a = rnd_long(0, 10);
		long b = rnd_long(0, 10);
		std::string cmd = std::to_string(a);
		cmd.push_back((rnd() & 1) ? ',' : ';');
		cmd += std::to_string(b);
		cmd.push_back('\n');
		run(cmd, rnd_long(0, 10), rnd_long(1, 12));
		random_iters++;
	}
}

void test_next_addr()
{
	Stat &st = reg("next_addr");
	auto run = [&](const std::string &cmd, long ca, long al) {
		reset_both();
		current_addr = ca;
		addr_last = al;
		setup_ibuf(cmd);
		long rr = ref_next_addr();
		long roff = ptr_off(ibuf, IBUFSZ, ibufp);
		long pr = port::next_addr();
		long poff = ptr_off(port::ibuf, IBUFSZ, port::ibufp);
		bool ok = (rr == pr && roff == poff && core_equal());
		note_case(st, ok);
	};

	run("\n", 1, 5);
	run(".\n", 2, 6);
	run("$\n", 2, 6);
	run("+1\n", 2, 6);
	run("-1\n", 2, 6);
	run("%\n", 2, 6);
	run("999999999999999999\n", 1, 5);
	for (long i = 0; i < 10000; ++i) {
		int kind = static_cast<int>(rnd() % 6);
		std::string cmd;
		switch (kind) {
		case 0: cmd = std::to_string(rnd_long(0, 9)); break;
		case 1: cmd = "+" + std::to_string(rnd_long(0, 9)); break;
		case 2: cmd = "-" + std::to_string(rnd_long(0, 9)); break;
		case 3: cmd = "."; break;
		case 4: cmd = "$"; break;
		default: cmd = "%"; break;
		}
		cmd.push_back('\n');
		run(cmd, rnd_long(0, 12), rnd_long(1, 12));
		random_iters++;
	}
}

void test_exec_command()
{
	Stat &st = reg("exec_command");
	auto run = [&](const std::vector<std::string> &lines, const std::string &cmd) {
		reset_both();
		if (!scratch_both(lines)) {
			note_case(st, false);
			return;
		}
		scripted = 1;
		current_addr = addr_last ? 1 : 0;
		setup_ibuf(cmd);
		int rr = quiet_stdio([] { return ref_exec_command(); });
		int pr = quiet_stdio([] { return port::exec_command(); });
		bool ok = (rr == pr && core_equal() && sbuf_equal());
		note_case(st, ok);
		close_scratch_both();
	};

	run({"a\n"}, "p\n");
	run({"a\n", "b\n"}, "1,2d\n");
	run({"a\n", "b\n"}, "1,2j\n");
	run({"aa\n"}, "1s/a/b/\n");
	run({"a\n", "b\n"}, "1,2n\n");
	for (long i = 0; i < 1200; ++i) {
		std::vector<std::string> lines;
		int k = static_cast<int>(rnd() % 4) + 1;
		for (int j = 0; j < k; ++j)
			lines.push_back(rand_line(12, false, false));
		const char *cmds[] = {"p\n", "n\n", "1,1d\n", "1,1p\n", "1,1s/a/b/\n"};
		run(lines, cmds[rnd() % 5]);
		random_iters++;
	}
}

void test_check_addr_range()
{
	Stat &st = reg("check_addr_range");
	auto run = [&](long n, long m, long al) {
		reset_both();
		addr_last = al;
		mirror_globals_to_port();
		int rr = ref_check_addr_range(n, m);
		int pr = port::check_addr_range(n, m);
		note_case(st, rr == pr && core_equal());
	};

	run(1, 1, 1);
	run(1, 2, 1);
	run(0, 1, 5);
	run(1, 5, 5);
	run(5, 1, 5);
	for (long i = 0; i < 10000; ++i) {
		run(rnd_long(0, 12), rnd_long(0, 12), rnd_long(0, 12));
		random_iters++;
	}
}

void test_get_matching_node_addr()
{
	Stat &st = reg("get_matching_node_addr");
	auto run = [&](const std::vector<std::string> &lines, long ca, int dir) {
		reset_both();
		if (!scratch_both(lines)) {
			note_case(st, false);
			return;
		}
		current_addr = ca;
		port::current_addr = ca;
		regex_t *rrx = mk_re("a");
		regex_t *prx = mk_re("a");
		long rr = ref_get_matching_node_addr(rrx, dir);
		long pr = port::get_matching_node_addr(prx, dir);
		note_case(st, rr == pr && core_equal());
		free_re(rrx);
		free_re(prx);
		close_scratch_both();
	};

	run({"a\n", "b\n"}, 1, 1);
	run({"a\n", "b\n"}, 2, 0);
	run({"b\n", "c\n"}, 1, 1);
	run({std::string("a\0\n", 3)}, 1, 1);
	run({std::string("\x80", 1) + "a\n", std::string("\xff\n", 2)}, 1, 1);
	for (long i = 0; i < 4000; ++i) {
		std::vector<std::string> lines;
		int k = static_cast<int>(rnd() % 5) + 1;
		for (int j = 0; j < k; ++j)
			lines.push_back(rand_line(16, true, true));
		run(lines, rnd_long(0, k), static_cast<int>(rnd() & 1));
		random_iters++;
	}
}

void test_get_filename()
{
	Stat &st = reg("get_filename");
	auto run = [&](const std::string &cmd) {
		reset_both();
		setup_ibuf(cmd);
		char *rr = ref_get_filename();
		long roff = ptr_off(ibuf, IBUFSZ, rr);
		std::string rs = str_or_empty(rr);
		char *pr = port::get_filename();
		long poff = ptr_off(port::ibuf, IBUFSZ, pr);
		std::string ps = str_or_empty(pr);
		bool ok = ((rr == nullptr) == (pr == nullptr) && roff == poff && rs == ps &&
		    core_equal());
		note_case(st, ok);
	};

	run("\n");
	run(" file\n");
	run("a\\ b\n");
	run(std::string("x\x80\xff\n", 4));
	run(std::string(500, 'K') + "\n");
	for (long i = 0; i < 9000; ++i) {
		std::string f;
		int n = static_cast<int>(rnd() % 20);
		for (int j = 0; j < n; ++j) {
			char c = static_cast<char>(32 + (rnd() % 90));
			if (c == '\n') c = 'n';
			f.push_back(c);
		}
		f.push_back('\n');
		run(f);
		random_iters++;
	}
}

void test_get_shell_command()
{
	Stat &st = reg("get_shell_command");
	auto run = [&](const std::string &cmd) {
		reset_both();
		setup_ibuf(cmd);
		int rr = ref_get_shell_command();
		long roff = ptr_off(ibuf, IBUFSZ, ibufp);
		int pr = port::get_shell_command();
		long poff = ptr_off(port::ibuf, IBUFSZ, port::ibufp);
		bool ok = (rr == pr && roff == poff && core_equal());
		note_case(st, ok);
	};

	run("echo hi\n");
	run("!\n");
	run("a\\!b\n");
	run(std::string("x\x80\xff\n", 4));
	run(std::string(510, 'L') + "\n");
	for (long i = 0; i < 8000; ++i) {
		std::string s;
		int n = static_cast<int>(rnd() % 24);
		for (int j = 0; j < n; ++j) {
			char c = static_cast<char>(32 + (rnd() % 90));
			if (c == '\n') c = 'n';
			s.push_back(c);
		}
		s.push_back('\n');
		run(s);
		random_iters++;
	}
}

void test_append_lines()
{
	Stat &st = reg("append_lines");
	auto run = [&](const std::vector<std::string> &seed, const std::string &app, long at) {
		reset_both();
		isglobal = 1;
		setup_ibuf(app);
		if (!scratch_both(seed)) {
			note_case(st, false);
			return;
		}
		int rr = ref_append_lines(at);
		int pr = port::append_lines(at);
		note_case(st, rr == pr && core_equal() && sbuf_equal());
		close_scratch_both();
	};

	run({"a\n"}, "x\n.\n", 1);
	run({"a\n"}, ".\n", 1);
	run({"a\n"}, std::string("\0\n.\n", 4), 1);
	run({"a\n"}, "\x80\xff\n.\n", 1);
	run({"a\n"}, std::string(510, 'M') + "\n.\n", 1);
	for (long i = 0; i < 1200; ++i) {
		std::string app;
		int ln = static_cast<int>(rnd() % 3) + 1;
		for (int j = 0; j < ln; ++j)
			app += rand_line(20, true, true);
		app += ".\n";
		run({"x\n", "y\n"}, app, rnd_long(0, 2));
		random_iters++;
	}
}

void test_join_lines()
{
	Stat &st = reg("join_lines");
	auto run = [&](const std::vector<std::string> &lines, long from, long to) {
		reset_both();
		if (!scratch_both(lines)) {
			note_case(st, false);
			return;
		}
		first_addr = from;
		second_addr = to;
		mirror_globals_to_port();
		int rr = ref_join_lines(from, to);
		int pr = port::join_lines(from, to);
		note_case(st, rr == pr && core_equal() && sbuf_equal());
		close_scratch_both();
	};

	run({"a\n", "b\n"}, 1, 2);
	run({std::string("a\0\n", 3), "b\n"}, 1, 2);
	run({"\x80\n", "\xff\n"}, 1, 2);
	run({std::string(260, 'N') + "\n", "x\n"}, 1, 2);
	run({"one\n"}, 1, 1);
	for (long i = 0; i < 1200; ++i) {
		std::vector<std::string> lines;
		int k = static_cast<int>(rnd() % 4) + 2;
		for (int j = 0; j < k; ++j)
			lines.push_back(rand_line(14, true, true));
		long a = rnd_long(1, k - 1);
		long b = rnd_long(a + 1, k);
		run(lines, a, b);
		random_iters++;
	}
}

void test_move_lines()
{
	Stat &st = reg("move_lines");
	auto run = [&](const std::vector<std::string> &lines, long f, long s, long d) {
		reset_both();
		if (!scratch_both(lines)) {
			note_case(st, false);
			return;
		}
		first_addr = f;
		second_addr = s;
		mirror_globals_to_port();
		int rr = ref_move_lines(d);
		int pr = port::move_lines(d);
		note_case(st, rr == pr && core_equal() && sbuf_equal());
		close_scratch_both();
	};

	run({"a\n", "b\n", "c\n"}, 1, 1, 2);
	run({"a\n", "b\n", "c\n"}, 2, 3, 1);
	run({std::string("a\0\n", 3), "b\n", "c\n"}, 1, 2, 3);
	run({"\x80\n", "\xff\n", "c\n"}, 1, 2, 3);
	run({std::string(260, 'O') + "\n", "x\n", "y\n"}, 1, 2, 3);
	for (long i = 0; i < 1200; ++i) {
		std::vector<std::string> lines;
		int k = static_cast<int>(rnd() % 5) + 3;
		for (int j = 0; j < k; ++j)
			lines.push_back(rand_line(12, true, true));
		long f = rnd_long(1, k - 1);
		long s = rnd_long(f, k);
		long d = rnd_long(0, k);
		run(lines, f, s, d);
		random_iters++;
	}
}

void test_copy_lines()
{
	Stat &st = reg("copy_lines");
	auto run = [&](const std::vector<std::string> &lines, long f, long s, long d) {
		reset_both();
		if (!scratch_both(lines)) {
			note_case(st, false);
			return;
		}
		first_addr = f;
		second_addr = s;
		mirror_globals_to_port();
		int rr = ref_copy_lines(d);
		int pr = port::copy_lines(d);
		note_case(st, rr == pr && core_equal() && sbuf_equal());
		close_scratch_both();
	};

	run({"a\n", "b\n"}, 1, 2, 0);
	run({std::string("a\0\n", 3), "b\n"}, 1, 2, 1);
	run({"\x80\n", "\xff\n"}, 1, 2, 2);
	run({std::string(280, 'P') + "\n", "x\n"}, 1, 1, 1);
	run({"one\n"}, 1, 1, 1);
	for (long i = 0; i < 1200; ++i) {
		std::vector<std::string> lines;
		int k = static_cast<int>(rnd() % 4) + 2;
		for (int j = 0; j < k; ++j)
			lines.push_back(rand_line(10, true, true));
		long f = rnd_long(1, k);
		long s = rnd_long(f, k);
		long d = rnd_long(0, k);
		run(lines, f, s, d);
		random_iters++;
	}
}

void test_delete_lines()
{
	Stat &st = reg("delete_lines");
	auto run = [&](const std::vector<std::string> &lines, long f, long t) {
		reset_both();
		if (!scratch_both(lines)) {
			note_case(st, false);
			return;
		}
		int rr = ref_delete_lines(f, t);
		int pr = port::delete_lines(f, t);
		note_case(st, rr == pr && core_equal() && sbuf_equal());
		close_scratch_both();
	};

	run({"a\n", "b\n"}, 1, 1);
	run({std::string("a\0\n", 3), "b\n"}, 1, 2);
	run({"\x80\n", "\xff\n"}, 2, 2);
	run({std::string(260, 'Q') + "\n", "x\n"}, 1, 1);
	run({"one\n"}, 1, 1);
	for (long i = 0; i < 1200; ++i) {
		std::vector<std::string> lines;
		int k = static_cast<int>(rnd() % 4) + 2;
		for (int j = 0; j < k; ++j)
			lines.push_back(rand_line(10, true, true));
		long f = rnd_long(1, k);
		long t = rnd_long(f, k);
		run(lines, f, t);
		random_iters++;
	}
}

void test_display_lines()
{
	Stat &st = reg("display_lines");
	auto run = [&](const std::vector<std::string> &lines, long f, long t, int gflag) {
		reset_both();
		scripted = 1;
		rows = 200;
		mirror_globals_to_port();
		if (!scratch_both(lines)) {
			note_case(st, false);
			return;
		}
		int rr = quiet_stdio([&] { return ref_display_lines(f, t, gflag); });
		int pr = quiet_stdio([&] { return port::display_lines(f, t, gflag); });
		note_case(st, rr == pr && core_equal() && sbuf_equal());
		close_scratch_both();
	};

	run({"a\n", "b\n"}, 1, 2, 0);
	run({std::string("a\0\n", 3), "b\n"}, 1, 2, GLS);
	run({"\x80\n", "\xff\n"}, 1, 2, GNP);
	run({std::string(260, 'R') + "\n", "x\n"}, 1, 2, GLS | GNP);
	run({"one\n"}, 1, 1, 0);
	for (long i = 0; i < 1200; ++i) {
		std::vector<std::string> lines;
		int k = static_cast<int>(rnd() % 4) + 1;
		for (int j = 0; j < k; ++j)
			lines.push_back(rand_line(12, true, true));
		long f = rnd_long(1, k);
		long t = rnd_long(f, k);
		int g = 0;
		if (rnd() & 1) g |= GLS;
		if (rnd() & 1) g |= GNP;
		run(lines, f, t, g);
		random_iters++;
	}
}

void test_mark_line_node()
{
	Stat &st = reg("mark_line_node");
	auto run = [&](int c, long idx) {
		reset_both();
		if (!scratch_both({"a\n", "b\n", "c\n"})) {
			note_case(st, false);
			return;
		}
		int rr = ref_mark_line_node(ref_get_addressed_line_node(idx), c);
		int pr = port::mark_line_node(port::get_addressed_line_node(idx), c);
		note_case(st, rr == pr && core_equal());
		close_scratch_both();
	};

	run('a', 1);
	run('z', 2);
	run('A', 1);
	run('\x80', 1);
	run('m', 3);
	for (long i = 0; i < 7000; ++i) {
		int c = rndb();
		run(c, rnd_long(1, 3));
		random_iters++;
	}
}

void test_get_marked_node_addr()
{
	Stat &st = reg("get_marked_node_addr");
	auto run = [&](int c) {
		reset_both();
		if (!scratch_both({"a\n", "b\n", "c\n"})) {
			note_case(st, false);
			return;
		}
		(void)ref_mark_line_node(ref_get_addressed_line_node(2), 'b');
		(void)port::mark_line_node(port::get_addressed_line_node(2), 'b');
		long rr = ref_get_marked_node_addr(c);
		long pr = port::get_marked_node_addr(c);
		note_case(st, rr == pr && core_equal());
		close_scratch_both();
	};

	run('b');
	run('a');
	run('z');
	run('A');
	run('\x80');
	for (long i = 0; i < 7000; ++i) {
		run(rndb());
		random_iters++;
	}
}

void test_unmark_line_node()
{
	Stat &st = reg("unmark_line_node");
	auto run = [&](int c, long idx) {
		reset_both();
		if (!scratch_both({"a\n", "b\n", "c\n"})) {
			note_case(st, false);
			return;
		}
		(void)ref_mark_line_node(ref_get_addressed_line_node(idx), c);
		(void)port::mark_line_node(port::get_addressed_line_node(idx), c);
		ref_unmark_line_node(ref_get_addressed_line_node(idx));
		port::unmark_line_node(port::get_addressed_line_node(idx));
		long rr = ref_get_marked_node_addr(c);
		long pr = port::get_marked_node_addr(c);
		note_case(st, rr == pr && core_equal());
		close_scratch_both();
	};

	run('a', 1);
	run('m', 2);
	run('z', 3);
	run('A', 1);
	run('\x80', 2);
	for (long i = 0; i < 5000; ++i) {
		run(static_cast<int>('a' + (rnd() % 26)), rnd_long(1, 3));
		random_iters++;
	}
}

void test_dup_line_node()
{
	Stat &st = reg("dup_line_node");
	auto run = [&](const std::string &line) {
		reset_both();
		if (!scratch_both({line})) {
			note_case(st, false);
			return;
		}
		line_t *rd = ref_dup_line_node(ref_get_addressed_line_node(1));
		port::line_t *pd = port::dup_line_node(port::get_addressed_line_node(1));
		bool ok = ((rd == nullptr) == (pd == nullptr) && core_equal());
		if (rd && pd) {
			ok = ok && (rd->len == pd->len) && (rd->seek == pd->seek);
			std::free(rd);
			std::free(pd);
		}
		note_case(st, ok);
		close_scratch_both();
	};

	run("a\n");
	run(std::string("a\0\n", 3));
	run("\x80\xff\n");
	run(std::string(512, 'S') + "\n");
	run("Z\n");
	for (long i = 0; i < 5000; ++i) {
		run(rand_line(30, true, true));
		random_iters++;
	}
}

void test_has_trailing_escape()
{
	Stat &st = reg("has_trailing_escape");
	auto run = [&](const std::string &s) {
		std::string a = s;
		std::string b = s;
		char *as = a.data();
		char *at = a.data() + (a.empty() ? 0 : a.size() - 1);
		char *bs = b.data();
		char *bt = b.data() + (b.empty() ? 0 : b.size() - 1);
		int rr = ref_has_trailing_escape(as, at);
		int pr = port::has_trailing_escape(bs, bt);
		bool ok = (rr == pr && a == b);
		note_case(st, ok);
	};

	run("");
	run("\\");
	run("a\\");
	run(std::string("\x80\\", 2));
	run(std::string(511, '\\'));
	for (long i = 0; i < 12000; ++i) {
		std::string s;
		int n = static_cast<int>(rnd() % 24);
		for (int j = 0; j < n; ++j)
			s.push_back(static_cast<char>(rndb()));
		run(s);
		random_iters++;
	}
}

void test_strip_escapes()
{
	Stat &st = reg("strip_escapes");
	auto run = [&](const std::string &s) {
		std::string a = s;
		std::string b = s;
		char *rr = ref_strip_escapes(a.data());
		char *pr = port::strip_escapes(b.data());
		bool ok = ((rr == nullptr) == (pr == nullptr) &&
		    str_or_empty(rr) == str_or_empty(pr) && a == b && core_equal());
		note_case(st, ok);
	};

	run("");
	run("a");
	run("\\a");
	run(std::string("\x80\\\xff", 3));
	run(std::string(520, '\\'));
	for (long i = 0; i < 12000; ++i) {
		std::string s;
		int n = static_cast<int>(rnd() % 30);
		for (int j = 0; j < n; ++j)
			s.push_back(static_cast<char>(rndb()));
		run(s);
		random_iters++;
	}
}

void test_signal_hup()
{
	Stat &st = reg("signal_hup");
	auto run = [&](int mtx, int signo) {
		reset_both();
		mutex = mtx;
		mirror_globals_to_port();
		oracle_quit_enter();
		ref_signal_hup(signo);
		int rq = oracle_quit_called, rs = oracle_quit_status;
		port::quit_enter();
		port::signal_hup(signo);
		int pq = port::quit_called, ps = port::quit_status;
		note_case(st, rq == pq && rs == ps && core_equal());
	};

	run(0, SIGHUP);
	run(1, SIGHUP);
	run(1, SIGINT);
	run(0, 1);
	run(1, 31);
	for (long i = 0; i < 2500; ++i) {
		run(static_cast<int>(rnd() & 1), static_cast<int>(rnd_long(1, 31)));
		random_iters++;
	}
}

void test_signal_int()
{
	Stat &st = reg("signal_int");
	auto run = [&](int mtx, int signo) {
		reset_both();
		mutex = mtx;
		mirror_globals_to_port();
		oracle_quit_enter();
		ref_signal_int(signo);
		int rq = oracle_quit_called, rs = oracle_quit_status;
		port::quit_enter();
		port::signal_int(signo);
		int pq = port::quit_called, ps = port::quit_status;
		note_case(st, rq == pq && rs == ps && core_equal());
	};

	run(0, SIGINT);
	run(1, SIGINT);
	run(1, SIGHUP);
	run(0, 2);
	run(1, 30);
	for (long i = 0; i < 2500; ++i) {
		run(static_cast<int>(rnd() & 1), static_cast<int>(rnd_long(1, 31)));
		random_iters++;
	}
}

void test_handle_hup()
{
	Stat &st = reg("handle_hup");
	auto run = [&](int signo) {
		reset_both();
		oracle_quit_enter();
		ref_handle_hup(signo);
		int rq = oracle_quit_called, rs = oracle_quit_status;
		port::quit_enter();
		port::handle_hup(signo);
		int pq = port::quit_called, ps = port::quit_status;
		note_case(st, rq == pq && rs == ps && core_equal());
	};

	run(SIGHUP);
	run(SIGINT);
	run(1);
	run(30);
	run(31);
	for (long i = 0; i < 2500; ++i) {
		run(static_cast<int>(rnd_long(1, 31)));
		random_iters++;
	}
}

void test_handle_int()
{
	Stat &st = reg("handle_int");
	auto run = [&](int signo) {
		reset_both();
		oracle_quit_enter();
		ref_handle_int(signo);
		int rq = oracle_quit_called, rs = oracle_quit_status;
		port::quit_enter();
		port::handle_int(signo);
		int pq = port::quit_called, ps = port::quit_status;
		note_case(st, rq == pq && rs == ps && core_equal());
	};

	run(SIGINT);
	run(SIGHUP);
	run(1);
	run(30);
	run(31);
	for (long i = 0; i < 2500; ++i) {
		run(static_cast<int>(rnd_long(1, 31)));
		random_iters++;
	}
}

void test_handle_winch()
{
	Stat &st = reg("handle_winch");
	auto run = [&](int signo) {
		reset_both();
		rows = rnd_long(1, 40);
		cols = static_cast<int>(rnd_long(1, 100));
		mirror_globals_to_port();
		ref_handle_winch(signo);
		port::handle_winch(signo);
		note_case(st, core_equal());
	};

	run(SIGWINCH);
	run(1);
	run(2);
	run(30);
	run(31);
	for (long i = 0; i < 2500; ++i) {
		run(static_cast<int>(rnd_long(1, 31)));
		random_iters++;
	}
}

void test_is_legal_filename()
{
	Stat &st = reg("is_legal_filename");
	auto run = [&](const std::string &s) {
		std::string a = s;
		std::string b = s;
		int rr = ref_is_legal_filename(a.data());
		int pr = port::is_legal_filename(b.data());
		note_case(st, rr == pr && a == b && core_equal());
	};

	run("");
	run("a");
	run("!");
	run("..");
	run("/tmp/x");
	run(std::string("\x80\xff", 2));
	run(std::string(512, 'T'));
	for (long i = 0; i < 9000; ++i) {
		std::string s;
		int n = static_cast<int>(rnd() % 24);
		for (int j = 0; j < n; ++j) {
			char c = static_cast<char>(32 + (rnd() % 90));
			if (c == '\n')
				c = 'n';
			s.push_back(c);
		}
		run(s);
		random_iters++;
	}
}

} /* namespace */

int main()
{
	setvbuf(stdin, nullptr, _IONBF, 0);

	test_extract_subst_tail();
	test_extract_subst_template();
	test_search_and_replace();
	test_substitute_matching_text();
	test_apply_subst_template();
	test_read_file();
	test_read_stream();
	test_get_stream_line();
	test_write_file();
	test_write_stream();
	test_put_stream_line();
	test_get_extended_line();
	test_get_tty_line();
	test_put_tty_line();
	test_extract_addr_range();
	test_next_addr();
	test_exec_command();
	test_check_addr_range();
	test_get_matching_node_addr();
	test_get_filename();
	test_get_shell_command();
	test_append_lines();
	test_join_lines();
	test_move_lines();
	test_copy_lines();
	test_delete_lines();
	test_display_lines();
	test_mark_line_node();
	test_get_marked_node_addr();
	test_unmark_line_node();
	test_dup_line_node();
	test_has_trailing_escape();
	test_strip_escapes();
	test_signal_hup();
	test_signal_int();
	test_handle_hup();
	test_handle_int();
	test_handle_winch();
	test_is_legal_filename();

	std::printf("PBSD batch b0193 differential test\n\n");
	std::printf("%-30s %12s %12s %10s\n", "function", "cases", "failures", "result");
	long tc = 0;
	long tf = 0;
	for (int i = 0; i < nstats; ++i) {
		tc += stats[i].cases;
		tf += stats[i].fails;
		std::printf("%-30s %12ld %12ld %10s\n", stats[i].name, stats[i].cases,
		    stats[i].fails, stats[i].fails ? "FAIL" : "PASS");
	}
	std::printf("%-30s %12ld %12ld %10s\n", "RANDOM_ITERS", random_iters, 0L,
	    random_iters >= RANDOM_TARGET ? "PASS" : "FAIL");
	std::printf("%-30s %12ld %12ld %10s\n", "TOTAL", tc, tf + (random_iters >= RANDOM_TARGET ? 0 : 1),
	    (tf == 0 && random_iters >= RANDOM_TARGET) ? "PASS" : "FAIL");
	return (tf == 0 && random_iters >= RANDOM_TARGET) ? 0 : 1;
}

