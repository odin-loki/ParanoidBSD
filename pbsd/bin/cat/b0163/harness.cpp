/*
 * harness.cpp -- differential test for PBSD batch b0163 (cat.c).
 */

#define _GNU_SOURCE

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <setjmp.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

import pbsd.bin.cat.b0163;

namespace port = pbsd::bin_cat::b0163;

extern "C" {
extern int rval;
extern const char *filename;
extern int oracle_err_called;
extern int oracle_err_status;
void oracle_err_arm(void);
void oracle_err_disarm(void);
extern jmp_buf oracle_err_jmp;
ssize_t ref_in_kernel_copy(int rfd);
void ref_raw_cat(int rfd);
}

namespace {

constexpr long RANDOM_ITERS = 200000;
constexpr unsigned char GUARD = 0x7f;

struct Stat {
	const char *name;
	long cases;
	long fails;
};

Stat stats[2];
int nstats = 0;

std::uint64_t rng = 0xb0163facefeedULL;

std::uint64_t
rnd()
{
	std::uint64_t z;

	rng += 0x9e3779b97f4a7c15ULL;
	z = rng;
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return z ^ (z >> 31);
}

unsigned char
rnd_byte()
{
	return (unsigned char)(rnd() & 0xffu);
}

Stat &
reg(const char *name)
{
	stats[nstats++] = {name, 0, 0};
	return stats[nstats - 1];
}

struct IoResult {
	ssize_t ret;
	int rval;
	int err_called;
	int err_status;
	std::vector<unsigned char> out;
};

int
save_stdout(int *saved)
{
	*saved = dup(STDOUT_FILENO);
	return *saved;
}

void
restore_stdout(int saved)
{
	fflush(stdout);
	dup2(saved, STDOUT_FILENO);
	close(saved);
}

bool
begin_stdout_capture(int saved_stdout, int cap_pipe[2])
{
	if (pipe(cap_pipe) != 0)
		return false;
	if (dup2(cap_pipe[1], STDOUT_FILENO) < 0)
		return false;
	close(cap_pipe[1]);
	cap_pipe[1] = -1;
	return true;
}

std::vector<unsigned char>
finish_stdout_capture(int saved_stdout, int cap_pipe[2])
{
	std::vector<unsigned char> out;
	unsigned char buf[8192];
	ssize_t nr;

	fflush(stdout);
	if (cap_pipe[1] >= 0)
		close(cap_pipe[1]);
	while ((nr = read(cap_pipe[0], buf, sizeof(buf))) > 0)
		out.insert(out.end(), buf, buf + nr);
	close(cap_pipe[0]);
	restore_stdout(saved_stdout);
	return out;
}

int
make_input_pipe(const unsigned char *data, std::size_t len)
{
	int fds[2];

	if (pipe(fds) != 0)
		return -1;
	if (len > 0 && write(fds[1], data, len) != (ssize_t)len) {
		close(fds[0]);
		close(fds[1]);
		return -1;
	}
	close(fds[1]);
	return fds[0];
}

IoResult
run_ref_raw_cat(int rfd)
{
	IoResult res{};
	int saved_stdout;
	int cap_pipe[2] = {-1, -1};

	rval = 0;
	filename = "oracle-input";
	oracle_err_called = 0;
	oracle_err_status = 0;
	if (save_stdout(&saved_stdout) < 0)
		return res;
	if (!begin_stdout_capture(saved_stdout, cap_pipe))
		return res;
	oracle_err_arm();
	if (setjmp(oracle_err_jmp) == 0)
		ref_raw_cat(rfd);
	else {
		res.err_called = 1;
		res.err_status = oracle_err_status;
	}
	oracle_err_disarm();
	res.rval = rval;
	res.out = finish_stdout_capture(saved_stdout, cap_pipe);
	res.err_called = oracle_err_called;
	res.err_status = oracle_err_status;
	return res;
}

IoResult
run_port_raw_cat(int rfd)
{
	IoResult res{};
	int saved_stdout;
	int cap_pipe[2] = {-1, -1};

	port::rval = 0;
	port::filename = "oracle-input";
	port::port_err_called = 0;
	port::port_err_status = 0;
	if (save_stdout(&saved_stdout) < 0)
		return res;
	if (!begin_stdout_capture(saved_stdout, cap_pipe))
		return res;
	port::port_err_arm();
	if (setjmp(port::port_err_jmp) == 0)
		port::raw_cat(rfd);
	else {
		res.err_called = 1;
		res.err_status = port::port_err_status;
	}
	port::port_err_disarm();
	res.rval = port::rval;
	res.out = finish_stdout_capture(saved_stdout, cap_pipe);
	res.err_called = port::port_err_called;
	res.err_status = port::port_err_status;
	return res;
}

bool
same(const IoResult &a, const IoResult &b)
{
	if (a.rval != b.rval || a.err_called != b.err_called ||
	    a.err_status != b.err_status || a.out.size() != b.out.size())
		return false;
	return a.out == b.out;
}

void
test_raw_cat_case(Stat &st, const unsigned char *data, std::size_t len,
    const char *tag)
{
	int rfd = make_input_pipe(data, len);
	IoResult ref_res;
	IoResult port_res;

	st.cases++;
	if (rfd < 0) {
		st.fails++;
		return;
	}
	ref_res = run_ref_raw_cat(rfd);
	close(rfd);
	rfd = make_input_pipe(data, len);
	if (rfd < 0) {
		st.fails++;
		return;
	}
	port_res = run_port_raw_cat(rfd);
	close(rfd);
	if (!same(ref_res, port_res))
		st.fails++;
	(void)tag;
}

void
test_raw_cat_read_error(Stat &st)
{
	int fds[2];

	st.cases++;
	if (pipe(fds) != 0) {
		st.fails++;
		return;
	}
	close(fds[0]);
	close(fds[1]);
	rval = 0;
	port::rval = 0;
	filename = "badfd";
	port::filename = "badfd";
	IoResult ref_res = run_ref_raw_cat(-1);
	IoResult port_res = run_port_raw_cat(-1);
	if (!same(ref_res, port_res))
		st.fails++;
}

void
test_raw_cat_regular_stdout_child(Stat &st, const unsigned char *data,
    std::size_t len)
{
	char inpath[] = "/tmp/pbsd_b0163_inXXXXXX";
	char outpath[] = "/tmp/pbsd_b0163_outXXXXXX";
	int infd;
	int outfd;
	pid_t pid;
	int status;
	unsigned char refbuf[1 << 20];
	unsigned char portbuf[1 << 20];
	ssize_t refnr, portnr;

	st.cases++;
	infd = mkstemp(inpath);
	outfd = mkstemp(outpath);
	if (infd < 0 || outfd < 0) {
		st.fails++;
		if (infd >= 0) {
			close(infd);
			unlink(inpath);
		}
		if (outfd >= 0) {
			close(outfd);
			unlink(outpath);
		}
		return;
	}
	if (len > 0 && write(infd, data, len) != (ssize_t)len) {
		st.fails++;
		close(infd);
		close(outfd);
		unlink(inpath);
		unlink(outpath);
		return;
	}
	lseek(infd, 0, SEEK_SET);
	pid = fork();
	if (pid < 0) {
		st.fails++;
		close(infd);
		close(outfd);
		unlink(inpath);
		unlink(outpath);
		return;
	}
	if (pid == 0) {
		IoResult res;

		dup2(outfd, STDOUT_FILENO);
		close(outfd);
		res = run_ref_raw_cat(infd);
		close(infd);
		_exit(res.err_called ? 100 : res.rval);
	}
	close(infd);
	waitpid(pid, &status, 0);
	lseek(outfd, 0, SEEK_SET);
	refnr = read(outfd, refbuf, sizeof(refbuf));
	ftruncate(outfd, 0);
	lseek(outfd, 0, SEEK_SET);

	pid = fork();
	if (pid < 0) {
		st.fails++;
		close(outfd);
		unlink(inpath);
		unlink(outpath);
		return;
	}
	if (pid == 0) {
		IoResult res;
		int rfd;

		rfd = open(inpath, O_RDONLY);
		dup2(outfd, STDOUT_FILENO);
		close(outfd);
		res = run_port_raw_cat(rfd);
		close(rfd);
		_exit(res.err_called ? 100 : res.rval);
	}
	close(outfd);
	waitpid(pid, &status, 0);
	outfd = open(outpath, O_RDONLY);
	portnr = read(outfd, portbuf, sizeof(portbuf));
	close(outfd);
	unlink(inpath);
	unlink(outpath);

	if (!WIFEXITED(status) || WEXITSTATUS(status) != 0 || refnr != portnr ||
	    (refnr > 0 && std::memcmp(refbuf, portbuf, (size_t)refnr) != 0))
		st.fails++;
}

void
test_raw_cat()
{
	Stat &st = reg("raw_cat");
	static const unsigned char empty[] = "";
	static const unsigned char one[] = {'a'};
	static const unsigned char nul[] = {'\0', 'x', '\0'};
	static const unsigned char high[] = {0x80, 0xff, 0x7f, 0x00};
	unsigned char block[131072];
	std::size_t sizes[] = {0, 1, 2, 3, 4095, 4096, 4097, 8191, 8192, 8193,
	    65535, 65536, 65537, 131071, 131072};

	for (std::size_t i = 0; i < sizeof(block); i++)
		block[i] = (unsigned char)(i & 0xff);

	test_raw_cat_case(st, empty, 0, "empty");
	test_raw_cat_case(st, one, 1, "one");
	test_raw_cat_case(st, nul, sizeof(nul), "nul");
	test_raw_cat_case(st, high, sizeof(high), "high");
	for (std::size_t sz : sizes)
		test_raw_cat_case(st, block, sz, "block");
	test_raw_cat_read_error(st);
	test_raw_cat_regular_stdout_child(st, block, 50000);
	test_raw_cat_regular_stdout_child(st, high, sizeof(high));

	for (long i = 0; i < RANDOM_ITERS; i++) {
		unsigned char buf[262144];
		std::size_t len = (std::size_t)(rnd() % (sizeof(buf) + 1));

		for (std::size_t j = 0; j < len; j++)
			buf[j] = rnd_byte();
		test_raw_cat_case(st, buf, len, "random");
	}
}

IoResult
run_ref_in_kernel_copy(int rfd)
{
	IoResult res{};
	int saved_stdout;
	int cap_pipe[2] = {-1, -1};

	rval = 0;
	filename = "oracle-input";
	oracle_err_called = 0;
	oracle_err_status = 0;
	if (save_stdout(&saved_stdout) < 0)
		return res;
	if (!begin_stdout_capture(saved_stdout, cap_pipe))
		return res;
	oracle_err_arm();
	if (setjmp(oracle_err_jmp) == 0)
		res.ret = ref_in_kernel_copy(rfd);
	else {
		res.err_called = 1;
		res.err_status = oracle_err_status;
	}
	oracle_err_disarm();
	res.rval = rval;
	res.out = finish_stdout_capture(saved_stdout, cap_pipe);
	res.err_called = oracle_err_called;
	res.err_status = oracle_err_status;
	return res;
}

IoResult
run_port_in_kernel_copy(int rfd)
{
	IoResult res{};
	int saved_stdout;
	int cap_pipe[2] = {-1, -1};

	port::rval = 0;
	port::filename = "oracle-input";
	port::port_err_called = 0;
	port::port_err_status = 0;
	if (save_stdout(&saved_stdout) < 0)
		return res;
	if (!begin_stdout_capture(saved_stdout, cap_pipe))
		return res;
	port::port_err_arm();
	if (setjmp(port::port_err_jmp) == 0)
		res.ret = port::in_kernel_copy(rfd);
	else {
		res.err_called = 1;
		res.err_status = port::port_err_status;
	}
	port::port_err_disarm();
	res.rval = port::rval;
	res.out = finish_stdout_capture(saved_stdout, cap_pipe);
	res.err_called = port::port_err_called;
	res.err_status = port::port_err_status;
	return res;
}

bool
same_copy(const IoResult &a, const IoResult &b)
{
	if (a.ret != b.ret || a.rval != b.rval || a.err_called != b.err_called ||
	    a.err_status != b.err_status || a.out.size() != b.out.size())
		return false;
	return a.out == b.out;
}

void
test_in_kernel_copy_case(Stat &st, const unsigned char *data, std::size_t len)
{
	char inpath[] = "/tmp/pbsd_b0163_kinXXXXXX";
	char outpath[] = "/tmp/pbsd_b0163_koutXXXXXX";
	int infd;
	int outfd;
	int routfd;
	IoResult ref_res;
	IoResult port_res;

	st.cases++;
	infd = mkstemp(inpath);
	outfd = mkstemp(outpath);
	if (infd < 0 || outfd < 0) {
		st.fails++;
		if (infd >= 0) {
			close(infd);
			unlink(inpath);
		}
		if (outfd >= 0) {
			close(outfd);
			unlink(outpath);
		}
		return;
	}
	if (len > 0 && write(infd, data, len) != (ssize_t)len) {
		st.fails++;
		close(infd);
		close(outfd);
		unlink(inpath);
		unlink(outpath);
		return;
	}
	lseek(infd, 0, SEEK_SET);
	routfd = dup(outfd);
	dup2(outfd, STDOUT_FILENO);
	ref_res = run_ref_in_kernel_copy(infd);
	close(infd);
	restore_stdout(routfd);
	close(routfd);

	infd = open(inpath, O_RDONLY);
	outfd = open(outpath, O_RDWR | O_TRUNC);
	routfd = dup(outfd);
	dup2(outfd, STDOUT_FILENO);
	port_res = run_port_in_kernel_copy(infd);
	close(infd);
	restore_stdout(routfd);
	close(routfd);
	unlink(inpath);
	unlink(outpath);

	if (!same_copy(ref_res, port_res))
		st.fails++;
}

void
test_in_kernel_copy_invalid_fd(Stat &st)
{
	int outfd;
	int routfd;
	IoResult ref_res;
	IoResult port_res;

	st.cases++;
	outfd = open("/tmp/pbsd_b0163_koutXXXXXX",
	    O_RDWR | O_CREAT | O_EXCL, 0600);
	if (outfd < 0) {
		st.fails++;
		return;
	}
	routfd = dup(outfd);
	dup2(outfd, STDOUT_FILENO);
	ref_res = run_ref_in_kernel_copy(-1);
	restore_stdout(routfd);
	close(routfd);
	close(outfd);
	unlink("/tmp/pbsd_b0163_koutXXXXXX");

	outfd = open("/tmp/pbsd_b0163_kout2XXXXXX",
	    O_RDWR | O_CREAT | O_EXCL, 0600);
	if (outfd < 0) {
		st.fails++;
		return;
	}
	routfd = dup(outfd);
	dup2(outfd, STDOUT_FILENO);
	port_res = run_port_in_kernel_copy(-1);
	restore_stdout(routfd);
	close(routfd);
	close(outfd);
	unlink("/tmp/pbsd_b0163_kout2XXXXXX");

	if (!same_copy(ref_res, port_res))
		st.fails++;
}

void
test_in_kernel_copy()
{
	Stat &st = reg("in_kernel_copy");
	static const unsigned char empty[] = "";
	static const unsigned char one[] = {'z'};
	static const unsigned char nul[] = {'\0', '\0', 'a'};
	static const unsigned char high[] = {0x80, 0xfe, 0xff, 0x01};
	unsigned char block[262144];
	std::size_t sizes[] = {0, 1, 2, 4095, 4096, 4097, 16384, 65536, 131072,
	    262144};

	for (std::size_t i = 0; i < sizeof(block); i++)
		block[i] = rnd_byte();

	test_in_kernel_copy_case(st, empty, 0);
	test_in_kernel_copy_case(st, one, 1);
	test_in_kernel_copy_case(st, nul, sizeof(nul));
	test_in_kernel_copy_case(st, high, sizeof(high));
	for (std::size_t sz : sizes)
		test_in_kernel_copy_case(st, block, sz);
	test_in_kernel_copy_invalid_fd(st);

	for (long i = 0; i < RANDOM_ITERS; i++) {
		unsigned char buf[65536];
		std::size_t len = (std::size_t)(rnd() % (sizeof(buf) + 1));

		for (std::size_t j = 0; j < len; j++)
			buf[j] = rnd_byte();
		test_in_kernel_copy_case(st, buf, len);
	}
}

} /* namespace */

int
main()
{
	test_raw_cat();
	test_in_kernel_copy();

	std::printf("PBSD batch b0163 differential test\n");
	std::printf("(oracle.c is the specification)\n\n");
	std::printf("%-32s %12s %12s %10s\n", "function", "cases",
	    "failures", "result");
	std::printf("%-32s %12s %12s %10s\n",
	    "--------------------------------", "------------",
	    "------------", "----------");

	long total_cases = 0;
	long total_fails = 0;
	for (int i = 0; i < nstats; i++) {
		total_cases += stats[i].cases;
		total_fails += stats[i].fails;
		std::printf("%-32s %12ld %12ld %10s\n", stats[i].name,
		    stats[i].cases, stats[i].fails,
		    stats[i].fails == 0 ? "PASS" : "FAIL");
	}
	std::printf("%-32s %12s %12s %10s\n",
	    "--------------------------------", "------------",
	    "------------", "----------");
	std::printf("%-32s %12ld %12ld %10s\n", "TOTAL", total_cases,
	    total_fails, total_fails == 0 ? "PASS" : "FAIL");
	std::printf("\n%ld function(s) tested, %ld case(s) run, "
	    "%ld failure(s).\n", (long)nstats, total_cases, total_fails);

	return total_fails == 0 ? 0 : 1;
}
