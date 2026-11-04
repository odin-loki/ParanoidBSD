/*
 * harness.cpp -- differential test for PBSD batch b0218 (bltin/echo.c).
 *
 * echocmd writes via putchar to stdout.  Each case forks a child to run
 * ref_echocmd or port::echocmd, captures stdout, and compares exit status
 * and output bytes.
 */

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

import pbsd.bin.sh.bltin.b0218;

namespace P = pbsd::bin_sh_bltin::b0218;

extern "C" int ref_echocmd(int argc, char *argv[]);

#define SWEEP 200000L
#define MAX_SHOW 8

namespace {

struct Stat {
	const char *name;
	long cases;
	long fails;
	int shown;
};

struct EchoResult {
	int exit_status;
	std::vector<unsigned char> out;
};

Stat st_echocmd = { "echocmd", 0, 0, 0 };

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
};

Rng rng(0x00b0218faceULL);

bool
fail(Stat &st, const char *what)
{
	st.fails++;
	if (st.shown < MAX_SHOW) {
		st.shown++;
		std::printf("  FAIL %s: %s\n", st.name, what);
	}
	return false;
}

EchoResult
run_child(bool use_port, int argc, char **argv)
{
	int outpipe[2];
	pid_t pid;

	if (pipe(outpipe) != 0) {
		std::perror("pipe");
		std::exit(2);
	}

	pid = fork();
	if (pid < 0) {
		std::perror("fork");
		std::exit(2);
	}

	if (pid == 0) {
		close(outpipe[0]);
		if (dup2(outpipe[1], STDOUT_FILENO) < 0)
			_exit(127);
		close(outpipe[1]);

		int ret = use_port ? P::echocmd(argc, argv) : ref_echocmd(argc, argv);
		_exit(ret);
	}

	close(outpipe[1]);

	EchoResult res{};
	unsigned char buf[8192];
	ssize_t nr;

	while ((nr = read(outpipe[0], buf, sizeof(buf))) > 0) {
		res.out.insert(res.out.end(), buf, buf + nr);
	}
	close(outpipe[0]);

	int status = 0;
	if (waitpid(pid, &status, 0) < 0) {
		std::perror("waitpid");
		std::exit(2);
	}
	if (WIFEXITED(status)) {
		res.exit_status = WEXITSTATUS(status);
	} else if (WIFSIGNALED(status)) {
		res.exit_status = 128 + WTERMSIG(status);
	}

	return res;
}

EchoResult
run_ref(int argc, char **argv)
{
	return run_child(false, argc, argv);
}

EchoResult
run_port(int argc, char **argv)
{
	return run_child(true, argc, argv);
}

bool
outputs_equal(const std::vector<unsigned char> &a,
    const std::vector<unsigned char> &b)
{
	if (a.size() != b.size())
		return false;
	return std::memcmp(a.data(), b.data(), a.size()) == 0;
}

bool
run_case(const char *label, int argc, char **argv)
{
	st_echocmd.cases++;

	EchoResult ref = run_ref(argc, argv);
	EchoResult port = run_port(argc, argv);

	if (ref.exit_status != port.exit_status) {
		std::printf("    %s: exit %d vs %d\n", label, ref.exit_status,
		    port.exit_status);
		return fail(st_echocmd, label);
	}
	if (!outputs_equal(ref.out, port.out)) {
		std::printf("    %s: stdout len %zu vs %zu\n", label,
		    ref.out.size(), port.out.size());
		return fail(st_echocmd, label);
	}
	return true;
}

void
test_echocmd_hand()
{
	static char prog[] = "echo";
	static char empty[] = "";
	static char one[] = "a";
	static char hi[] = "\x80\xff\xfe";
	static char nul_heavy[] = "\0a\0b\0c";
	static char nflag[] = "-n";
	static char eflag[] = "-e";
	static char arg1[] = "hello";
	static char arg2[] = "world";
	static char esc_n[] = "\\n";
	static char esc_c[] = "\\c";
	static char esc_a[] = "\\a";
	static char esc_b[] = "\\b";
	static char esc_e[] = "\\e";
	static char esc_f[] = "\\f";
	static char esc_r[] = "\\r";
	static char esc_t[] = "\\t";
	static char esc_v[] = "\\v";
	static char esc_bs[] = "\\\\";
	static char esc_nul[] = "\\0";
	static char esc_oct3[] = "\\123";
	static char esc_oct8[] = "\\078";
	static char esc_oct0[] = "\\0";
	static char esc_bad[] = "\\z";
	static char lone_bs[] = "\\";
	static char bs_hi[] = "\\xff";
	static char not_n[] = "x-n";
	static char not_e[] = "x-e";
	static char two_bs[] = "\\\\";
	static char pre_c[] = "pre\\c";
	static char mid[] = "a\\tb";
	static char hi_esc[] = "\xff\\n";

	char *av_zero[] = { nullptr };
	run_case("argc_zero", 0, av_zero);

	char *av_prog_only[] = { prog, nullptr };
	run_case("prog_only_newline", 1, av_prog_only);

	char *av_empty[] = { prog, empty, nullptr };
	run_case("one_empty_arg", 2, av_empty);

	char *av_one[] = { prog, one, nullptr };
	run_case("one_char", 2, av_one);

	char *av_hi[] = { prog, hi, nullptr };
	run_case("high_bit_bytes", 2, av_hi);

	char *av_nul[] = { prog, nul_heavy, nullptr };
	run_case("nul_heavy", 2, av_nul);

	char *av_n_only[] = { prog, nflag, nullptr };
	run_case("n_flag_only", 2, av_n_only);

	char *av_n_one[] = { prog, nflag, one, nullptr };
	run_case("n_flag_one_arg", 3, av_n_one);

	char *av_n_two[] = { prog, nflag, arg1, arg2, nullptr };
	run_case("n_flag_two_args", 4, av_n_two);

	char *av_two[] = { prog, arg1, arg2, nullptr };
	run_case("two_args_newline", 3, av_two);

	char *av_three[] = { prog, one, arg1, hi, nullptr };
	run_case("three_args", 4, av_three);

	char *av_n_not_first[] = { prog, one, nflag, nullptr };
	run_case("n_not_first_arg", 3, av_n_not_first);

	char *av_e_only[] = { prog, eflag, nullptr };
	run_case("e_flag_only", 2, av_e_only);

	char *av_e_n[] = { prog, eflag, esc_n, nullptr };
	run_case("e_escape_n", 3, av_e_n);

	char *av_e_c[] = { prog, eflag, esc_c, nullptr };
	run_case("e_escape_c_exit", 3, av_e_c);

	char *av_e_a[] = { prog, eflag, esc_a, nullptr };
	run_case("e_escape_a", 3, av_e_a);

	char *av_e_b[] = { prog, eflag, esc_b, nullptr };
	run_case("e_escape_b", 3, av_e_b);

	char *av_e_e[] = { prog, eflag, esc_e, nullptr };
	run_case("e_escape_e", 3, av_e_e);

	char *av_e_f[] = { prog, eflag, esc_f, nullptr };
	run_case("e_escape_f", 3, av_e_f);

	char *av_e_r[] = { prog, eflag, esc_r, nullptr };
	run_case("e_escape_r", 3, av_e_r);

	char *av_e_t[] = { prog, eflag, esc_t, nullptr };
	run_case("e_escape_t", 3, av_e_t);

	char *av_e_v[] = { prog, eflag, esc_v, nullptr };
	run_case("e_escape_v", 3, av_e_v);

	char *av_e_bs[] = { prog, eflag, esc_bs, nullptr };
	run_case("e_escape_backslash", 3, av_e_bs);

	char *av_e_nul[] = { prog, eflag, esc_nul, nullptr };
	run_case("e_escape_nul", 3, av_e_nul);

	char *av_e_oct3[] = { prog, eflag, esc_oct3, nullptr };
	run_case("e_escape_oct3", 3, av_e_oct3);

	char *av_e_oct8[] = { prog, eflag, esc_oct8, nullptr };
	run_case("e_escape_oct8_stop", 3, av_e_oct8);

	char *av_e_oct0[] = { prog, eflag, esc_oct0, nullptr };
	run_case("e_escape_oct0", 3, av_e_oct0);

	char *av_e_bad[] = { prog, eflag, esc_bad, nullptr };
	run_case("e_escape_unknown", 3, av_e_bad);

	char *av_lone_bs_e[] = { prog, eflag, lone_bs, nullptr };
	run_case("e_lone_backslash", 3, av_lone_bs_e);

	char *av_lone_bs[] = { prog, lone_bs, nullptr };
	run_case("no_e_lone_backslash", 2, av_lone_bs);

	char *av_two_bs_e[] = { prog, eflag, two_bs, nullptr };
	run_case("e_double_backslash", 3, av_two_bs_e);

	char *av_pre_c_e[] = { prog, eflag, pre_c, nullptr };
	run_case("e_pre_c_exit", 3, av_pre_c_e);

	char *av_mid_e[] = { prog, eflag, mid, nullptr };
	run_case("e_mixed_escape", 3, av_mid_e);

	char *av_hi_esc_e[] = { prog, eflag, hi_esc, nullptr };
	run_case("e_high_bit_escape", 3, av_hi_esc_e);

	char *av_bs_hi_e[] = { prog, eflag, bs_hi, nullptr };
	run_case("e_octal_high", 3, av_bs_hi_e);

	char *av_not_n[] = { prog, not_n, nullptr };
	run_case("not_n_flag", 2, av_not_n);

	char *av_not_e[] = { prog, not_e, nullptr };
	run_case("not_e_flag", 2, av_not_e);

	char *av_e_not_first[] = { prog, one, eflag, esc_n, nullptr };
	run_case("e_not_first_arg", 4, av_e_not_first);

	char *av_n_then_e[] = { prog, nflag, eflag, esc_n, nullptr };
	run_case("n_then_e_literal", 4, av_n_then_e);

	char *av_e_two[] = { prog, eflag, arg1, arg2, nullptr };
	run_case("e_two_args_space", 4, av_e_two);

	char *av_e_n_two[] = { prog, eflag, nflag, arg1, nullptr };
	run_case("e_flag_n_literal", 4, av_e_n_two);
}

std::vector<char>
rnd_string(std::size_t len, bool with_escapes)
{
	std::vector<char> s(len + 1, 0);
	static const char esc_chars[] = "abcefnrtv\\0";
	for (std::size_t i = 0; i < len; i++) {
		if (with_escapes && (rng.next() & 7u) == 0u) {
			s[i] = '\\';
		} else {
			s[i] = static_cast<char>(rng.next() & 0xffu);
		}
	}
	if (with_escapes && len >= 2 && (rng.next() & 15u) == 0u) {
		s[len - 2] = '\\';
		s[len - 1] = esc_chars[rng.next() % 11u];
	}
	return s;
}

void
test_echocmd_sweep()
{
	std::vector<std::vector<char>> pool;
	pool.reserve(64);

	for (long i = 0; i < SWEEP; i++) {
		std::size_t extra = static_cast<std::size_t>(rng.next() % 12u);
		std::vector<char *> argv(extra + 3, nullptr);
		static char prog[] = "echo";
		static char nflag[] = "-n";
		static char eflag[] = "-e";

		argv[0] = prog;
		std::size_t arg_start = 1;
		bool use_n = (rng.next() & 3u) == 0u;
		bool use_e = (rng.next() & 3u) == 0u;

		if (use_n) {
			argv[arg_start] = nflag;
			arg_start++;
		} else if (use_e) {
			argv[arg_start] = eflag;
			arg_start++;
		}

		for (std::size_t j = arg_start; j < extra + 2; j++) {
			std::size_t len = static_cast<std::size_t>(rng.next() % 40u);
			bool escapes = use_e || use_n;
			if (!use_n && !use_e && (rng.next() & 7u) == 0u)
				escapes = true;
			pool.push_back(rnd_string(len, escapes));
			argv[j] = pool.back().data();
		}

		run_case("sweep", static_cast<int>(extra + 2), argv.data());

		if (pool.size() > 64)
			pool.clear();
	}
}

} // namespace

int
main()
{
	std::printf("PBSD batch b0218 differential test\n");
	std::printf("(oracle.c is the specification)\n\n");

	test_echocmd_hand();
	test_echocmd_sweep();

	std::printf("%-32s %12s %12s %10s\n", "function", "cases",
	    "failures", "result");
	std::printf("%-32s %12s %12s %10s\n",
	    "--------------------------------", "------------",
	    "------------", "----------");
	std::printf("%-32s %12ld %12ld %10s\n", st_echocmd.name, st_echocmd.cases,
	    st_echocmd.fails, st_echocmd.fails == 0 ? "PASS" : "FAIL");
	std::printf("%-32s %12s %12s %10s\n",
	    "--------------------------------", "------------",
	    "------------", "----------");
	std::printf("%-32s %12ld %12ld %10s\n", "TOTAL", st_echocmd.cases,
	    st_echocmd.fails, st_echocmd.fails == 0 ? "PASS" : "FAIL");

	std::printf("\n%ld case(s) run, %ld failure(s).\n", st_echocmd.cases,
	    st_echocmd.fails);

	return st_echocmd.fails == 0 ? 0 : 1;
}
