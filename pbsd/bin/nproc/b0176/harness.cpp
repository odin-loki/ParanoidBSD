/*
 * harness.cpp -- differential test for PBSD batch b0176 (nproc.c).
 *
 * Each function is exercised in a forked child so exit paths and stdio can be
 * captured without disturbing the parent.  ref_* oracle symbols are the spec.
 */

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <setjmp.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

import pbsd.bin.nproc.b0176;

namespace P = pbsd::bin_nproc::b0176;

extern "C" {
extern jmp_buf oracle_err_jmp;
extern int oracle_err_called;
extern int oracle_err_status;
void oracle_err_arm(void);
void oracle_err_disarm(void);
void ref_help(void);
void ref_usage(void);
void ref_version(void);
int ref_main(int argc, char *argv[]);
}

#define SWEEP 200000L
#define MAX_SHOW 8

namespace {

struct Stat {
	const char *name;
	long cases;
	long fails;
	int shown;
};

struct IoResult {
	int exit_status;
	std::vector<unsigned char> out;
	std::vector<unsigned char> err;
};

Stat st_help = { "help", 0, 0, 0 };
Stat st_usage = { "usage", 0, 0, 0 };
Stat st_version = { "version", 0, 0, 0 };
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

Rng rng(0x00b0176faceULL);

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
bytes_equal(const std::vector<unsigned char> &a,
    const std::vector<unsigned char> &b)
{
	if (a.size() != b.size())
		return false;
	return std::memcmp(a.data(), b.data(), a.size()) == 0;
}

void
drain_pipe(int fd, std::vector<unsigned char> &dst)
{
	unsigned char buf[8192];
	ssize_t nr;

	while ((nr = read(fd, buf, sizeof(buf))) > 0) {
		dst.insert(dst.end(), buf, buf + nr);
	}
}

enum class FnKind {
	Help,
	Usage,
	Version,
	Main,
};

IoResult
run_child(FnKind kind, bool use_port, int argc, char **argv)
{
	int outpipe[2];
	int errpipe[2];
	pid_t pid;

	if (pipe(outpipe) != 0 || pipe(errpipe) != 0) {
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
		close(errpipe[0]);
		if (dup2(outpipe[1], STDOUT_FILENO) < 0)
			_exit(127);
		if (dup2(errpipe[1], STDERR_FILENO) < 0)
			_exit(127);
		close(outpipe[1]);
		close(errpipe[1]);

		if (use_port) {
			switch (kind) {
			case FnKind::Help:
				P::help();
				_exit(0);
			case FnKind::Usage:
				P::usage();
				_exit(99);
			case FnKind::Version:
				P::version();
				_exit(99);
			case FnKind::Main:
				P::port_err_arm();
				if (setjmp(P::port_err_jmp) == 0) {
					int ret = P::main(argc, argv);
					P::port_err_disarm();
					_exit(ret);
				}
				P::port_err_disarm();
				_exit(P::port_err_status);
			}
		}

		switch (kind) {
		case FnKind::Help:
			ref_help();
			_exit(0);
		case FnKind::Usage:
			ref_usage();
			_exit(99);
		case FnKind::Version:
			ref_version();
			_exit(99);
		case FnKind::Main:
			oracle_err_arm();
			if (setjmp(oracle_err_jmp) == 0) {
				int ret = ref_main(argc, argv);
				oracle_err_disarm();
				_exit(ret);
			}
			oracle_err_disarm();
			_exit(oracle_err_status);
		}
	}

	close(outpipe[1]);
	close(errpipe[1]);

	IoResult res{};
	drain_pipe(outpipe[0], res.out);
	drain_pipe(errpipe[0], res.err);
	close(outpipe[0]);
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
compare_io(Stat &st, const char *label, const IoResult &ref,
    const IoResult &port)
{
	if (ref.exit_status != port.exit_status) {
		std::printf("    %s: exit %d vs %d\n", label, ref.exit_status,
		    port.exit_status);
		return fail(st, label);
	}
	if (!bytes_equal(ref.out, port.out)) {
		std::printf("    %s: stdout len %zu vs %zu\n", label,
		    ref.out.size(), port.out.size());
		return fail(st, label);
	}
	if (!bytes_equal(ref.err, port.err)) {
		std::printf("    %s: stderr len %zu vs %zu\n", label,
		    ref.err.size(), port.err.size());
		return fail(st, label);
	}
	return true;
}

bool
run_help_case(const char *label)
{
	st_help.cases++;
	IoResult ref = run_child(FnKind::Help, false, 0, nullptr);
	IoResult port = run_child(FnKind::Help, true, 0, nullptr);
	return compare_io(st_help, label, ref, port);
}

bool
run_usage_case(const char *label)
{
	st_usage.cases++;
	IoResult ref = run_child(FnKind::Usage, false, 0, nullptr);
	IoResult port = run_child(FnKind::Usage, true, 0, nullptr);
	return compare_io(st_usage, label, ref, port);
}

bool
run_version_case(const char *label)
{
	st_version.cases++;
	IoResult ref = run_child(FnKind::Version, false, 0, nullptr);
	IoResult port = run_child(FnKind::Version, true, 0, nullptr);
	return compare_io(st_version, label, ref, port);
}

bool
run_main_case(const char *label, int argc, char **argv)
{
	st_main.cases++;
	IoResult ref = run_child(FnKind::Main, false, argc, argv);
	IoResult port = run_child(FnKind::Main, true, argc, argv);
	return compare_io(st_main, label, ref, port);
}

void
test_help_hand()
{
	run_help_case("direct");
}

void
test_usage_hand()
{
	run_usage_case("direct");
}

void
test_version_hand()
{
	run_version_case("direct");
}

void
test_main_hand()
{
	static char prog[] = "nproc";
	static char all[] = "--all";
	static char help[] = "--help";
	static char version[] = "--version";
	static char ignore0[] = "--ignore=0";
	static char ignore1[] = "--ignore=1";
	static char ignore_max[] = "--ignore=2147483647";
	static char ignore_bad_neg[] = "--ignore=-1";
	static char ignore_bad_alpha[] = "--ignore=abc";
	static char ignore_bad_empty[] = "--ignore=";
	static char ignore_bad_space[] = "--ignore= ";
	static char ignore_hi[] = "--ignore=\xff\xfe";
	static char ignore_0xff[] = "--ignore=255";
	static char extra[] = "extra";
	static char extra_hi[] = "\x80\xff";
	static char unknown[] = "--unknown";
	static char short_bad[] = "-a";
	static char empty[] = "";

	char *av_default[] = { prog, nullptr };
	run_main_case("default", 1, av_default);

	char *av_all[] = { prog, all, nullptr };
	run_main_case("all", 2, av_all);

	char *av_help[] = { prog, help, nullptr };
	run_main_case("help_flag", 2, av_help);

	char *av_version[] = { prog, version, nullptr };
	run_main_case("version_flag", 2, av_version);

	char *av_ignore0[] = { prog, ignore0, nullptr };
	run_main_case("ignore_zero", 2, av_ignore0);

	char *av_ignore1[] = { prog, ignore1, nullptr };
	run_main_case("ignore_one", 2, av_ignore1);

	char *av_ignore_max[] = { prog, ignore_max, nullptr };
	run_main_case("ignore_int_max", 2, av_ignore_max);

	char *av_all_ignore0[] = { prog, all, ignore0, nullptr };
	run_main_case("all_ignore_zero", 3, av_all_ignore0);

	char *av_all_ignore1[] = { prog, all, ignore1, nullptr };
	run_main_case("all_ignore_one", 3, av_all_ignore1);

	char *av_all_ignore_max[] = { prog, all, ignore_max, nullptr };
	run_main_case("all_ignore_int_max", 3, av_all_ignore_max);

	char *av_ignore_bad_neg[] = { prog, ignore_bad_neg, nullptr };
	run_main_case("ignore_negative", 2, av_ignore_bad_neg);

	char *av_ignore_bad_alpha[] = { prog, ignore_bad_alpha, nullptr };
	run_main_case("ignore_alpha", 2, av_ignore_bad_alpha);

	char *av_ignore_bad_empty[] = { prog, ignore_bad_empty, nullptr };
	run_main_case("ignore_empty", 2, av_ignore_bad_empty);

	char *av_ignore_bad_space[] = { prog, ignore_bad_space, nullptr };
	run_main_case("ignore_space", 2, av_ignore_bad_space);

	char *av_ignore_hi[] = { prog, ignore_hi, nullptr };
	run_main_case("ignore_high_bit", 2, av_ignore_hi);

	char *av_ignore_0xff[] = { prog, ignore_0xff, nullptr };
	run_main_case("ignore_255", 2, av_ignore_0xff);

	char *av_extra[] = { prog, extra, nullptr };
	run_main_case("extra_arg", 2, av_extra);

	char *av_extra_hi[] = { prog, extra_hi, nullptr };
	run_main_case("extra_high_bit", 2, av_extra_hi);

	char *av_all_extra[] = { prog, all, extra, nullptr };
	run_main_case("all_extra_arg", 3, av_all_extra);

	char *av_unknown[] = { prog, unknown, nullptr };
	run_main_case("unknown_option", 2, av_unknown);

	char *av_short_bad[] = { prog, short_bad, nullptr };
	run_main_case("short_option", 2, av_short_bad);

	char *av_empty[] = { prog, empty, nullptr };
	run_main_case("empty_operand", 2, av_empty);

	char *av_help_extra[] = { prog, help, extra, nullptr };
	run_main_case("help_extra_operand", 3, av_help_extra);

	char *av_version_extra[] = { prog, version, extra, nullptr };
	run_main_case("version_extra_operand", 3, av_version_extra);

	{
		static char ignore_eq[] = "--ignore";
		char *av_ignore_no_eq[] = { prog, ignore_eq, nullptr };
		run_main_case("ignore_no_equals", 2, av_ignore_no_eq);
	}

	{
		static char overflow[] = "--ignore=9999999999999999999";
		char *av_overflow[] = { prog, overflow, nullptr };
		run_main_case("ignore_overflow", 2, av_overflow);
	}

	{
		static char prog_hi[] = "\xffnproc";
		char *av_prog_hi[] = { prog_hi, nullptr };
		run_main_case("prog_high_bit", 1, av_prog_hi);
	}
}

std::vector<char>
rnd_string(std::size_t len)
{
	std::vector<char> s(len + 1, 0);
	for (std::size_t i = 0; i < len; i++) {
		s[i] = static_cast<char>(rng.next() & 0xffu);
	}
	return s;
}

void
test_main_sweep()
{
	std::vector<std::vector<char>> pool;
	pool.reserve(64);

	for (long i = 0; i < SWEEP; i++) {
		static char prog[] = "nproc";
		std::vector<char *> argv;
		argv.push_back(prog);

		std::uint64_t mode = rng.next() % 16u;

		switch (mode) {
		case 0:
			break;
		case 1: {
			static char all[] = "--all";
			argv.push_back(all);
			break;
		}
		case 2: {
			static char help[] = "--help";
			argv.push_back(help);
			break;
		}
		case 3: {
			static char version[] = "--version";
			argv.push_back(version);
			break;
		}
		case 4: {
			static char unknown[] = "--bogus";
			argv.push_back(unknown);
			break;
		}
		case 5: {
			static char short_bad[] = "-x";
			argv.push_back(short_bad);
			break;
		}
		default: {
			bool use_all = (rng.next() & 1u) != 0;
			if (use_all) {
				static char all[] = "--all";
				argv.push_back(all);
			}
			std::uint64_t ign = rng.next() % 23u;
			std::vector<char> opt;
			opt.push_back('-');
			opt.push_back('-');
			opt.push_back('i');
			opt.push_back('g');
			opt.push_back('n');
			opt.push_back('o');
			opt.push_back('r');
			opt.push_back('e');
			opt.push_back('=');
			if (ign == 0) {
				opt.push_back('0');
			} else if (ign == 1) {
				opt.push_back('1');
			} else if (ign == 2) {
				const char *s = "2147483647";
				opt.insert(opt.end(), s, s + std::strlen(s));
			} else if (ign == 3) {
				opt.push_back('-');
				opt.push_back('1');
			} else if (ign == 4) {
				const char *s = "abc";
				opt.insert(opt.end(), s, s + std::strlen(s));
			} else if (ign == 5) {
				/* empty value */
			} else if (ign == 6) {
				opt.push_back(' ');
			} else if (ign == 7) {
				opt.push_back('\xff');
			} else if (ign == 8) {
				const char *s = "9999999999999999999";
				opt.insert(opt.end(), s, s + std::strlen(s));
			} else {
				std::uint64_t val = rng.next() % 5000u;
				char buf[32];
				std::snprintf(buf, sizeof(buf), "%llu",
				    (unsigned long long)val);
				opt.insert(opt.end(), buf,
				    buf + std::strlen(buf));
			}
			opt.push_back('\0');
			pool.push_back(std::move(opt));
			argv.push_back(pool.back().data());
			break;
		}
		}

		if ((rng.next() & 7u) == 0u) {
			std::size_t len = static_cast<std::size_t>(
			    rng.next() % 16u);
			pool.push_back(rnd_string(len));
			argv.push_back(pool.back().data());
		}

		argv.push_back(nullptr);
		run_main_case("sweep", static_cast<int>(argv.size() - 1),
		    argv.data());

		if (pool.size() > 64) {
			pool.clear();
		}
	}
}

void
print_stat(const Stat &st)
{
	std::printf("%-32s %12ld %12ld %10s\n", st.name, st.cases,
	    st.fails, st.fails == 0 ? "PASS" : "FAIL");
}

} // namespace

int
main()
{
	std::printf("PBSD batch b0176 differential test\n");
	std::printf("(oracle.c is the specification)\n\n");

	test_help_hand();
	test_usage_hand();
	test_version_hand();
	test_main_hand();
	test_main_sweep();

	long total_cases = st_help.cases + st_usage.cases + st_version.cases +
	    st_main.cases;
	long total_fails = st_help.fails + st_usage.fails + st_version.fails +
	    st_main.fails;

	std::printf("%-32s %12s %12s %10s\n", "function", "cases",
	    "failures", "result");
	std::printf("%-32s %12s %12s %10s\n",
	    "--------------------------------", "------------",
	    "------------", "----------");
	print_stat(st_help);
	print_stat(st_usage);
	print_stat(st_version);
	print_stat(st_main);
	std::printf("%-32s %12s %12s %10s\n",
	    "--------------------------------", "------------",
	    "------------", "----------");
	std::printf("%-32s %12ld %12ld %10s\n", "TOTAL", total_cases,
	    total_fails, total_fails == 0 ? "PASS" : "FAIL");

	std::printf("\n%ld case(s) run, %ld failure(s).\n", total_cases,
	    total_fails);

	return total_fails == 0 ? 0 : 1;
}
