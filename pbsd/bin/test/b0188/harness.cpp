/*
 * harness.cpp -- differential test for PBSD batch b0188 (test.c).
 */

#define _GNU_SOURCE

#include <cstdarg>
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <string>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

import pbsd.bin.test.b0188;

namespace P = pbsd::bin_test::b0188;

extern "C" {
struct t_op {
	char op_text[2];
	short op_num;
};

extern const struct t_op ref_ops1[];
extern const struct t_op ref_opsm1[];
extern const struct t_op ref_ops2[];
extern const struct t_op ref_opsm2[];
extern const struct t_op *const ref_ops1_end;
extern const struct t_op *const ref_opsm1_end;
extern const struct t_op *const ref_ops2_end;
extern const struct t_op *const ref_opsm2_end;

extern int ref_nargc;
extern char **ref_t_wp;
extern int ref_parenlevel;

int ref_main(int argc, char **argv);
void ref_error(const char *msg, ...);
void ref_syntax(const char *op, const char *msg);
int ref_find_op_1char(const struct t_op *op, const struct t_op *end,
    const char *s);
int ref_find_op_2char(const struct t_op *op, const struct t_op *end,
    const char *s);
int ref_find_op(const char *s);
int ref_isunopoperand(void);
int ref_islparenoperand(void);
int ref_isrparenoperand(void);
int ref_t_lex(char *s);
int ref_getn(const char *s);
intmax_t ref_getq(const char *s);
int ref_intcmp(const char *s1, const char *s2);
int ref_oexpr(int n);
int ref_aexpr(int n);
int ref_nexpr(int n);
int ref_primary(int n);
int ref_binop(int n);
int ref_filstat(char *nm, int mode);
int ref_newerf(const char *f1, const char *f2);
int ref_olderf(const char *f1, const char *f2);
int ref_equalf(const char *f1, const char *f2);
}

namespace {

constexpr long SWEEP = 200000L;
constexpr int MAX_SHOW = 8;

struct Stat {
	const char *name;
	long cases;
	long fails;
	int shown;
};

struct Rng {
	std::uint64_t s;

	explicit Rng(std::uint64_t seed) : s(seed) {}

	std::uint64_t next()
	{
		s += 0x9E3779B97F4A7C15ull;
		std::uint64_t z = s;
		z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
		z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
		return z ^ (z >> 31);
	}

	int bits(int lo, int hi)
	{
		if (hi <= lo)
			return lo;
		return lo + (int)(next() % (std::uint64_t)(hi - lo + 1));
	}

	bool coin()
	{
		return (next() & 1u) != 0;
	}

	unsigned char byte()
	{
		return (unsigned char)(next() & 0xffu);
	}
};

Rng rng(0xb0188faceULL);

Stat st_find_op_1char = { "find_op_1char", 0, 0, 0 };
Stat st_find_op_2char = { "find_op_2char", 0, 0, 0 };
Stat st_find_op = { "find_op", 0, 0, 0 };
Stat st_isunopoperand = { "isunopoperand", 0, 0, 0 };
Stat st_islparenoperand = { "islparenoperand", 0, 0, 0 };
Stat st_isrparenoperand = { "isrparenoperand", 0, 0, 0 };
Stat st_t_lex = { "t_lex", 0, 0, 0 };
Stat st_getn = { "getn", 0, 0, 0 };
Stat st_getq = { "getq", 0, 0, 0 };
Stat st_intcmp = { "intcmp", 0, 0, 0 };
Stat st_oexpr = { "oexpr", 0, 0, 0 };
Stat st_aexpr = { "aexpr", 0, 0, 0 };
Stat st_nexpr = { "nexpr", 0, 0, 0 };
Stat st_primary = { "primary", 0, 0, 0 };
Stat st_binop = { "binop", 0, 0, 0 };
Stat st_filstat = { "filstat", 0, 0, 0 };
Stat st_newerf = { "newerf", 0, 0, 0 };
Stat st_olderf = { "olderf", 0, 0, 0 };
Stat st_equalf = { "equalf", 0, 0, 0 };
Stat st_syntax = { "syntax", 0, 0, 0 };
Stat st_error = { "error", 0, 0, 0 };
Stat st_main = { "main", 0, 0, 0 };

std::string g_tmpdir;

void
fail(Stat &st, const char *fmt, ...)
{
	st.fails++;
	if (st.shown < MAX_SHOW) {
		va_list ap;
		va_start(ap, fmt);
		std::fprintf(stderr, "%s: ", st.name);
		std::vfprintf(stderr, fmt, ap);
		std::fputc('\n', stderr);
		va_end(ap);
		st.shown++;
	}
}

struct ArgvDup {
	std::vector<std::string> storage;
	std::vector<char *> ptrs;

	void
	build(const std::vector<std::string> &args)
	{
		storage = args;
		ptrs.clear();
		ptrs.reserve(storage.size());
		for (auto &s : storage)
			ptrs.push_back(s.data());
	}

	char **
	base()
	{
		return ptrs.empty() ? nullptr : ptrs.data();
	}
};

struct ParserSnap {
	int nargc;
	ptrdiff_t wp_off;
	int parenlevel;
};

void
set_ref_state(ArgvDup &av, int nargc, ptrdiff_t wp_off, int paren)
{
	ref_nargc = nargc;
	ref_t_wp = av.base() + wp_off;
	ref_parenlevel = paren;
}

void
set_port_state(ArgvDup &av, int nargc, ptrdiff_t wp_off, int paren)
{
	P::nargc = nargc;
	P::t_wp = av.base() + wp_off;
	P::parenlevel = paren;
}

void
setup_parser_both(ArgvDup &ref_av, ArgvDup &port_av, int nargc,
    ptrdiff_t wp_off, int paren)
{
	set_ref_state(ref_av, nargc, wp_off, paren);
	set_port_state(port_av, nargc, wp_off, paren);
}

bool
snap_eq(const ParserSnap &a, const ParserSnap &b)
{
	return a.nargc == b.nargc && a.wp_off == b.wp_off &&
	    a.parenlevel == b.parenlevel;
}

std::string
rand_string(int minlen, int maxlen)
{
	int len = rng.bits(minlen, maxlen);
	std::string s;
	s.resize((size_t)len);
	for (int i = 0; i < len; i++) {
		unsigned char c = rng.byte();
		if (rng.coin() && i + 1 < len)
			c = '\0';
		s[(size_t)i] = (char)c;
	}
	if (len > 0 && rng.coin())
		s[(size_t)(len - 1)] = '\0';
	return s;
}

template <typename Fn>
int
fork_fn(Fn fn)
{
	fflush(nullptr);
	pid_t pid = fork();
	if (pid == 0) {
		fn();
		::_exit(99);
	}
	int status = 0;
	if (waitpid(pid, &status, 0) < 0)
		return -1;
	if (WIFEXITED(status))
		return WEXITSTATUS(status);
	return -1;
}

template <typename RefFn, typename PortFn>
void
cmp_int(Stat &st, RefFn ref_fn, PortFn port_fn)
{
	st.cases++;
	int rr = ref_fn();
	int rp = port_fn();
	if (rr != rp)
		fail(st, "mismatch ref=%d port=%d", rr, rp);
}

void
test_find_op_tables()
{
	static const char *one_ops[] = { "", "=", "<", ">", "!", "(", ")", "\x80" };
	for (const char *s : one_ops) {
		int rr = ref_find_op_1char(ref_ops1, ref_ops1_end, s);
		int rp = P::find_op_1char(P::ops1, P::ops1_end(), s);
		cmp_int(st_find_op_1char, [&] { return rr; }, [&] { return rp; });
	}

	static const char *two_ops[] = { "==", "!=", "xy", "\xff\xfe", "=\x00" };
	for (const char *s : two_ops) {
		int rr = ref_find_op_2char(ref_ops2, ref_ops2_end, s);
		int rp = P::find_op_2char(P::ops2, P::ops2_end(), s);
		cmp_int(st_find_op_2char, [&] { return rr; }, [&] { return rp; });
	}
}

void
test_find_op_edge()
{
	static const char *fixed[] = {
		"", "=", "<", ">", "!", "(", ")", "-r", "-z", "-eq", "-nt",
		"==", "!=", "-", "abc", "----", "-\x80", "\x80", "\xff",
		"-eqx", "-e", "-ef", "-ot", "-ne", "-ge", "-le", "-gt", "-lt",
		"-h", "-L", "-O", "-G", "-S", "-a", "-o", "-k", "-u", "-g",
		"-s", "-t", "-n", "-b", "-c", "-p", "-d", "-f", "-w", "-x",
	};
	for (const char *s : fixed) {
		cmp_int(st_find_op, [&] { return ref_find_op(s); },
		    [&] { return P::find_op(s); });
	}
}

void
test_find_op_sweep()
{
	for (long i = 0; i < SWEEP; i++) {
		std::string s = rand_string(0, 6);
		cmp_int(st_find_op, [&] { return ref_find_op(s.c_str()); },
		    [&] { return P::find_op(s.c_str()); });
	}
}

void
test_operand_predicates_edge()
{
	struct Case {
		std::vector<std::string> args;
		int nargc;
		ptrdiff_t wp_off;
		int paren;
	};

	Case cases[] = {
		{ { "a" }, 1, 0, 0 },
		{ { "a", "=" }, 2, 0, 0 },
		{ { "a", "=", "b" }, 3, 0, 0 },
		{ { "a", ")" }, 2, 0, 1 },
		{ { "(", "a", ")" }, 3, 0, 1 },
		{ { "x", "-eq", "y" }, 3, 0, 0 },
		{ { ")", "a" }, 2, 0, 1 },
		{ { "a", "b", "=" }, 3, 0, 0 },
		{ { "\x80", "=" }, 2, 0, 0 },
	};

	for (const auto &c : cases) {
		ArgvDup ref_av, port_av;
		ref_av.build(c.args);
		port_av.build(c.args);
		setup_parser_both(ref_av, port_av, c.nargc, c.wp_off, c.paren);

		cmp_int(st_isunopoperand, [&] { return ref_isunopoperand(); },
		    [&] { return P::isunopoperand(); });

		setup_parser_both(ref_av, port_av, c.nargc, c.wp_off, c.paren);
		cmp_int(st_islparenoperand, [&] { return ref_islparenoperand(); },
		    [&] { return P::islparenoperand(); });

		setup_parser_both(ref_av, port_av, c.nargc, c.wp_off, c.paren);
		cmp_int(st_isrparenoperand, [&] { return ref_isrparenoperand(); },
		    [&] { return P::isrparenoperand(); });
	}
}

void
test_operand_predicates_sweep()
{
	static const char *pool[] = { "a", "b", "=", "!", "(", ")", "-eq", "-nt",
	    "\x80", "" };
	for (long i = 0; i < SWEEP; i++) {
		int n = rng.bits(1, 4);
		std::vector<std::string> args;
		for (int j = 0; j < n; j++)
			args.push_back(pool[rng.bits(0, 9)]);
		ArgvDup ref_av, port_av;
		ref_av.build(args);
		port_av.build(args);
		int paren = rng.bits(0, 2);
		setup_parser_both(ref_av, port_av, n, 0, paren);

		cmp_int(st_isunopoperand, [&] { return ref_isunopoperand(); },
		    [&] { return P::isunopoperand(); });

		setup_parser_both(ref_av, port_av, n, 0, paren);
		cmp_int(st_islparenoperand, [&] { return ref_islparenoperand(); },
		    [&] { return P::islparenoperand(); });

		setup_parser_both(ref_av, port_av, n, 0, paren);
		cmp_int(st_isrparenoperand, [&] { return ref_isrparenoperand(); },
		    [&] { return P::isrparenoperand(); });
	}
}

void
test_t_lex_edge()
{
	struct Case {
		std::vector<std::string> args;
		int nargc;
		ptrdiff_t wp_off;
		int paren;
		const char *lex;
	};

	Case cases[] = {
		{ { "a" }, 1, 0, 0, "a" },
		{ { "a", "-eq", "b" }, 3, 0, 0, "a" },
		{ { "a", "-eq", "b" }, 3, 0, 0, "-eq" },
		{ { "(", "a", ")" }, 3, 0, 1, "(" },
		{ { "(", "a", ")" }, 3, 0, 1, ")" },
		{ { "!", "a" }, 2, 0, 0, "!" },
		{ { "x", ")" }, 2, 0, 1, ")" },
		{ { "x", ")" }, 2, 0, 1, "x" },
	};

	for (const auto &c : cases) {
		ArgvDup ref_av, port_av;
		ref_av.build(c.args);
		port_av.build(c.args);
		setup_parser_both(ref_av, port_av, c.nargc, c.wp_off, c.paren);
		size_t lex_idx = 0;
		for (size_t j = 0; j < c.args.size(); j++) {
			if (c.args[j] == c.lex) {
				lex_idx = j;
				break;
			}
		}
		char *ls = ref_av.ptrs[lex_idx];
		char *lp = port_av.ptrs[lex_idx];
		st_t_lex.cases++;
		int rr = ref_t_lex(ls);
		int rp = (int)P::t_lex(lp);
		if (rr != rp)
			fail(st_t_lex, "token mismatch ref=%d port=%d lex=%s", rr,
			    rp, c.lex);
	}

	st_t_lex.cases++;
	int rr = ref_t_lex(nullptr);
	int rp = (int)P::t_lex(nullptr);
	if (rr != rp)
		fail(st_t_lex, "NULL mismatch ref=%d port=%d", rr, rp);
}

void
test_t_lex_sweep()
{
	static const char *pool[] = { "a", "=", "!", "(", ")", "-eq", "-z", "-nt",
	    "\x80", "" };
	for (long i = 0; i < SWEEP; i++) {
		int n = rng.bits(1, 5);
		std::vector<std::string> args;
		for (int j = 0; j < n; j++)
			args.push_back(pool[rng.bits(0, 9)]);
		ArgvDup ref_av, port_av;
		ref_av.build(args);
		port_av.build(args);
		int paren = rng.bits(0, 2);
		int idx = rng.bits(0, n - 1);
		setup_parser_both(ref_av, port_av, n, 0, paren);
		st_t_lex.cases++;
		int rr = ref_t_lex(ref_av.ptrs[(size_t)idx]);
		int rp = (int)P::t_lex(port_av.ptrs[(size_t)idx]);
		if (rr != rp)
			fail(st_t_lex, "sweep mismatch ref=%d port=%d", rr, rp);
	}
}

void
test_getn_valid(const char *s)
{
	st_getn.cases++;
	int rr = ref_getn(s);
	int rp = P::getn(s);
	if (rr != rp)
		fail(st_getn, "valid %s ref=%d port=%d", s, rr, rp);
}

void
test_getn_error(const char *s)
{
	st_getn.cases++;
	int er = fork_fn([&] { (void)ref_getn(s); });
	int ep = fork_fn([&] { (void)P::getn(s); });
	if (er != ep)
		fail(st_getn, "error %s ref_exit=%d port_exit=%d", s, er, ep);
}

void
test_getn_edge()
{
	static const char *valid[] = { "0", "1", "-1", "42", " 7", "8 ",
	    "9223372036854775807", "-9223372036854775808" };
	for (const char *s : valid)
		test_getn_valid(s);

	static const char *invalid[] = { "", "x", "1x", "1.0", "++1", " 1a",
	    "9223372036854775808" };
	for (const char *s : invalid)
		test_getn_error(s);
}

void
test_getn_sweep()
{
	for (long i = 0; i < SWEEP; i++) {
		if (rng.coin()) {
			char buf[32];
			int v = rng.bits(-100000, 100000);
			std::snprintf(buf, sizeof(buf), "%d", v);
			test_getn_valid(buf);
		} else {
			std::string s = rand_string(0, 8);
			if (s.empty() || s[0] < '0' || s[0] > '9' || rng.coin())
				test_getn_error(s.c_str());
			else
				test_getn_valid(s.c_str());
		}
	}
}

void
test_getq_valid(const char *s)
{
	st_getq.cases++;
	intmax_t rr = ref_getq(s);
	intmax_t rp = P::getq(s);
	if (rr != rp)
		fail(st_getq, "valid %s ref=%jd port=%jd", s, (intmax_t)rr,
		    (intmax_t)rp);
}

void
test_getq_error(const char *s)
{
	st_getq.cases++;
	int er = fork_fn([&] { (void)ref_getq(s); });
	int ep = fork_fn([&] { (void)P::getq(s); });
	if (er != ep)
		fail(st_getq, "error %s ref_exit=%d port_exit=%d", s, er, ep);
}

void
test_getq_edge()
{
	static const char *valid[] = { "0", "-0", "1", "-1", "999",
	    "9223372036854775807", "-9223372036854775808" };
	for (const char *s : valid)
		test_getq_valid(s);

	static const char *invalid[] = { "", "bad", "1e3", "1 2" };
	for (const char *s : invalid)
		test_getq_error(s);
}

void
test_getq_sweep()
{
	for (long i = 0; i < SWEEP; i++) {
		if (rng.bits(0, 3) != 0) {
			char buf[48];
			long long v = (long long)rng.bits(-1000000, 1000000);
			std::snprintf(buf, sizeof(buf), "%lld", v);
			test_getq_valid(buf);
		} else {
			std::string s = rand_string(0, 10);
			test_getq_error(s.c_str());
		}
	}
}

void
test_intcmp_edge()
{
	static const char *pairs[][2] = {
		{ "0", "0" }, { "1", "2" }, { "2", "1" }, { "-1", "1" },
		{ "9223372036854775807", "9223372036854775806" },
		{ "-9223372036854775808", "-9223372036854775807" },
	};
	for (auto &p : pairs) {
		cmp_int(st_intcmp, [&] { return ref_intcmp(p[0], p[1]); },
		    [&] { return P::intcmp(p[0], p[1]); });
	}
}

void
test_intcmp_sweep()
{
	for (long i = 0; i < SWEEP; i++) {
		char a[32], b[32];
		int va = rng.bits(-100000, 100000);
		int vb = rng.bits(-100000, 100000);
		std::snprintf(a, sizeof(a), "%d", va);
		std::snprintf(b, sizeof(b), "%d", vb);
		cmp_int(st_intcmp, [&] { return ref_intcmp(a, b); },
		    [&] { return P::intcmp(a, b); });
	}
}

void
run_parser_fn(Stat &st, int token, int (*ref_fn)(int),
    int (*port_fn)(P::token))
{
	st.cases++;
	ArgvDup ref_av, port_av;
	std::vector<std::string> args = { "1", "-eq", "1" };
	ref_av.build(args);
	port_av.build(args);
	setup_parser_both(ref_av, port_av, 3, 0, 0);
	int rr = ref_fn(token);
	ParserSnap sr = { ref_nargc, ref_t_wp - ref_av.base(), ref_parenlevel };
	int rp = port_fn((P::token)token);
	ParserSnap sp = { P::nargc, P::t_wp - port_av.base(), P::parenlevel };
	if (rr != rp || !snap_eq(sr, sp))
		fail(st, "token=%d res %d/%d snap mismatch", token, rr, rp);
}

void
test_parser_units()
{
	run_parser_fn(st_nexpr, P::UNOT, ref_nexpr,
	    [](P::token t) { return P::nexpr(t); });
	run_parser_fn(st_primary, P::OPERAND, ref_primary,
	    [](P::token t) { return P::primary(t); });
	run_parser_fn(st_binop, P::INTEQ, ref_binop,
	    [](P::token t) { return P::binop(t); });
	run_parser_fn(st_aexpr, P::OPERAND, ref_aexpr,
	    [](P::token t) { return P::aexpr(t); });
	run_parser_fn(st_oexpr, P::OPERAND, ref_oexpr,
	    [](P::token t) { return P::oexpr(t); });
}

struct ExprCase {
	std::vector<const char *> argv;
	int expect;
};

void
test_main_case(const ExprCase &ec)
{
	st_main.cases++;
	std::vector<std::string> storage;
	std::vector<char *> av;
	storage.push_back("test");
	for (const char *s : ec.argv)
		storage.emplace_back(s);
	for (auto &s : storage)
		av.push_back(s.data());
	int argc = (int)av.size();
	int er = fork_fn([&] { ::_exit(ref_main(argc, av.data())); });
	int ep = fork_fn([&] { ::_exit(P::main(argc, av.data())); });
	if (er != ep || er != ec.expect)
		fail(st_main, "argv exit ref=%d port=%d expect=%d", er, ep,
		    ec.expect);
}

void
test_main_edge()
{
	test_main_case({ {}, 1 });
	test_main_case({ { "" }, 1 });
	test_main_case({ { "x" }, 0 });
	test_main_case({ { "a", "=", "a" }, 0 });
	test_main_case({ { "a", "!=", "b" }, 0 });
	test_main_case({ { "1", "-eq", "1" }, 0 });
	test_main_case({ { "1", "-ne", "2" }, 0 });
	test_main_case({ { "0", "-eq", "1" }, 1 });
	test_main_case({ { "!", "", "-o", "x" }, 0 });
	test_main_case({ { "-z", "" }, 0 });
	test_main_case({ { "-n", "x" }, 0 });
	test_main_case({ { "(", "0", "=", "1", ")" }, 1 });
	test_main_case({ { "a", "-o", "b" }, 0 });
	test_main_case({ { "", "-a", "x" }, 1 });
}

void
test_main_bracket()
{
	st_main.cases++;
	std::vector<std::string> storage = { "[", "1", "-eq", "1", "]" };
	std::vector<char *> av;
	for (auto &s : storage)
		av.push_back(s.data());
	int er = fork_fn([&] { ::_exit(ref_main((int)av.size(), av.data())); });
	int ep = fork_fn([&] { ::_exit(P::main((int)av.size(), av.data())); });
	if (er != ep || er != 0)
		fail(st_main, "bracket ref=%d port=%d", er, ep);
}

void
test_main_sweep()
{
	static const char *atoms[] = { "", "x", "0", "1", "a", "\x80" };
	for (long i = 0; i < SWEEP; i++) {
		ExprCase ec;
		int kind = rng.bits(0, 5);
		if (kind == 0)
			ec = { { atoms[rng.bits(0, 5)] }, rng.coin() ? 0 : 1 };
		else if (kind == 1)
			ec = { { "1", "-eq", atoms[rng.bits(2, 4)] }, 1 };
		else if (kind == 2)
			ec = { { "1", "-eq", "1" }, 0 };
		else if (kind == 3)
			ec = { { atoms[rng.bits(0, 5)], "-nt", atoms[rng.bits(0, 5)] },
			    1 };
		else if (kind == 4)
			ec = { { "-z", atoms[rng.bits(0, 3)] },
			    atoms[rng.bits(0, 3)][0] == '\0' ? 0 : 1 };
		else
			ec = { { "!", atoms[rng.bits(0, 5)] },
			    atoms[rng.bits(0, 5)][0] == '\0' ? 0 : 1 };
		test_main_case(ec);
	}
}

void
test_syntax_error()
{
	st_syntax.cases++;
	int er = fork_fn([] { ref_syntax("op", "msg"); });
	int ep = fork_fn([] { P::syntax("op", "msg"); });
	if (er != ep)
		fail(st_syntax, "syntax exit ref=%d port=%d", er, ep);

	st_error.cases++;
	er = fork_fn([] { ref_error("boom"); });
	ep = fork_fn([] { P::error("boom"); });
	if (er != ep)
		fail(st_error, "error exit ref=%d port=%d", er, ep);
}

std::string
path_join(const std::string &name)
{
	return g_tmpdir + "/" + name;
}

void
mkfile(const std::string &p, const char *data, mode_t mode)
{
	int fd = open(p.c_str(), O_CREAT | O_TRUNC | O_WRONLY, mode);
	if (fd >= 0) {
		if (data)
			(void)write(fd, data, strlen(data));
		close(fd);
	}
}

void
test_filstat_edge()
{
	std::string f = path_join("file");
	std::string d = path_join("dir");
	std::string l = path_join("link");
	std::string e = path_join("empty");
	std::string nz = path_join("nz");
	mkfile(f, "data", 0644);
	mkdir(d.c_str(), 0755);
	mkfile(e, "", 0644);
	mkfile(nz, "x", 0644);
	(void)symlink(f.c_str(), l.c_str());

	struct ModeCase {
		int tok;
		const char *path;
	};
	ModeCase cases[] = {
		{ P::FILRD, f.c_str() }, { P::FILWR, f.c_str() },
		{ P::FILEX, d.c_str() }, { P::FILEXIST, f.c_str() },
		{ P::FILREG, f.c_str() }, { P::FILDIR, d.c_str() },
		{ P::FILSYM, l.c_str() }, { P::FILGZ, nz.c_str() },
		{ P::FILGZ, e.c_str() }, { P::FILRD, path_join("missing").c_str() },
	};

	for (auto &c : cases) {
		st_filstat.cases++;
		char rpbuf[512];
		std::strncpy(rpbuf, c.path, sizeof(rpbuf) - 1);
		rpbuf[sizeof(rpbuf) - 1] = '\0';
		char ppbuf[512];
		std::strncpy(ppbuf, c.path, sizeof(ppbuf) - 1);
		ppbuf[sizeof(ppbuf) - 1] = '\0';
		int rr = ref_filstat(rpbuf, c.tok);
		int rpp = P::filstat(ppbuf, (P::token)c.tok);
		if (rr != rpp)
			fail(st_filstat, "mode=%d path=%s ref=%d port=%d", c.tok,
			    c.path, rr, rpp);
	}
}

void
test_filstat_sweep()
{
	std::string f = path_join("sweepfile");
	mkfile(f, "z", 0644);
	for (long i = 0; i < SWEEP; i++) {
		int tok = P::FILRD + rng.bits(0, 20);
		if (tok == P::STREZ || tok == P::STRNZ || tok == P::FILTT)
			continue;
		st_filstat.cases++;
		char rb[256], pb[256];
		const char *path = rng.coin() ? f.c_str() :
		    path_join("missing").c_str();
		std::strncpy(rb, path, sizeof(rb) - 1);
		std::strncpy(pb, path, sizeof(pb) - 1);
		int rr = ref_filstat(rb, tok);
		int rp = P::filstat(pb, (P::token)tok);
		if (rr != rp)
			fail(st_filstat, "sweep tok=%d ref=%d port=%d", tok, rr,
			    rp);
	}
}

void
test_newerf_olderf_equalf()
{
	std::string a = path_join("old");
	std::string b = path_join("new");
	std::string hl = path_join("hardlink");
	mkfile(a, "a", 0644);
	usleep(10000);
	mkfile(b, "bb", 0644);
	(void)link(a.c_str(), hl.c_str());

	cmp_int(st_newerf, [&] { return ref_newerf(b.c_str(), a.c_str()); },
	    [&] { return P::newerf(b.c_str(), a.c_str()); });
	cmp_int(st_newerf, [&] { return ref_newerf(a.c_str(), b.c_str()); },
	    [&] { return P::newerf(a.c_str(), b.c_str()); });
	cmp_int(st_olderf, [&] { return ref_olderf(a.c_str(), b.c_str()); },
	    [&] { return P::olderf(a.c_str(), b.c_str()); });
	cmp_int(st_equalf, [&] { return ref_equalf(a.c_str(), hl.c_str()); },
	    [&] { return P::equalf(a.c_str(), hl.c_str()); });
	cmp_int(st_equalf,
	    [&] { return ref_equalf(a.c_str(), b.c_str()); },
	    [&] { return P::equalf(a.c_str(), b.c_str()); });
	cmp_int(st_newerf,
	    [&] { return ref_newerf("missing", a.c_str()); },
	    [&] { return P::newerf("missing", a.c_str()); });
}

void
test_newerf_sweep()
{
	std::string a = path_join("sweep_a");
	std::string b = path_join("sweep_b");
	for (long i = 0; i < SWEEP; i++) {
		mkfile(a, "a", 0644);
		if (rng.coin())
			usleep(1000);
		mkfile(b, "bb", 0644);
		cmp_int(st_newerf, [&] { return ref_newerf(a.c_str(), b.c_str()); },
		    [&] { return P::newerf(a.c_str(), b.c_str()); });
		cmp_int(st_olderf, [&] { return ref_olderf(a.c_str(), b.c_str()); },
		    [&] { return P::olderf(a.c_str(), b.c_str()); });
	}
}

void
test_parser_expr_sweep()
{
	static const char *ops[] = { "-eq", "-ne", "-lt", "-gt", "=", "!=" };
	for (long i = 0; i < SWEEP; i++) {
		char a[16], b[16];
		int va = rng.bits(-50, 50);
		int vb = rng.bits(-50, 50);
		std::snprintf(a, sizeof(a), "%d", va);
		std::snprintf(b, sizeof(b), "%d", vb);
		const char *op = ops[rng.bits(0, 5)];
		ExprCase ec = { { a, op, b }, 0 };
		if (std::strcmp(op, "-eq") == 0 || std::strcmp(op, "=") == 0)
			ec.expect = (va == vb) ? 0 : 1;
		else if (std::strcmp(op, "-ne") == 0 || std::strcmp(op, "!=") == 0)
			ec.expect = (va != vb) ? 0 : 1;
		else if (std::strcmp(op, "-lt") == 0)
			ec.expect = (va < vb) ? 0 : 1;
		else if (std::strcmp(op, "-gt") == 0)
			ec.expect = (va > vb) ? 0 : 1;
		test_main_case(ec);
	}
}

long
print_stat(const Stat &st)
{
	std::printf("%-16s %8ld %8ld\n", st.name, st.cases, st.fails);
	return st.fails;
}

} // namespace

int
main()
{
	char tmpl[] = "/tmp/pbsd_b0188_XXXXXX";
	char *td = mkdtemp(tmpl);
	if (td == nullptr) {
		std::perror("mkdtemp");
		return 1;
	}
	g_tmpdir = td;

	test_find_op_tables();
	test_find_op_edge();
	test_find_op_sweep();
	test_operand_predicates_edge();
	test_operand_predicates_sweep();
	test_t_lex_edge();
	test_t_lex_sweep();
	test_getn_edge();
	test_getn_sweep();
	test_getq_edge();
	test_getq_sweep();
	test_intcmp_edge();
	test_intcmp_sweep();
	test_parser_units();
	test_syntax_error();
	test_filstat_edge();
	test_filstat_sweep();
	test_newerf_olderf_equalf();
	test_newerf_sweep();
	test_main_edge();
	test_main_bracket();
	test_main_sweep();
	test_parser_expr_sweep();

	std::printf("\n%-16s %8s %8s\n", "function", "cases", "failures");
	long total_fails = 0;
	Stat *all[] = {
		&st_find_op_1char, &st_find_op_2char, &st_find_op,
		&st_isunopoperand, &st_islparenoperand, &st_isrparenoperand,
		&st_t_lex, &st_getn, &st_getq, &st_intcmp, &st_oexpr,
		&st_aexpr, &st_nexpr, &st_primary, &st_binop, &st_filstat,
		&st_newerf, &st_olderf, &st_equalf, &st_syntax, &st_error,
		&st_main,
	};
	for (Stat *st : all)
		total_fails += print_stat(*st);

	return total_fails == 0 ? 0 : 1;
}
