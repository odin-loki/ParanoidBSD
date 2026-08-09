/*
 * harness.cpp -- differential test for PBSD batch b0169 (realpath.c).
 *
 * main() and usage() call exit(); each case forks a child, captures stdout
 * and stderr, and compares exit status and full stream output.
 */

#define _GNU_SOURCE

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

import pbsd.bin.realpath.b0169;

namespace P = pbsd::bin_realpath::b0169;

extern "C" int ref_main(int argc, char *argv[]);
extern "C" void ref_usage(void);

#define SWEEP 200000L
#define MAX_SHOW 8

namespace {

struct Stat {
	const char *name;
	long cases;
	long fails;
	int shown;
};

struct RunResult {
	int exit_status;
	std::vector<unsigned char> stdout_bytes;
	std::vector<unsigned char> stderr_bytes;
};

Stat st_main = { "main", 0, 0, 0 };
Stat st_usage = { "usage", 0, 0, 0 };

char g_tmpdir[PATH_MAX];
char g_tmpfile[PATH_MAX];
char g_subdir[PATH_MAX];

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

Rng rng(0x00b0169faceULL);

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

std::vector<unsigned char>
read_pipe_all(int fd)
{
	std::vector<unsigned char> out;
	unsigned char buf[8192];
	ssize_t nr;

	while ((nr = read(fd, buf, sizeof(buf))) > 0) {
		out.insert(out.end(), buf, buf + nr);
	}
	return out;
}

RunResult
run_child(void (*fn)(void), int (*main_fn)(int, char **), int argc, char **argv)
{
	int outpipe[2];
	int errpipe[2];
	pid_t pid;
	int status = 0;

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
		dup2(outpipe[1], STDOUT_FILENO);
		dup2(errpipe[1], STDERR_FILENO);
		close(outpipe[1]);
		close(errpipe[1]);
		int devnull = open("/dev/null", O_RDONLY);
		if (devnull >= 0) {
			dup2(devnull, STDIN_FILENO);
			close(devnull);
		}
		if (fn != nullptr) {
			fn();
		} else {
			main_fn(argc, argv);
		}
		_exit(99);
	}

	close(outpipe[1]);
	close(errpipe[1]);

	RunResult res;
	res.stdout_bytes = read_pipe_all(outpipe[0]);
	res.stderr_bytes = read_pipe_all(errpipe[0]);
	close(outpipe[0]);
	close(errpipe[0]);

	if (waitpid(pid, &status, 0) < 0) {
		std::perror("waitpid");
		std::exit(2);
	}
	if (WIFEXITED(status)) {
		res.exit_status = WEXITSTATUS(status);
	} else if (WIFSIGNALED(status)) {
		res.exit_status = 128 + WTERMSIG(status);
	} else {
		res.exit_status = -1;
	}
	return res;
}

RunResult
run_ref_main(int argc, char **argv)
{
	return run_child(nullptr, ref_main, argc, argv);
}

RunResult
run_port_main(int argc, char **argv)
{
	return run_child(nullptr,
	    [](int argc, char **argv) -> int {
		    return P::main(argc, argv);
	    },
	    argc, argv);
}

void
call_ref_usage(void)
{
	ref_usage();
}

void
call_port_usage(void)
{
	P::usage();
}

RunResult
run_ref_usage(void)
{
	return run_child(call_ref_usage, nullptr, 0, nullptr);
}

RunResult
run_port_usage(void)
{
	return run_child(call_port_usage, nullptr, 0, nullptr);
}

bool
same_bytes(const std::vector<unsigned char> &a,
    const std::vector<unsigned char> &b)
{
	return a == b;
}

bool
compare_runs(Stat &st, const char *label, const RunResult &ref,
    const RunResult &port)
{
	if (ref.exit_status != port.exit_status) {
		std::printf("    %s: exit %d vs %d\n", label, ref.exit_status,
		    port.exit_status);
		return fail(st, label);
	}
	if (!same_bytes(ref.stdout_bytes, port.stdout_bytes)) {
		std::printf("    %s: stdout mismatch (%zu vs %zu bytes)\n",
		    label, ref.stdout_bytes.size(), port.stdout_bytes.size());
		return fail(st, label);
	}
	if (!same_bytes(ref.stderr_bytes, port.stderr_bytes)) {
		std::printf("    %s: stderr mismatch (%zu vs %zu bytes)\n",
		    label, ref.stderr_bytes.size(), port.stderr_bytes.size());
		return fail(st, label);
	}
	return true;
}

bool
run_main_case(const char *label, int argc, char **argv)
{
	st_main.cases++;
	RunResult ref = run_ref_main(argc, argv);
	RunResult port = run_port_main(argc, argv);
	return compare_runs(st_main, label, ref, port);
}

bool
run_usage_case(const char *label)
{
	st_usage.cases++;
	RunResult ref = run_ref_usage();
	RunResult port = run_port_usage();
	return compare_runs(st_usage, label, ref, port);
}

void
setup_tempdir(void)
{
	const char *base = getenv("TMPDIR");
	const char *td;

	if (base == nullptr || base[0] == '\0') {
		base = "/tmp";
	}
	std::snprintf(g_tmpdir, sizeof(g_tmpdir), "%s/pbsd_b0169_XXXXXX", base);
	td = mkdtemp(g_tmpdir);
	if (td == nullptr) {
		std::perror("mkdtemp");
		std::exit(2);
	}
	std::snprintf(g_subdir, sizeof(g_subdir), "%s/sub", g_tmpdir);
	if (mkdir(g_subdir, 0755) != 0) {
		std::perror("mkdir sub");
		std::exit(2);
	}
	std::snprintf(g_tmpfile, sizeof(g_tmpfile), "%s/file", g_tmpdir);
	int fd = open(g_tmpfile, O_CREAT | O_RDWR | O_TRUNC, 0644);
	if (fd < 0) {
		std::perror("open tmpfile");
		std::exit(2);
	}
	close(fd);
}

void
test_main_hand(void)
{
	static char prog[] = "realpath";
	static char empty[] = "";
	static char dot[] = ".";
	static char slash[] = "/";
	static char qflag[] = "-q";
	static char badopt[] = "-x";
	static char badopt2[] = "-?";
	static char hi[] = "\x80\xff\xfe";
	static char nul0[] = "\0tail";
	static char single[] = "a";
	static char longpath[PATH_MAX];

	char noexist[PATH_MAX];
	std::snprintf(noexist, sizeof(noexist), "%s/no_such_path_b0169", g_tmpdir);

	std::snprintf(longpath, sizeof(longpath), "%s/", g_tmpdir);
	for (int i = 0; i < 200; i++) {
		std::strcat(longpath, "d/");
	}
	std::strcat(longpath, "deep");

	char *av0[] = { nullptr };
	run_main_case("argc0_argv_null", 0, av0);

	char *av1[] = { prog, nullptr };
	run_main_case("argc1_prog_only", 1, av1);

	char *av2[] = { prog, dot, nullptr };
	run_main_case("argc2_dot", 2, av2);

	char *av3[] = { prog, slash, nullptr };
	run_main_case("argc2_slash", 2, av3);

	char *av4[] = { prog, g_tmpdir, nullptr };
	run_main_case("argc2_tmpdir", 2, av4);

	char *av5[] = { prog, g_tmpfile, nullptr };
	run_main_case("argc2_tmpfile", 2, av5);

	char *av6[] = { prog, g_subdir, nullptr };
	run_main_case("argc2_subdir", 2, av6);

	char *av7[] = { prog, noexist, nullptr };
	run_main_case("argc2_missing", 2, av7);

	char *av8[] = { prog, qflag, noexist, nullptr };
	run_main_case("argc3_q_missing", 3, av8);

	char *av9[] = { prog, qflag, nullptr };
	run_main_case("argc2_q_only", 2, av9);

	char *av10[] = { prog, qflag, dot, nullptr };
	run_main_case("argc3_q_dot", 3, av10);

	char *av11[] = { prog, badopt, nullptr };
	run_main_case("argc2_badopt_x", 2, av11);

	char *av12[] = { prog, badopt2, nullptr };
	run_main_case("argc2_badopt_qmark", 2, av12);

	char *av13[] = { prog, empty, nullptr };
	run_main_case("argc2_empty_path", 2, av13);

	char *av14[] = { prog, hi, nullptr };
	run_main_case("argc2_high_bit_path", 2, av14);

	char *av15[] = { prog, nul0, nullptr };
	run_main_case("argc2_nul_lead", 2, av15);

	char *av16[] = { prog, single, nullptr };
	run_main_case("argc2_single_char", 2, av16);

	char *av17[] = { prog, dot, g_tmpdir, nullptr };
	run_main_case("argc3_two_ok", 3, av17);

	char *av18[] = { prog, dot, noexist, nullptr };
	run_main_case("argc3_ok_then_fail", 3, av18);

	char *av19[] = { prog, noexist, dot, nullptr };
	run_main_case("argc3_fail_then_ok", 3, av19);

	char *av20[] = { prog, qflag, dot, noexist, nullptr };
	run_main_case("argc4_q_mixed", 4, av20);

	char *av21[] = { hi, dot, nullptr };
	run_main_case("argc2_hi_progname", 2, av21);

	char *av22[] = { prog, longpath, nullptr };
	run_main_case("argc2_long_missing", 2, av22);

	char *av23[] = { prog, qflag, g_tmpfile, g_subdir, dot, nullptr };
	run_main_case("argc5_q_multi_ok", 5, av23);
}

void
test_usage_hand(void)
{
	run_usage_case("direct");
	run_usage_case("direct_repeat");
}

std::vector<char>
rnd_string(std::size_t len)
{
	std::vector<char> s(len + 1, '\0');
	for (std::size_t i = 0; i < len; i++) {
		s[i] = (char)(rng.next() & 0xffu);
	}
	return s;
}

const char *
pick_known_path(std::uint64_t r)
{
	switch (r % 5u) {
	case 0:
		return ".";
	case 1:
		return g_tmpdir;
	case 2:
		return g_tmpfile;
	case 3:
		return g_subdir;
	default:
		return "/";
	}
}

void
test_main_sweep(void)
{
	std::vector<std::vector<char>> pool;
	pool.reserve(64);

	char noexist_base[PATH_MAX];
	std::snprintf(noexist_base, sizeof(noexist_base),
	    "%s/rnd_missing_", g_tmpdir);

	for (long i = 0; i < SWEEP; i++) {
		std::size_t argc = (std::size_t)(rng.next() % 12u);
		std::vector<char *> argv(argc + 1, nullptr);
		std::size_t idx = 0;

		pool.push_back(rnd_string((std::size_t)(rng.next() % 32u)));
		argv[idx++] = pool.back().data();

		bool use_q = (rng.next() & 1u) != 0;
		bool badopt = (rng.next() % 97u) == 0;
		if (badopt) {
			pool.push_back(rnd_string(1 + (rng.next() % 8u)));
			char *p = pool.back().data();
			p[0] = '-';
			if (p[1] != '\0' && p[1] != 'q') {
				/* leave non-q option */
			} else if (p[1] != '\0') {
				p[1] = (char)('a' + (rng.next() % 25u));
			}
			if (idx < argc) {
				argv[idx++] = p;
			}
		} else if (use_q && idx < argc) {
			static char q[] = "-q";
			argv[idx++] = q;
		}

		std::size_t npaths = (std::size_t)(rng.next() % 6u);
		for (std::size_t j = 0; j < npaths && idx < argc; j++) {
			if ((rng.next() & 3u) == 0u) {
				pool.push_back(rnd_string(
				    (std::size_t)(rng.next() % 128u)));
				argv[idx++] = pool.back().data();
			} else if ((rng.next() & 1u) == 0u) {
				pool.push_back(rnd_string(16));
				std::snprintf(pool.back().data(), 17, "%s%lu",
				    noexist_base, (unsigned long)(rng.next()));
				argv[idx++] = pool.back().data();
			} else {
				argv[idx++] =
				    const_cast<char *>(pick_known_path(rng.next()));
			}
		}

		run_main_case("sweep", (int)idx, argv.data());

		if (pool.size() > 128) {
			pool.clear();
		}
	}
}

void
test_usage_sweep(void)
{
	for (long i = 0; i < SWEEP; i++) {
		(void)i;
		run_usage_case("sweep");
	}
}

} // namespace

int
main()
{
	setup_tempdir();

	std::printf("PBSD batch b0169 differential test\n");
	std::printf("(oracle.c is the specification)\n\n");

	test_usage_hand();
	test_main_hand();
	test_usage_sweep();
	test_main_sweep();

	std::printf("%-32s %12s %12s %10s\n", "function", "cases",
	    "failures", "result");
	std::printf("%-32s %12s %12s %10s\n",
	    "--------------------------------", "------------",
	    "------------", "----------");

	long total_cases = 0;
	long total_fails = 0;

	for (Stat *st : { &st_main, &st_usage }) {
		total_cases += st->cases;
		total_fails += st->fails;
		std::printf("%-32s %12ld %12ld %10s\n", st->name, st->cases,
		    st->fails, st->fails == 0 ? "PASS" : "FAIL");
	}

	std::printf("%-32s %12s %12s %10s\n",
	    "--------------------------------", "------------",
	    "------------", "----------");
	std::printf("%-32s %12ld %12ld %10s\n", "TOTAL", total_cases,
	    total_fails, total_fails == 0 ? "PASS" : "FAIL");

	std::printf("\n%ld case(s) run, %ld failure(s).\n", total_cases,
	    total_fails);

	return total_fails == 0 ? 0 : 1;
}
