/*
 * harness.cpp -- differential test for PBSD batch b0171 (echo.c).
 *
 * echo.c defines main(), which builds an iovec list and writev(2)s to stdout.
 * Each case runs ref_main and port::main in-process; stdout is captured by a
 * writev(2) linker wrap and compared byte-for-byte.
 */

#define _GNU_SOURCE

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <setjmp.h>
#include <sys/uio.h>
#include <unistd.h>
#include <vector>

import pbsd.bin.echo.b0171;

namespace P = pbsd::bin_echo::b0171;

extern "C" {
extern jmp_buf oracle_err_jmp;
extern int oracle_err_called;
extern int oracle_err_status;
void oracle_err_arm(void);
void oracle_err_disarm(void);
int ref_main(int argc, char *argv[]);
extern void *(*__malloc_hook)(size_t);
void *__real_malloc(size_t);
ssize_t __real_writev(int fd, const struct iovec *iov, int iovcnt);
}

#define SWEEP 200000L
#define MAX_SHOW 8

namespace {

int g_fail_malloc;
std::vector<unsigned char> g_writev_capture;

extern "C" void *
__wrap_malloc(size_t size)
{
	if (g_fail_malloc && __malloc_hook != nullptr)
		return __malloc_hook(size);
	return __real_malloc(size);
}

extern "C" ssize_t
__wrap_writev(int fd, const struct iovec *iov, int iovcnt)
{
	ssize_t total = 0;

	if (fd != STDOUT_FILENO)
		return __real_writev(fd, iov, iovcnt);

	for (int i = 0; i < iovcnt; i++) {
		const unsigned char *base =
		    static_cast<const unsigned char *>(iov[i].iov_base);
		std::size_t len = iov[i].iov_len;

		g_writev_capture.insert(g_writev_capture.end(), base, base + len);
		total += static_cast<ssize_t>(len);
	}
	return total;
}

struct Stat {
	const char *name;
	long cases;
	long fails;
	int shown;
};

struct EchoResult {
	int exit_status;
	int err_called;
	int err_status;
	std::vector<unsigned char> out;
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

Rng rng(0x00b0171faceULL);

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
run_ref(int argc, char **argv)
{
	EchoResult res{};

	g_writev_capture.clear();
	oracle_err_called = 0;
	oracle_err_status = 0;

	oracle_err_arm();
	if (setjmp(oracle_err_jmp) == 0) {
		res.exit_status = ref_main(argc, argv);
		oracle_err_disarm();
	} else {
		oracle_err_disarm();
		res.exit_status = oracle_err_status;
	}

	res.err_called = oracle_err_called;
	res.err_status = oracle_err_status;
	res.out = g_writev_capture;
	return res;
}

EchoResult
run_port(int argc, char **argv)
{
	EchoResult res{};

	g_writev_capture.clear();
	P::port_err_called = 0;
	P::port_err_status = 0;

	P::port_err_arm();
	if (setjmp(P::port_err_jmp) == 0) {
		res.exit_status = P::main(argc, argv);
		P::port_err_disarm();
	} else {
		P::port_err_disarm();
		res.exit_status = P::port_err_status;
	}

	res.err_called = P::port_err_called;
	res.err_status = P::port_err_status;
	res.out = g_writev_capture;
	return res;
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
	st_main.cases++;

	g_fail_malloc = 0;
	__malloc_hook = nullptr;

	EchoResult ref = run_ref(argc, argv);
	EchoResult port = run_port(argc, argv);

	if (ref.exit_status != port.exit_status) {
		std::printf("    %s: exit %d vs %d\n", label, ref.exit_status,
		    port.exit_status);
		return fail(st_main, label);
	}
	if (ref.err_called != port.err_called ||
	    ref.err_status != port.err_status) {
		std::printf("    %s: err %d/%d vs %d/%d\n", label,
		    ref.err_called, ref.err_status, port.err_called,
		    port.err_status);
		return fail(st_main, label);
	}
	if (!outputs_equal(ref.out, port.out)) {
		std::printf("    %s: stdout len %zu vs %zu\n", label,
		    ref.out.size(), port.out.size());
		return fail(st_main, label);
	}
	return true;
}

extern "C" void *
malloc_always_fail(size_t)
{
	return nullptr;
}

bool
run_malloc_fail_case(const char *label, int argc, char **argv)
{
	EchoResult ref{};
	EchoResult port{};

	st_main.cases++;

	g_fail_malloc = 1;
	__malloc_hook = malloc_always_fail;
	ref = run_ref(argc, argv);

	g_fail_malloc = 1;
	__malloc_hook = malloc_always_fail;
	port = run_port(argc, argv);

	g_fail_malloc = 0;
	__malloc_hook = nullptr;

	if (ref.exit_status != port.exit_status) {
		std::printf("    %s: exit %d vs %d\n", label, ref.exit_status,
		    port.exit_status);
		return fail(st_main, label);
	}
	if (ref.exit_status != 1) {
		std::printf("    %s: exit %d expect 1\n", label,
		    ref.exit_status);
		return fail(st_main, label);
	}
	if (!ref.out.empty() || !port.out.empty()) {
		std::printf("    %s: unexpected stdout on malloc fail\n",
		    label);
		return fail(st_main, label);
	}
	return true;
}

void
test_main_hand()
{
	static char prog[] = "echo";
	static char empty[] = "";
	static char one[] = "a";
	static char hi[] = "\x80\xff\xfe";
	static char nul_heavy[] = "\0a\0b\0c";
	static char nflag[] = "-n";
	static char backslash_c[] = "\\c";
	static char x_backslash_c[] = "x\\c";
	static char not_c[] = "x\\d";
	static char single_c[] = "c";
	static char two_char[] = "ab";
	static char arg1[] = "hello";
	static char arg2[] = "world";
	static char mid_c[] = "foo\\cbar";
	static char hi_c[] = "\xff\\c";

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

	char *av_bs_c[] = { prog, backslash_c, nullptr };
	run_case("backslash_c_only", 2, av_bs_c);

	char *av_x_bs_c[] = { prog, x_backslash_c, nullptr };
	run_case("x_backslash_c", 2, av_x_bs_c);

	char *av_not_c[] = { prog, not_c, nullptr };
	run_case("not_backslash_c", 2, av_not_c);

	char *av_single_c[] = { prog, single_c, nullptr };
	run_case("single_c_no_chop", 2, av_single_c);

	char *av_two_char[] = { prog, two_char, nullptr };
	run_case("two_char_boundary", 2, av_two_char);

	char *av_mid_c[] = { prog, mid_c, arg1, nullptr };
	run_case("backslash_c_not_last", 3, av_mid_c);

	char *av_hi_c[] = { prog, hi_c, nullptr };
	run_case("high_bit_backslash_c", 2, av_hi_c);

	char *av_n_not_first[] = { prog, one, nflag, nullptr };
	run_case("n_not_first_arg", 3, av_n_not_first);

	char *av_three[] = { prog, one, two_char, hi, nullptr };
	run_case("three_args", 4, av_three);

	char *av_boundary2[] = { prog, arg1, nullptr };
	run_case("boundary_argc2", 2, av_boundary2);

	char *av_boundary3[] = { prog, arg1, arg2, nullptr };
	run_case("boundary_argc3", 3, av_boundary3);

	char *av_n_boundary3[] = { prog, nflag, arg1, nullptr };
	run_case("boundary_n_argc3", 3, av_n_boundary3);

	{
		std::vector<std::vector<char>> many_bufs;
		std::vector<char *> many_argv;
		many_argv.push_back(prog);
		for (int i = 0; i < 600; i++) {
			many_bufs.push_back(
			    { static_cast<char>('a' + (i % 26)), '\0' });
			many_argv.push_back(many_bufs.back().data());
		}
		many_argv.push_back(nullptr);
		run_case("iov_max_chunking", (int)many_argv.size() - 1,
		    many_argv.data());
	}

	run_malloc_fail_case("malloc_fail_two_args", 3, av_two);
	run_malloc_fail_case("malloc_fail_prog_only", 1, av_prog_only);
}

std::vector<char>
rnd_string(std::size_t len, bool maybe_bs_c)
{
	std::vector<char> s(len + 1, 0);
	for (std::size_t i = 0; i < len; i++) {
		s[i] = static_cast<char>(rng.next() & 0xffu);
	}
	if (maybe_bs_c && len >= 2) {
		s[len - 2] = '\\';
		s[len - 1] = 'c';
	}
	return s;
}

void
test_main_sweep()
{
	std::vector<std::vector<char>> pool;
	pool.reserve(64);

	for (long i = 0; i < SWEEP; i++) {
		std::size_t extra = static_cast<std::size_t>(rng.next() % 16u);
		std::vector<char *> argv(extra + 3, nullptr);
		static char prog[] = "echo";

		argv[0] = prog;
		bool use_n = (rng.next() & 1u) != 0;
		std::size_t arg_start = 1;

		if (use_n) {
			static char nflag[] = "-n";
			argv[1] = nflag;
			arg_start = 2;
		}

		for (std::size_t j = arg_start; j < extra + 2; j++) {
			std::size_t len = static_cast<std::size_t>(
			    rng.next() % 48u);
			bool maybe_bs_c = (j == extra + 1) &&
			    ((rng.next() & 7u) == 0u);
			pool.push_back(rnd_string(len, maybe_bs_c));
			argv[j] = pool.back().data();
		}

		run_case("sweep", static_cast<int>(extra + 2), argv.data());

		if (pool.size() > 64) {
			pool.clear();
		}
	}
}

} // namespace

extern "C" void *(*__malloc_hook)(size_t) = nullptr;

int
main()
{
	std::printf("PBSD batch b0171 differential test\n");
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
