/*
 * harness.cpp -- differential test for PBSD batch b0175 (sleep.c).
 */

#define _GNU_SOURCE

#ifndef SIGINFO
#define SIGINFO SIGUSR1
#endif

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <cmath>
#include <setjmp.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

import pbsd.bin.sleep.b0175;

namespace P = pbsd::bin_sleep::b0175;

extern "C" {
extern jmp_buf oracle_err_jmp;
extern int oracle_err_called;
extern int oracle_err_status;
extern int oracle_warnx_called;
extern volatile sig_atomic_t report_requested;
extern int optind;
void oracle_err_arm(void);
void oracle_err_disarm(void);
double ref_parse_interval(const char *arg);
void ref_report_request(int signo);
void ref_usage(void);
int ref_main(int argc, char *argv[]);
extern int __real_nanosleep(const struct timespec *, struct timespec *);
extern void __real_exit(int status);
}

#define SWEEP 200000L
#define MAX_SHOW 8

namespace {

bool g_test_child = false;
int g_nanosleep_mode = 0;
int g_nanosleep_calls = 0;
bool g_set_report_on_eintr = false;

extern "C" void
__wrap_exit(int status)
{
	if (g_test_child) {
		::_exit(status);
	}
	__real_exit(status);
}

extern "C" int
__wrap_nanosleep(const struct timespec *req, struct timespec *rem)
{
	if (g_test_child) {
		g_nanosleep_calls++;
		if (g_nanosleep_mode == 2) {
			errno = EINVAL;
			return (-1);
		}
		if (g_nanosleep_mode == 1 && g_nanosleep_calls == 1) {
			if (rem != nullptr) {
				*rem = *req;
			}
			if (g_set_report_on_eintr) {
				report_requested = 1;
				P::report_requested = 1;
			}
			errno = EINTR;
			return (-1);
		}
		if (rem != nullptr) {
			rem->tv_sec = 0;
			rem->tv_nsec = 0;
		}
		return (0);
	}
	return __real_nanosleep(req, rem);
}

struct Stat {
	const char *name;
	long cases;
	long fails;
	int shown;
};

Stat st_parse_interval = { "parse_interval", 0, 0, 0 };
Stat st_report_request = { "report_request", 0, 0, 0 };
Stat st_usage = { "usage", 0, 0, 0 };
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

Rng rng(0x00b0175faceULL);

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

bool
double_eq(double a, double b)
{
	return std::memcmp(&a, &b, sizeof(double)) == 0;
}

bool
run_parse_case(const char *label, const char *arg)
{
	st_parse_interval.cases++;

	int saved_stderr = dup(STDERR_FILENO);
	int devnull = open("/dev/null", O_WRONLY);
	if (devnull >= 0) {
		dup2(devnull, STDERR_FILENO);
	}

	oracle_warnx_called = 0;
	double ref = ref_parse_interval(arg);

	P::port_warnx_called = 0;
	double port = P::parse_interval(arg);

	if (saved_stderr >= 0) {
		dup2(saved_stderr, STDERR_FILENO);
		close(saved_stderr);
	}
	if (devnull >= 0) {
		close(devnull);
	}

	if (!double_eq(ref, port)) {
		std::printf("    %s: arg=%s ref=%g port=%g\n", label, arg, ref,
		    port);
		return fail(st_parse_interval, label);
	}
	if (oracle_warnx_called != P::port_warnx_called) {
		std::printf("    %s: warnx %d vs %d\n", label,
		    oracle_warnx_called, P::port_warnx_called);
		return fail(st_parse_interval, label);
	}
	return true;
}

void
test_parse_interval_hand()
{
	run_parse_case("empty", "");
	run_parse_case("zero", "0");
	run_parse_case("one", "1");
	run_parse_case("negative", "-3");
	run_parse_case("decimal", "3.14");
	run_parse_case("tiny", "1e-10");
	run_parse_case("seconds_unit", "5s");
	run_parse_case("minutes_unit", "5m");
	run_parse_case("hours_unit", "2h");
	run_parse_case("days_unit", "1d");
	run_parse_case("day_chain", "1d");
	run_parse_case("half_minute", "0.5m");
	run_parse_case("invalid_unit", "5x");
	run_parse_case("invalid_alpha", "abc");
	run_parse_case("extra_chars", "5sx");
	run_parse_case("double_unit", "5ss");
	run_parse_case("nan", "nan");
	run_parse_case("NaN", "NaN");
	run_parse_case("plus_sign", "+7");
	run_parse_case("space_prefix", " 3");
	run_parse_case("high_bit_prefix", "\x80");
	run_parse_case("high_bit_suffix", "1\xff");
	run_parse_case("nul_heavy", "\0\0\0");
	run_parse_case("single_s", "s");
	run_parse_case("single_m", "m");
	run_parse_case("zero_s", "0s");
	run_parse_case("zero_m", "0m");
	run_parse_case("zero_h", "0h");
	run_parse_case("zero_d", "0d");
	run_parse_case("large_int", "2147483647");
	run_parse_case("over_int_max", "2147483648");
	run_parse_case("huge", "1e308");
	run_parse_case("inf_str", "inf");
	run_parse_case("minus_inf", "-inf");
	run_parse_case("unit_only_d", "1d");
	run_parse_case("fractional_h", "1.5h");
	run_parse_case("boundary_1e-9", "1e-9");
	run_parse_case("boundary_below", "9.99e-10");
	run_parse_case("byte_7f", "\x7f");
	run_parse_case("byte_80", "\x80");
	run_parse_case("byte_ff", "\xff");
	run_parse_case("mixed_hi", "\x80\x81\x82");
	run_parse_case("digit_hi_unit", "5\x80s");
	run_parse_case("m_upper", "5M");
	run_parse_case("trailing_space", "5 ");
	run_parse_case("leading_tab", "\t2s");
}

std::vector<char>
rnd_interval_string()
{
	std::size_t len = static_cast<std::size_t>(rng.next() % 32u);
	std::vector<char> s(len + 1, '\0');
	static const char units[] = "smhdxSMHDX";
	static const char digits[] = "0123456789.-+eE";

	for (std::size_t i = 0; i < len; i++) {
		std::uint64_t kind = rng.next() % 16u;
		if (kind < 8) {
			s[i] = digits[rng.next() % (sizeof(digits) - 1)];
		} else if (kind < 12) {
			s[i] = units[rng.next() % (sizeof(units) - 1)];
		} else {
			s[i] = static_cast<char>(rng.next() & 0xffu);
		}
	}
	return s;
}

void
test_parse_interval_sweep()
{
	for (long i = 0; i < SWEEP; i++) {
		std::vector<char> s = rnd_interval_string();
		run_parse_case("sweep", s.data());
	}
}

bool
run_report_case(const char *label, int signo)
{
	st_report_request.cases++;

	report_requested = 0;
	ref_report_request(signo);
	int ref_flag = report_requested;

	P::report_requested = 0;
	P::report_request(signo);
	int port_flag = P::report_requested;

	if (ref_flag != port_flag) {
		std::printf("    %s: flag %d vs %d signo=%d\n", label,
		    ref_flag, port_flag, signo);
		return fail(st_report_request, label);
	}
	if (ref_flag != 1) {
		std::printf("    %s: flag %d expect 1\n", label, ref_flag);
		return fail(st_report_request, label);
	}
	return true;
}

void
test_report_request_hand()
{
	run_report_case("signo_zero", 0);
	run_report_case("signo_one", 1);
	run_report_case("signo_siginfo", SIGINFO);
	run_report_case("signo_neg", -1);
	run_report_case("signo_high", 0x7f);
	run_report_case("signo_80", 0x80);
	run_report_case("signo_ff", 0xff);
}

void
test_report_request_sweep()
{
	for (long i = 0; i < SWEEP; i++) {
		int signo = static_cast<int>(rng.next() & 0x7fffffffu);
		if ((rng.next() & 1u) != 0) {
			signo = -signo;
		}
		run_report_case("sweep", signo);
	}
}

struct ChildResult {
	int exit_status;
	std::vector<unsigned char> err;
};

ChildResult
run_usage_child(bool use_port)
{
	int errpipe[2];
	pid_t pid;

	if (pipe(errpipe) != 0) {
		std::perror("pipe");
		std::exit(2);
	}

	pid = fork();
	if (pid < 0) {
		std::perror("fork");
		std::exit(2);
	}

	if (pid == 0) {
		close(errpipe[0]);
		if (dup2(errpipe[1], STDERR_FILENO) < 0) {
			::_exit(127);
		}
		close(errpipe[1]);
		g_test_child = true;
		if (use_port) {
			P::usage();
		} else {
			ref_usage();
		}
		::_exit(99);
	}

	close(errpipe[1]);

	ChildResult res{};
	unsigned char buf[4096];
	ssize_t nr;

	while ((nr = read(errpipe[0], buf, sizeof(buf))) > 0) {
		res.err.insert(res.err.end(), buf, buf + nr);
	}
	close(errpipe[0]);

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
run_usage_case(const char *label)
{
	st_usage.cases++;

	ChildResult ref = run_usage_child(false);
	ChildResult port = run_usage_child(true);

	if (ref.exit_status != port.exit_status) {
		std::printf("    %s: exit %d vs %d\n", label, ref.exit_status,
		    port.exit_status);
		return fail(st_usage, label);
	}
	if (ref.exit_status != 1) {
		std::printf("    %s: exit %d expect 1\n", label,
		    ref.exit_status);
		return fail(st_usage, label);
	}
	if (ref.err.size() != port.err.size() ||
	    std::memcmp(ref.err.data(), port.err.data(), ref.err.size()) != 0) {
		std::printf("    %s: stderr len %zu vs %zu\n", label,
		    ref.err.size(), port.err.size());
		return fail(st_usage, label);
	}
	return true;
}

void
test_usage_hand()
{
	run_usage_case("hand");
}

void
test_usage_sweep()
{
	for (long i = 0; i < SWEEP; i++) {
		(void)i;
		run_usage_case("sweep");
	}
}

struct MainResult {
	int exit_status;
	std::vector<unsigned char> err;
	int nanosleep_calls;
};

MainResult
run_main_child(bool use_port, int argc, char **argv)
{
	int errpipe[2];
	int nspipe[2];
	pid_t pid;

	if (pipe(errpipe) != 0 || pipe(nspipe) != 0) {
		std::perror("pipe");
		std::exit(2);
	}

	pid = fork();
	if (pid < 0) {
		std::perror("fork");
		std::exit(2);
	}

	if (pid == 0) {
		close(errpipe[0]);
		close(nspipe[0]);
		if (dup2(errpipe[1], STDERR_FILENO) < 0) {
			::_exit(127);
		}
		close(errpipe[1]);
		g_test_child = true;
		g_nanosleep_calls = 0;
		report_requested = 0;
		P::report_requested = 0;
		optind = 1;

		int ret;
		unsigned char b;
		if (use_port) {
			P::port_err_arm();
			if (setjmp(P::port_err_jmp) == 0) {
				ret = P::main(argc, argv);
				P::port_err_disarm();
				b = static_cast<unsigned char>(g_nanosleep_calls);
				(void)write(nspipe[1], &b, 1);
				close(nspipe[1]);
				::_exit(ret);
			}
			P::port_err_disarm();
			b = static_cast<unsigned char>(g_nanosleep_calls);
			(void)write(nspipe[1], &b, 1);
			close(nspipe[1]);
			::_exit(P::port_err_status);
		}

		oracle_err_arm();
		if (setjmp(oracle_err_jmp) == 0) {
			ret = ref_main(argc, argv);
			oracle_err_disarm();
			b = static_cast<unsigned char>(g_nanosleep_calls);
			(void)write(nspipe[1], &b, 1);
			close(nspipe[1]);
			::_exit(ret);
		}
		oracle_err_disarm();
		b = static_cast<unsigned char>(g_nanosleep_calls);
		(void)write(nspipe[1], &b, 1);
		close(nspipe[1]);
		::_exit(oracle_err_status);
	}

	close(errpipe[1]);
	close(nspipe[1]);

	MainResult res{};
	unsigned char buf[8192];
	ssize_t nr;

	while ((nr = read(errpipe[0], buf, sizeof(buf))) > 0) {
		res.err.insert(res.err.end(), buf, buf + nr);
	}
	close(errpipe[0]);

	unsigned char nsb = 0;
	if (read(nspipe[0], &nsb, 1) == 1) {
		res.nanosleep_calls = nsb;
	}
	close(nspipe[0]);

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
compare_main_results(const char *label, const MainResult &ref,
    const MainResult &port)
{
	if (ref.exit_status != port.exit_status) {
		std::printf("    %s: exit %d vs %d\n", label, ref.exit_status,
		    port.exit_status);
		return fail(st_main, label);
	}
	if (ref.nanosleep_calls != port.nanosleep_calls) {
		std::printf("    %s: nanosleep %d vs %d\n", label,
		    ref.nanosleep_calls, port.nanosleep_calls);
		return fail(st_main, label);
	}
	if (ref.err.size() != port.err.size() ||
	    std::memcmp(ref.err.data(), port.err.data(), ref.err.size()) != 0) {
		std::printf("    %s: stderr len %zu vs %zu\n", label,
		    ref.err.size(), port.err.size());
		return fail(st_main, label);
	}
	return true;
}

bool
run_main_case(const char *label, int argc, char **argv)
{
	st_main.cases++;

	g_nanosleep_mode = 0;
	g_set_report_on_eintr = false;

	MainResult ref = run_main_child(false, argc, argv);

	g_nanosleep_mode = 0;
	g_set_report_on_eintr = false;

	MainResult port = run_main_child(true, argc, argv);

	return compare_main_results(label, ref, port);
}

bool
run_main_case_nanosleep(int mode, bool set_report, const char *label,
    int argc, char **argv)
{
	st_main.cases++;

	g_nanosleep_mode = mode;
	g_set_report_on_eintr = set_report;

	MainResult ref = run_main_child(false, argc, argv);

	g_nanosleep_mode = mode;
	g_set_report_on_eintr = set_report;

	MainResult port = run_main_child(true, argc, argv);

	return compare_main_results(label, ref, port);
}

void
test_main_hand()
{
	static char prog[] = "sleep";
	static char zero[] = "0";
	static char tiny[] = "1e-10";
	static char one[] = "1";
	static char two[] = "2";
	static char one_s[] = "1s";
	static char one_m[] = "1m";
	static char bad[] = "notime";
	static char huge[] = "2147483648";
	static char badopt[] = "-x";
	static char hi[] = "\x80\xff";
	static char nul[] = "\0\0";
	static char frac[] = "1.5";
	static char neg[] = "-1";

	char *av_none[] = { prog, nullptr };
	run_main_case("no_args", 1, av_none);

	char *av_zero[] = { prog, zero, nullptr };
	run_main_case("zero_seconds", 2, av_zero);

	char *av_tiny[] = { prog, tiny, nullptr };
	run_main_case("tiny_exit", 2, av_tiny);

	char *av_one[] = { prog, one, nullptr };
	run_main_case("one_second", 2, av_one);

	char *av_two_sum[] = { prog, one, two, nullptr };
	run_main_case("sum_two", 3, av_two_sum);

	char *av_one_s[] = { prog, one_s, nullptr };
	run_main_case("one_s_unit", 2, av_one_s);

	char *av_one_m[] = { prog, one_m, nullptr };
	run_main_case("one_m_unit", 2, av_one_m);

	char *av_bad[] = { prog, bad, nullptr };
	run_main_case("invalid_interval", 2, av_bad);

	char *av_huge[] = { prog, huge, nullptr };
	run_main_case("over_int_max", 2, av_huge);

	char *av_badopt[] = { prog, badopt, one, nullptr };
	run_main_case("bad_option", 3, av_badopt);

	char *av_hi[] = { prog, hi, nullptr };
	run_main_case("high_bit_arg", 2, av_hi);

	char *av_nul[] = { prog, nul, nullptr };
	run_main_case("nul_heavy_arg", 2, av_nul);

	char *av_frac[] = { prog, frac, nullptr };
	run_main_case("fractional", 2, av_frac);

	char *av_neg[] = { prog, neg, nullptr };
	run_main_case("negative", 2, av_neg);

	char *av_boundary[] = { prog, one, nullptr };
	run_main_case("boundary_argc2", 2, av_boundary);

	run_main_case_nanosleep(1, true, "eintr_report", 2, av_one);
	run_main_case_nanosleep(2, false, "nanosleep_err", 2, av_one);
}

void
test_main_sweep()
{
	static char prog[] = "sleep";

	for (long i = 0; i < SWEEP; i++) {
		std::size_t extra = static_cast<std::size_t>(rng.next() % 6u);
		std::vector<std::vector<char>> pool;
		pool.reserve(extra + 1);

		for (std::size_t j = 0; j < extra + 1; j++) {
			pool.push_back(rnd_interval_string());
		}

		std::vector<char *> argv(extra + 2, nullptr);
		argv[0] = prog;
		for (std::size_t j = 0; j < extra + 1; j++) {
			argv[j + 1] = pool[j].data();
		}

		int mode = 0;
		bool set_report = false;
		if ((rng.next() & 0xffu) == 0u) {
			mode = 1;
			set_report = true;
		} else if ((rng.next() & 0x1ffu) == 0u) {
			mode = 2;
		}

		run_main_case_nanosleep(mode, set_report, "sweep",
		    static_cast<int>(extra + 2), argv.data());
	}
}

long
total_cases()
{
	return st_parse_interval.cases + st_report_request.cases +
	    st_usage.cases + st_main.cases;
}

long
total_fails()
{
	return st_parse_interval.fails + st_report_request.fails +
	    st_usage.fails + st_main.fails;
}

} // namespace

int
main()
{
	std::printf("PBSD batch b0175 differential test\n");
	std::printf("(oracle.c is the specification)\n\n");

	test_parse_interval_hand();
	test_parse_interval_sweep();
	test_report_request_hand();
	test_report_request_sweep();
	test_usage_hand();
	test_usage_sweep();
	test_main_hand();
	test_main_sweep();

	std::printf("%-32s %12s %12s %10s\n", "function", "cases",
	    "failures", "result");
	std::printf("%-32s %12s %12s %10s\n",
	    "--------------------------------", "------------",
	    "------------", "----------");

	Stat stats[] = { st_parse_interval, st_report_request, st_usage,
	    st_main };

	for (const Stat &st : stats) {
		std::printf("%-32s %12ld %12ld %10s\n", st.name, st.cases,
		    st.fails, st.fails == 0 ? "PASS" : "FAIL");
	}

	std::printf("%-32s %12s %12s %10s\n",
	    "--------------------------------", "------------",
	    "------------", "----------");
	std::printf("%-32s %12ld %12ld %10s\n", "TOTAL", total_cases(),
	    total_fails(), total_fails() == 0 ? "PASS" : "FAIL");

	std::printf("\n%ld case(s) run, %ld failure(s).\n", total_cases(),
	    total_fails());

	return total_fails() == 0 ? 0 : 1;
}
