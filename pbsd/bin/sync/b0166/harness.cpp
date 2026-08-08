/*
 * harness.cpp -- differential test for PBSD batch b0166 (sync.c).
 *
 * sync.c defines main(), which calls sync() once and exit(0).  Each case
 * forks a child to run ref_main or port::main (they never return), counts
 * sync() via --wrap=sync, and compares exit status and sync call count.
 */

#define _GNU_SOURCE

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

import pbsd.bin.sync.b0166;

namespace P = pbsd::bin_sync::b0166;

extern "C" int ref_main(int argc, char *argv[]);
extern "C" int __real_sync(void);
extern "C" void __real_exit(int status);

#define SWEEP 200000L
#define MAX_SHOW 8

namespace {

int g_sync_pipe = -1;
bool g_test_child = false;

extern "C" void
__wrap_exit(int status)
{
	if (g_test_child) {
		::_exit(status);
	}
	__real_exit(status);
}

extern "C" int
__wrap_sync(void)
{
	if (g_sync_pipe >= 0) {
		unsigned char b = 1;
		(void)write(g_sync_pipe, &b, 1);
	}
	return __real_sync();
}

struct Stat {
	const char *name;
	long cases;
	long fails;
	int shown;
};

struct RunResult {
	int exit_status;
	int sync_calls;
};

Stat st_main = { "main", 0, 0, 0 };

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

Rng rng(0x00b0166faceULL);

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

RunResult
run_ref(int argc, char **argv)
{
	int pipefd[2];

	if (pipe(pipefd) != 0) {
		std::perror("pipe");
		std::exit(2);
	}

	pid_t pid = fork();
	if (pid < 0) {
		std::perror("fork");
		std::exit(2);
	}
	if (pid == 0) {
		close(pipefd[0]);
		g_sync_pipe = pipefd[1];
		g_test_child = true;
		ref_main(argc, argv);
		::_exit(99);
	}

	close(pipefd[1]);
	g_sync_pipe = -1;

	RunResult res = { -1, 0 };
	unsigned char b;
	while (read(pipefd[0], &b, 1) == 1) {
		res.sync_calls++;
	}
	close(pipefd[0]);

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

RunResult
run_port(int argc, char **argv)
{
	int pipefd[2];

	if (pipe(pipefd) != 0) {
		std::perror("pipe");
		std::exit(2);
	}

	pid_t pid = fork();
	if (pid < 0) {
		std::perror("fork");
		std::exit(2);
	}
	if (pid == 0) {
		close(pipefd[0]);
		g_sync_pipe = pipefd[1];
		g_test_child = true;
		P::main(argc, argv);
		::_exit(99);
	}

	close(pipefd[1]);
	g_sync_pipe = -1;

	RunResult res = { -1, 0 };
	unsigned char b;
	while (read(pipefd[0], &b, 1) == 1) {
		res.sync_calls++;
	}
	close(pipefd[0]);

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

bool
run_case(const char *label, int argc, char **argv)
{
	st_main.cases++;

	RunResult ref = run_ref(argc, argv);
	RunResult port = run_port(argc, argv);

	if (ref.exit_status != port.exit_status) {
		std::printf("    %s: exit %d vs %d\n", label, ref.exit_status,
		    port.exit_status);
		return fail(st_main, label);
	}
	if (ref.sync_calls != port.sync_calls) {
		std::printf("    %s: sync_calls %d vs %d\n", label,
		    ref.sync_calls, port.sync_calls);
		return fail(st_main, label);
	}
	if (ref.exit_status != 0) {
		std::printf("    %s: exit %d expect 0\n", label,
		    ref.exit_status);
		return fail(st_main, label);
	}
	if (ref.sync_calls != 1) {
		std::printf("    %s: sync_calls %d expect 1\n", label,
		    ref.sync_calls);
		return fail(st_main, label);
	}
	return true;
}

void
test_main_hand()
{
	static char empty[] = "";
	static char one[] = "sync";
	static char hi[] = "\x80\xff\xfe";
	static char nul_heavy[] = "\0a\0b\0c";
	static char argv0_only[] = "x";
	static char argv0_hi[] = "\xff";
	static char argv1[] = "arg1";
	static char argv2[] = "arg2";

	char *av0[] = { nullptr };
	run_case("argc0_argv_null", 0, av0);

	char *av1[] = { empty, nullptr };
	run_case("argc1_empty", 1, av1);

	char *av2[] = { one, nullptr };
	run_case("argc1_sync", 1, av2);

	char *av3[] = { hi, nullptr };
	run_case("argc1_high_bit", 1, av3);

	char *av4[] = { nul_heavy, nullptr };
	run_case("argc1_nul_heavy", 1, av4);

	char *av5[] = { argv0_only, nullptr };
	run_case("argc1_single_char", 1, av5);

	char *av6[] = { argv0_hi, nullptr };
	run_case("argc1_byte_ff", 1, av6);

	char *av7[] = { argv1, argv2, nullptr };
	run_case("argc2_two_args", 2, av7);

	char *av8[] = { empty, empty, nullptr };
	run_case("argc2_both_empty", 2, av8);

	char *av9[] = { hi, nul_heavy, nullptr };
	run_case("argc2_mixed_bytes", 2, av9);

	char *av10[] = { argv0_only, argv0_hi, argv1, nullptr };
	run_case("argc3_mixed", 3, av10);

	char *av11[] = { argv0_only, nullptr };
	run_case("boundary_argc1", 1, av11);

	char *av12[] = { argv0_only, argv1, nullptr };
	run_case("boundary_argc2", 2, av12);
}

std::vector<char>
rnd_string(std::size_t len)
{
	std::vector<char> s(len + 1, 0);
	for (std::size_t i = 0; i < len; i++) {
		s[i] = (char)(rng.next() & 0xffu);
	}
	return s;
}

void
test_main_sweep()
{
	std::vector<std::vector<char>> pool;
	pool.reserve(32);

	for (long i = 0; i < SWEEP; i++) {
		std::size_t argc = (std::size_t)(rng.next() % 11u);
		std::vector<char *> argv(argc + 1, nullptr);

		for (std::size_t j = 0; j < argc; j++) {
			std::size_t len = (std::size_t)(rng.next() % 64u);
			pool.push_back(rnd_string(len));
			argv[j] = pool.back().data();
		}

		run_case("sweep", (int)argc, argv.data());

		if (pool.size() > 64) {
			pool.clear();
		}
	}
}

} // namespace

int
main()
{
	std::printf("PBSD batch b0166 differential test\n");
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
