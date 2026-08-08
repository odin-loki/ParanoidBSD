/*
 * harness.cpp -- differential test for PBSD batch b0177 (kill.c).
 */

#define _GNU_SOURCE

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <signal.h>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

import pbsd.bin.kill.b0177;

namespace P = pbsd::bin_kill::b0177;

extern "C" {
int ref_main(int argc, char *argv[]);
void ref_nosig(const char *name);
void ref_printsignals(FILE *fp);
void ref_usage(void);
int __real_kill(pid_t pid, int sig);
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

Rng rng(0xb0177faceULL);

Stat st_main = { "main", 0, 0, 0 };
Stat st_nosig = { "nosig", 0, 0, 0 };
Stat st_printsignals = { "printsignals", 0, 0, 0 };
Stat st_usage = { "usage", 0, 0, 0 };

struct IoCapture {
	int saved;
	int pipefd[2];
	std::vector<unsigned char> bytes;

	bool begin(int fd)
	{
		saved = dup(fd);
		if (saved < 0)
			return false;
		if (pipe(pipefd) != 0) {
			close(saved);
			return false;
		}
		if (dup2(pipefd[1], fd) < 0) {
			close(pipefd[0]);
			close(pipefd[1]);
			close(saved);
			return false;
		}
		close(pipefd[1]);
		pipefd[1] = -1;
		return true;
	}

	std::vector<unsigned char>
	end(int fd)
	{
		unsigned char buf[4096];
		ssize_t nr;

		std::fflush(fd == STDOUT_FILENO ? stdout : stderr);
		if (pipefd[1] >= 0)
			close(pipefd[1]);
		while ((nr = read(pipefd[0], buf, sizeof(buf))) > 0)
			bytes.insert(bytes.end(), buf, buf + nr);
		close(pipefd[0]);
		dup2(saved, fd);
		close(saved);
		return bytes;
	}
};

struct ExitRun {
	int status;
	std::vector<unsigned char> stdout_bytes;
	std::vector<unsigned char> stderr_bytes;
};

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
same_bytes(const std::vector<unsigned char> &a,
    const std::vector<unsigned char> &b)
{
	return a == b;
}

extern "C" int
__wrap_kill(pid_t pid, int sig)
{
	(void)sig;
	if (pid > 0 && (pid % 5) == 1)
		return (0);
	if (pid == (pid_t)getpid() && sig == 0)
		return (0);
	errno = ESRCH;
	return (-1);
}

ExitRun
run_ref_main(int argc, char **argv)
{
	ExitRun res{};
	int pipe_out[2];
	int pipe_err[2];

	if (pipe(pipe_out) != 0 || pipe(pipe_err) != 0)
		return res;
	pid_t pid = fork();
	if (pid < 0)
		return res;
	if (pid == 0) {
		dup2(pipe_out[1], STDOUT_FILENO);
		dup2(pipe_err[1], STDERR_FILENO);
		close(pipe_out[0]);
		close(pipe_out[1]);
		close(pipe_err[0]);
		close(pipe_err[1]);
		int ret = ref_main(argc, argv);
		_exit(ret);
	}
	close(pipe_out[1]);
	close(pipe_err[1]);
	unsigned char buf[4096];
	ssize_t nr;
	while ((nr = read(pipe_out[0], buf, sizeof(buf))) > 0)
		res.stdout_bytes.insert(res.stdout_bytes.end(), buf, buf + nr);
	while ((nr = read(pipe_err[0], buf, sizeof(buf))) > 0)
		res.stderr_bytes.insert(res.stderr_bytes.end(), buf, buf + nr);
	close(pipe_out[0]);
	close(pipe_err[0]);
	int st = 0;
	if (waitpid(pid, &st, 0) >= 0 && WIFEXITED(st))
		res.status = WEXITSTATUS(st);
	return res;
}

ExitRun
run_port_main(int argc, char **argv)
{
	ExitRun res{};
	int pipe_out[2];
	int pipe_err[2];

	if (pipe(pipe_out) != 0 || pipe(pipe_err) != 0)
		return res;
	pid_t pid = fork();
	if (pid < 0)
		return res;
	if (pid == 0) {
		dup2(pipe_out[1], STDOUT_FILENO);
		dup2(pipe_err[1], STDERR_FILENO);
		close(pipe_out[0]);
		close(pipe_out[1]);
		close(pipe_err[0]);
		close(pipe_err[1]);
		int ret = P::main(argc, argv);
		_exit(ret);
	}
	close(pipe_out[1]);
	close(pipe_err[1]);
	unsigned char buf[4096];
	ssize_t nr;
	while ((nr = read(pipe_out[0], buf, sizeof(buf))) > 0)
		res.stdout_bytes.insert(res.stdout_bytes.end(), buf, buf + nr);
	while ((nr = read(pipe_err[0], buf, sizeof(buf))) > 0)
		res.stderr_bytes.insert(res.stderr_bytes.end(), buf, buf + nr);
	close(pipe_out[0]);
	close(pipe_err[0]);
	int st = 0;
	if (waitpid(pid, &st, 0) >= 0 && WIFEXITED(st))
		res.status = WEXITSTATUS(st);
	return res;
}

bool
check_main(const char *label, int argc, char **argv)
{
	st_main.cases++;
	ExitRun r = run_ref_main(argc, argv);
	ExitRun p = run_port_main(argc, argv);
	if (r.status != p.status) {
		std::printf("  [%s] status ref=%d port=%d\n", label, r.status,
		    p.status);
		return fail(st_main, label);
	}
	if (!same_bytes(r.stdout_bytes, p.stdout_bytes))
		return fail(st_main, label);
	if (!same_bytes(r.stderr_bytes, p.stderr_bytes))
		return fail(st_main, label);
	return true;
}

std::vector<unsigned char>
capture_ref_printsignals()
{
	std::vector<unsigned char> out;
	int pipefd[2];

	if (pipe(pipefd) != 0)
		return out;
	FILE *fp = fdopen(pipefd[1], "w");
	if (fp == nullptr) {
		close(pipefd[0]);
		close(pipefd[1]);
		return out;
	}
	ref_printsignals(fp);
	fclose(fp);
	unsigned char buf[4096];
	ssize_t nr;
	while ((nr = read(pipefd[0], buf, sizeof(buf))) > 0)
		out.insert(out.end(), buf, buf + nr);
	close(pipefd[0]);
	return out;
}

std::vector<unsigned char>
capture_port_printsignals()
{
	std::vector<unsigned char> out;
	int pipefd[2];

	if (pipe(pipefd) != 0)
		return out;
	FILE *fp = fdopen(pipefd[1], "w");
	if (fp == nullptr) {
		close(pipefd[0]);
		close(pipefd[1]);
		return out;
	}
	P::printsignals(fp);
	fclose(fp);
	unsigned char buf[4096];
	ssize_t nr;
	while ((nr = read(pipefd[0], buf, sizeof(buf))) > 0)
		out.insert(out.end(), buf, buf + nr);
	close(pipefd[0]);
	return out;
}

bool
check_printsignals(const char *label)
{
	st_printsignals.cases++;
	auto r = capture_ref_printsignals();
	auto p = capture_port_printsignals();
	if (!same_bytes(r, p))
		return fail(st_printsignals, label);
	return true;
}

ExitRun
run_ref_nosig(const char *name)
{
	ExitRun res{};
	int pipe_out[2];
	int pipe_err[2];

	if (pipe(pipe_out) != 0 || pipe(pipe_err) != 0)
		return res;
	pid_t pid = fork();
	if (pid < 0)
		return res;
	if (pid == 0) {
		dup2(pipe_out[1], STDOUT_FILENO);
		dup2(pipe_err[1], STDERR_FILENO);
		close(pipe_out[0]);
		close(pipe_out[1]);
		close(pipe_err[0]);
		close(pipe_err[1]);
		ref_nosig(name);
		_exit(99);
	}
	close(pipe_out[1]);
	close(pipe_err[1]);
	unsigned char buf[4096];
	ssize_t nr;
	while ((nr = read(pipe_out[0], buf, sizeof(buf))) > 0)
		res.stdout_bytes.insert(res.stdout_bytes.end(), buf, buf + nr);
	while ((nr = read(pipe_err[0], buf, sizeof(buf))) > 0)
		res.stderr_bytes.insert(res.stderr_bytes.end(), buf, buf + nr);
	close(pipe_out[0]);
	close(pipe_err[0]);
	int st = 0;
	if (waitpid(pid, &st, 0) >= 0 && WIFEXITED(st))
		res.status = WEXITSTATUS(st);
	return res;
}

ExitRun
run_port_nosig(const char *name)
{
	ExitRun res{};
	int pipe_out[2];
	int pipe_err[2];

	if (pipe(pipe_out) != 0 || pipe(pipe_err) != 0)
		return res;
	pid_t pid = fork();
	if (pid < 0)
		return res;
	if (pid == 0) {
		dup2(pipe_out[1], STDOUT_FILENO);
		dup2(pipe_err[1], STDERR_FILENO);
		close(pipe_out[0]);
		close(pipe_out[1]);
		close(pipe_err[0]);
		close(pipe_err[1]);
		P::nosig(name);
		_exit(99);
	}
	close(pipe_out[1]);
	close(pipe_err[1]);
	unsigned char buf[4096];
	ssize_t nr;
	while ((nr = read(pipe_out[0], buf, sizeof(buf))) > 0)
		res.stdout_bytes.insert(res.stdout_bytes.end(), buf, buf + nr);
	while ((nr = read(pipe_err[0], buf, sizeof(buf))) > 0)
		res.stderr_bytes.insert(res.stderr_bytes.end(), buf, buf + nr);
	close(pipe_out[0]);
	close(pipe_err[0]);
	int st = 0;
	if (waitpid(pid, &st, 0) >= 0 && WIFEXITED(st))
		res.status = WEXITSTATUS(st);
	return res;
}

bool
check_nosig(const char *label, const char *name)
{
	st_nosig.cases++;
	ExitRun r = run_ref_nosig(name);
	ExitRun p = run_port_nosig(name);
	if (r.status != p.status)
		return fail(st_nosig, label);
	if (!same_bytes(r.stdout_bytes, p.stdout_bytes))
		return fail(st_nosig, label);
	if (!same_bytes(r.stderr_bytes, p.stderr_bytes))
		return fail(st_nosig, label);
	return true;
}

ExitRun
run_ref_usage()
{
	ExitRun res{};
	int pipe_out[2];
	int pipe_err[2];

	if (pipe(pipe_out) != 0 || pipe(pipe_err) != 0)
		return res;
	pid_t pid = fork();
	if (pid < 0)
		return res;
	if (pid == 0) {
		dup2(pipe_out[1], STDOUT_FILENO);
		dup2(pipe_err[1], STDERR_FILENO);
		close(pipe_out[0]);
		close(pipe_out[1]);
		close(pipe_err[0]);
		close(pipe_err[1]);
		ref_usage();
		_exit(99);
	}
	close(pipe_out[1]);
	close(pipe_err[1]);
	unsigned char buf[4096];
	ssize_t nr;
	while ((nr = read(pipe_out[0], buf, sizeof(buf))) > 0)
		res.stdout_bytes.insert(res.stdout_bytes.end(), buf, buf + nr);
	while ((nr = read(pipe_err[0], buf, sizeof(buf))) > 0)
		res.stderr_bytes.insert(res.stderr_bytes.end(), buf, buf + nr);
	close(pipe_out[0]);
	close(pipe_err[0]);
	int st = 0;
	if (waitpid(pid, &st, 0) >= 0 && WIFEXITED(st))
		res.status = WEXITSTATUS(st);
	return res;
}

ExitRun
run_port_usage()
{
	ExitRun res{};
	int pipe_out[2];
	int pipe_err[2];

	if (pipe(pipe_out) != 0 || pipe(pipe_err) != 0)
		return res;
	pid_t pid = fork();
	if (pid < 0)
		return res;
	if (pid == 0) {
		dup2(pipe_out[1], STDOUT_FILENO);
		dup2(pipe_err[1], STDERR_FILENO);
		close(pipe_out[0]);
		close(pipe_out[1]);
		close(pipe_err[0]);
		close(pipe_err[1]);
		P::usage();
		_exit(99);
	}
	close(pipe_out[1]);
	close(pipe_err[1]);
	unsigned char buf[4096];
	ssize_t nr;
	while ((nr = read(pipe_out[0], buf, sizeof(buf))) > 0)
		res.stdout_bytes.insert(res.stdout_bytes.end(), buf, buf + nr);
	while ((nr = read(pipe_err[0], buf, sizeof(buf))) > 0)
		res.stderr_bytes.insert(res.stderr_bytes.end(), buf, buf + nr);
	close(pipe_out[0]);
	close(pipe_err[0]);
	int st = 0;
	if (waitpid(pid, &st, 0) >= 0 && WIFEXITED(st))
		res.status = WEXITSTATUS(st);
	return res;
}

bool
check_usage(const char *label)
{
	st_usage.cases++;
	ExitRun r = run_ref_usage();
	ExitRun p = run_port_usage();
	if (r.status != p.status)
		return fail(st_usage, label);
	if (!same_bytes(r.stdout_bytes, p.stdout_bytes))
		return fail(st_usage, label);
	if (!same_bytes(r.stderr_bytes, p.stderr_bytes))
		return fail(st_usage, label);
	return true;
}

void
test_main_edge()
{
	static char prog[] = "kill";
	static char dash_l[] = "-l";
	static char nine[] = "9";
	static char fifteen[] = "15";
	static char one37[] = "137";
	static char one28[] = "128";
	static char badnum[] = "9x";
	static char notdig[] = "-5";
	static char extra[] = "1";
	static char dash_s[] = "-s";
	static char hup[] = "HUP";
	static char zero[] = "0";
	static char bogus[] = "BOGUS";
	static char dash_hup[] = "-HUP";
	static char dash0[] = "-0";
	static char dash9[] = "-9";
	static char dash_dash[] = "--";
	static char pid1[] = "1";
	static char pid6[] = "6";
	static char badpid[] = "abc";
	static char pid1x[] = "1x";
	static char term[] = "TERM";
	static char overflow[] = "9223372036854775808";
	static char hi_sig[] = "\xff\xff";

	char *av_usage[] = { prog, nullptr };
	check_main("no_args", 1, av_usage);

	char *av_list[] = { prog, dash_l, nullptr };
	check_main("list_all", 2, av_list);

	char *av_list9[] = { prog, dash_l, nine, nullptr };
	check_main("list_9", 3, av_list9);

	char *av_list15[] = { prog, dash_l, fifteen, nullptr };
	check_main("list_15", 3, av_list15);

	char *av_list137[] = { prog, dash_l, one37, nullptr };
	check_main("list_137", 3, av_list137);

	char *av_list128[] = { prog, dash_l, one28, nullptr };
	check_main("list_128_nosig", 3, av_list128);

	char *av_list_bad[] = { prog, dash_l, badnum, nullptr };
	check_main("list_badnum", 3, av_list_bad);

	char *av_list_notdig[] = { prog, dash_l, notdig, nullptr };
	check_main("list_notdigit", 3, av_list_notdig);

	char *av_list_extra[] = { prog, dash_l, nine, extra, nullptr };
	check_main("list_extra_arg", 4, av_list_extra);

	char *av_s_hup[] = { prog, dash_s, hup, pid1, nullptr };
	check_main("s_hup_ok", 4, av_s_hup);

	char *av_s_zero[] = { prog, dash_s, zero, pid1, nullptr };
	check_main("s_zero_ok", 4, av_s_zero);

	char *av_s_bogus[] = { prog, dash_s, bogus, pid1, nullptr };
	check_main("s_bogus_nosig", 4, av_s_bogus);

	char *av_s_missing[] = { prog, dash_s, nullptr };
	check_main("s_missing_arg", 2, av_s_missing);

	char *av_short_hup[] = { prog, dash_hup, pid1, nullptr };
	check_main("short_hup", 3, av_short_hup);

	char *av_short0[] = { prog, dash0, pid6, nullptr };
	check_main("short_0_fail", 3, av_short0);

	char *av_short9[] = { prog, dash9, pid1, nullptr };
	check_main("short_9", 3, av_short9);

	char *av_dd[] = { prog, dash_dash, pid1, nullptr };
	check_main("double_dash", 3, av_dd);

	char *av_two[] = { prog, pid1, pid6, nullptr };
	check_main("two_pids_mixed", 3, av_two);

	char *av_bad[] = { prog, badpid, nullptr };
	check_main("bad_pid", 2, av_bad);

	char *av_pid1x[] = { prog, pid1x, nullptr };
	check_main("pid_suffix", 2, av_pid1x);

	char *av_only_s[] = { prog, dash_s, term, nullptr };
	check_main("s_no_pid", 3, av_only_s);

	char *av_overflow[] = { prog, overflow, nullptr };
	check_main("pid_overflow", 2, av_overflow);

	char *av_hi[] = { prog, dash_s, hi_sig, pid1, nullptr };
	check_main("highbit_signal", 4, av_hi);

	char *av_plain[] = { prog, pid6, nullptr };
	check_main("default_term_fail", 2, av_plain);

	char *av_plain_ok[] = { prog, pid1, nullptr };
	check_main("default_term_ok", 2, av_plain_ok);
}

std::string
rand_signal_token(Rng &r)
{
	static const char *names[] = {
		"HUP", "INT", "KILL", "TERM", "USR1", "0", "9", "15", "137",
		"BOGUS", "SIGTERM", "RTMIN", "RTMAX", "\xff\xfe", ""
	};
	int pick = r.bits(0, (int)(sizeof(names) / sizeof(names[0])) - 1);
	if (pick == (int)(sizeof(names) / sizeof(names[0])) - 1) {
		std::string s;
		int len = r.bits(0, 8);
		for (int i = 0; i < len; i++)
			s.push_back((char)r.byte());
		return s;
	}
	return names[pick];
}

std::string
rand_pid_token(Rng &r)
{
	switch (r.bits(0, 5)) {
	case 0:
		return "";
	case 1:
		return std::to_string(r.bits(1, 200000));
	case 2:
		return "abc";
	case 3:
		return std::to_string(r.bits(1, 9)) + "x";
	case 4:
		return "9223372036854775808";
	default: {
		std::string s;
		int len = r.bits(1, 12);
		for (int i = 0; i < len; i++)
			s.push_back((char)('0' + r.bits(0, 9)));
		return s;
	}
	}
}

void
test_main_random()
{
	static char prog[] = "kill";
	static char dash_l[] = "-l";
	static char dash_s[] = "-s";
	static char dash_dash[] = "--";

	std::vector<std::string> pool;
	pool.reserve(32);

	for (long i = 0; i < SWEEP; i++) {
		std::vector<char *> argv;
		argv.push_back(prog);
		int mode = rng.bits(0, 7);

		switch (mode) {
		case 0:
			argv.push_back(dash_l);
			if (rng.coin()) {
				pool.push_back(rand_signal_token(rng));
				argv.push_back(const_cast<char *>(pool.back().c_str()));
			}
			break;
		case 1:
			argv.push_back(dash_s);
			pool.push_back(rand_signal_token(rng));
			argv.push_back(const_cast<char *>(pool.back().c_str()));
			break;
		case 2: {
			std::string sig = "-" + rand_signal_token(rng);
			pool.push_back(sig);
			argv.push_back(const_cast<char *>(pool.back().c_str()));
			break;
		}
		case 3:
			if (rng.coin()) {
				argv.push_back(dash_dash);
			}
			break;
		default:
			break;
		}

		int np = rng.bits(0, 4);
		for (int j = 0; j < np; j++) {
			pool.push_back(rand_pid_token(rng));
			argv.push_back(const_cast<char *>(pool.back().c_str()));
		}

		argv.push_back(nullptr);
		check_main("sweep", (int)argv.size() - 1, argv.data());

		if (pool.size() > 32)
			pool.clear();
	}
}

void
test_printsignals_edge()
{
	(void)check_printsignals("stdout");
}

void
test_printsignals_random()
{
	for (long i = 0; i < SWEEP; i++)
		(void)check_printsignals("sweep");
}

void
test_nosig_edge()
{
	(void)check_nosig("bogus", "BOGUS");
	(void)check_nosig("empty", "");
	(void)check_nosig("single", "a");
	(void)check_nosig("highbit", "\x80\xff");
	(void)check_nosig("nul_heavy", "\0a\0b");
	(void)check_nosig("digit", "9");
	(void)check_nosig("rtmin", "RTMIN");
}

void
test_nosig_random()
{
	std::vector<std::string> pool;
	pool.reserve(32);

	for (long i = 0; i < SWEEP; i++) {
		int len = rng.bits(0, 16);
		std::string s;
		for (int j = 0; j < len; j++)
			s.push_back((char)rng.byte());
		pool.push_back(s);
		check_nosig("sweep", pool.back().c_str());
		if (pool.size() > 32)
			pool.clear();
	}
}

void
test_usage_edge()
{
	(void)check_usage("usage");
}

void
test_usage_random()
{
	for (long i = 0; i < SWEEP; i++)
		(void)check_usage("sweep");
}

} // namespace

int
main()
{
	test_printsignals_edge();
	test_nosig_edge();
	test_usage_edge();
	test_main_edge();

	test_printsignals_random();
	test_nosig_random();
	test_usage_random();
	test_main_random();

	Stat *all[] = { &st_printsignals, &st_nosig, &st_usage, &st_main };
	long total_cases = 0;
	long total_fails = 0;

	std::printf("\n%-14s %12s %12s\n", "function", "cases", "failures");
	for (Stat *st : all) {
		std::printf("%-14s %12ld %12ld\n", st->name, st->cases,
		    st->fails);
		total_cases += st->cases;
		total_fails += st->fails;
	}
	std::printf("%-14s %12ld %12ld\n", "TOTAL", total_cases, total_fails);

	return total_fails == 0 ? 0 : 1;
}
