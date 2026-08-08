/*
 * harness.cpp -- differential test for PBSD batch b0167 (cksum.c).
 *
 * cksum.c defines only main(), which reads hex values from stdin, stores them
 * in buf, and prints checksums.  Each case redirects stdin, captures stdout,
 * and compares ref_main against port::main.  At least four inputs are required
 * per case because fewer values leave q != p forever in the original.
 */

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <unistd.h>
#include <vector>

import pbsd.usr.sbin.route6d.misc.b0167;

namespace P = pbsd::usr_sbin_route6d_misc::b0167;

extern "C" {
extern unsigned short buf[];
int ref_main(void);
}

#define SWEEP 200000L
#define MAX_SHOW 8

struct Stat {
	const char *name;
	long cases;
	long fails;
	int shown;
};

static Stat st_main = { "main", 0, 0, 0 };

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

	int next_u16()
	{
		return static_cast<int>(next() & 0xffffu);
	}

	int count_4_64()
	{
		return 4 + static_cast<int>(next() % 61u);
	}
};

static Rng rng(0x00b0167faceULL);

struct IoResult {
	int ret;
	std::string out;
};

static bool
fail(Stat &st, const char *what)
{
	st.fails++;
	if (st.shown < MAX_SHOW) {
		st.shown++;
		std::printf("  FAIL %s: %s\n", st.name, what);
	}
	return false;
}

static void
poison_buf(unsigned short *b, std::size_t nshorts)
{
	std::memset(b, 0x7f, nshorts * sizeof(unsigned short));
}

static IoResult
run_ref(const char *input)
{
	IoResult r;
	int outpipe[2];
	int oldin, oldout;
	FILE *inmem;
	std::size_t inlen;
	char bufout[65536];
	ssize_t nread;

	poison_buf(buf, BUFSIZ);

	inlen = std::strlen(input);
	inmem = fmemopen(const_cast<char *>(input), inlen, "r");
	if (inmem == nullptr)
		std::abort();

	if (pipe(outpipe) != 0)
		std::abort();

	oldin = dup(STDIN_FILENO);
	oldout = dup(STDOUT_FILENO);
	if (oldin < 0 || oldout < 0)
		std::abort();

	dup2(fileno(inmem), STDIN_FILENO);
	dup2(outpipe[1], STDOUT_FILENO);
	std::fflush(stdout);

	r.ret = ref_main();

	std::fflush(stdout);
	dup2(oldin, STDIN_FILENO);
	dup2(oldout, STDOUT_FILENO);
	close(oldin);
	close(oldout);
	close(outpipe[1]);
	fclose(inmem);

	nread = 0;
	while (true) {
		ssize_t chunk = read(outpipe[0], bufout + nread,
		    sizeof(bufout) - 1 - static_cast<std::size_t>(nread));
		if (chunk <= 0)
			break;
		nread += chunk;
		if (static_cast<std::size_t>(nread) >= sizeof(bufout) - 1)
			break;
	}
	close(outpipe[0]);

	bufout[nread] = '\0';
	r.out.assign(bufout, static_cast<std::size_t>(nread));
	return r;
}

static IoResult
run_port(const char *input)
{
	IoResult r;
	int outpipe[2];
	int oldin, oldout;
	FILE *inmem;
	std::size_t inlen;
	char bufout[65536];
	ssize_t nread;

	poison_buf(P::buf, BUFSIZ);

	inlen = std::strlen(input);
	inmem = fmemopen(const_cast<char *>(input), inlen, "r");
	if (inmem == nullptr)
		std::abort();

	if (pipe(outpipe) != 0)
		std::abort();

	oldin = dup(STDIN_FILENO);
	oldout = dup(STDOUT_FILENO);
	if (oldin < 0 || oldout < 0)
		std::abort();

	dup2(fileno(inmem), STDIN_FILENO);
	dup2(outpipe[1], STDOUT_FILENO);
	std::fflush(stdout);

	r.ret = P::main();

	std::fflush(stdout);
	dup2(oldin, STDIN_FILENO);
	dup2(oldout, STDOUT_FILENO);
	close(oldin);
	close(oldout);
	close(outpipe[1]);
	fclose(inmem);

	nread = 0;
	while (true) {
		ssize_t chunk = read(outpipe[0], bufout + nread,
		    sizeof(bufout) - 1 - static_cast<std::size_t>(nread));
		if (chunk <= 0)
			break;
		nread += chunk;
		if (static_cast<std::size_t>(nread) >= sizeof(bufout) - 1)
			break;
	}
	close(outpipe[0]);

	bufout[nread] = '\0';
	r.out.assign(bufout, static_cast<std::size_t>(nread));
	return r;
}

static std::string
format_input(const std::vector<int> &vals)
{
	std::string s;
	char tmp[32];

	for (std::size_t i = 0; i < vals.size(); i++) {
		std::snprintf(tmp, sizeof(tmp), "%x\n", vals[i]);
		s += tmp;
	}
	return s;
}

static bool
run_case(const char *label, const char *input)
{
	st_main.cases++;

	IoResult rr = run_ref(input);
	IoResult rp = run_port(input);

	if (rr.ret != rp.ret) {
		std::printf("    %s: ret %d vs %d\n", label, rr.ret, rp.ret);
		return fail(st_main, label);
	}
	if (rr.out != rp.out) {
		std::printf("    %s: stdout mismatch\n", label);
		std::printf("      ref: %s", rr.out.c_str());
		std::printf("      port:%s", rp.out.c_str());
		return fail(st_main, label);
	}
	return true;
}

static bool
run_case_vec(const char *label, const std::vector<int> &vals)
{
	std::string input = format_input(vals);
	return run_case(label, input.c_str());
}

static void
test_main_hand()
{
	run_case("four_zeros", "0\n0\n0\n0\n");
	run_case("four_ones", "1\n1\n1\n1\n");
	run_case("four_max", "ffff\nffff\nffff\nffff\n");
	run_case("four_high_bit", "80\nff\n7f\n80\n");
	run_case("four_boundary_buf3_hi", "0\n0\n0\n1234\n");
	run_case("four_boundary_buf3_ff", "0\n0\n0\nff00\n");
	run_case("five_loop_once", "1\n2\n3\n4\n5\n");
	run_case("six_loop_twice", "1\n2\n3\n4\n5\n6\n");
	run_case("seven_mixed", "dead\nbeef\ncafe\nbabe\n1\nffff\n80\n");
	run_case("ten_carry", "1\n2\n3\n4\n5\n6\n7\n8\n9\na\n");
	run_case("twenty_ff", "0\n0\n0\n0\n"
	    "ffff\nffff\nffff\nffff\nffff\n"
	    "ffff\nffff\nffff\nffff\nffff\n"
	    "ffff\nffff\nffff\nffff\nffff\n");
	run_case("buf3_shift_low", "0\n0\n0\nff\n");
	run_case("buf3_shift_high", "0\n0\n0\n100\n");
	run_case("buf2_max_buf3_byte", "ffff\n0\nffff\n1234\n");
	run_case("loop_high_words", "0\n0\n0\n0\n80ff\nc000\ndead\nbeef\n");
	run_case("single_extra", "10\n20\n30\n40\n50\n");
	run_case("whitespace_sep", "10 20 30 40 50\n");
	run_case("high_trunc_store", "0\n0\n0\n0\n10000\n");
	run_case("negative_scanf", "0\n0\n0\n0\n-1\n");
	run_case("many_small", "1\n2\n3\n4\n"
	    "1\n2\n3\n4\n1\n2\n3\n4\n1\n2\n3\n4\n");
}

static void
test_main_sweep()
{
	std::vector<int> vals;

	for (long i = 0; i < SWEEP; i++) {
		int n = rng.count_4_64();
		vals.clear();
		vals.reserve(static_cast<std::size_t>(n));
		for (int j = 0; j < n; j++)
			vals.push_back(rng.next_u16());
		run_case_vec("sweep", vals);
	}
}

int
main()
{
	std::printf("PBSD batch b0167 differential test\n");
	std::printf("(oracle.c is the specification)\n\n");

	test_main_hand();
	test_main_sweep();

	std::printf("%-32s %12s %12s %10s\n", "function", "cases",
	    "failures", "result");
	std::printf("%-32s %12s %12s %10s\n",
	    "--------------------------------", "------------",
	    "------------", "----------");
	std::printf("%-32s %12ld %12ld %10s\n", st_main.name, st_main.cases,
	    st_main.fails, st_main.fails == 0 ? "PASS" : "FAIL");
	std::printf("%-32s %12s %12s %10s\n",
	    "--------------------------------", "------------",
	    "------------", "----------");
	std::printf("%-32s %12ld %12ld %10s\n", "TOTAL", st_main.cases,
	    st_main.fails, st_main.fails == 0 ? "PASS" : "FAIL");

	std::printf("\n%ld case(s) run, %ld failure(s).\n", st_main.cases,
	    st_main.fails);

	return st_main.fails == 0 ? 0 : 1;
}
