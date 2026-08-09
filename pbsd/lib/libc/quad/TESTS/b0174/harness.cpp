/*
 * Differential test for PBSD batch b0174 (lib/libc/quad/TESTS).
 *
 * Each interactive main is exercised in a child process with redirected
 * stdin/stdout.  Port and oracle must produce identical output and exit
 * status for every input transcript.
 */

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

import pbsd.lib.libc.quad.tests.b0174;

extern "C" int ref_mul_main(void);
extern "C" int ref_divrem_main(void);

namespace P = pbsd::lib_libc_quad_tests::b0174;

namespace {

constexpr int kMaxReport = 8;
constexpr long kRandIters = 200000;

using u64 = unsigned long long;

struct Stat {
	const char *name;
	u64 cases;
	u64 fails;
	int reported;
};

Stat st_mul = { "mul_main", 0, 0, 0 };
Stat st_divrem = { "divrem_main", 0, 0, 0 };

u64 rng_state = 0x0123456789abcdefULL;

u64
nextrand(void)
{
	u64 x = rng_state;

	x ^= x >> 12;
	x ^= x << 25;
	x ^= x >> 27;
	rng_state = x;
	return (x * 0x2545f4914f6cdd1dULL);
}

struct RunOutcome {
	int wait_status;
	std::string output;
};

using MainFn = void (*)();

RunOutcome
run_in_child(MainFn fn, const char *input, std::size_t input_len)
{
	int inpipe[2];
	int outpipe[2];
	pid_t pid;

	if (pipe(inpipe) != 0 || pipe(outpipe) != 0)
		std::abort();

	pid = fork();
	if (pid < 0)
		std::abort();

	if (pid == 0) {
		(void)dup2(inpipe[0], STDIN_FILENO);
		(void)dup2(outpipe[1], STDOUT_FILENO);
		close(inpipe[0]);
		close(inpipe[1]);
		close(outpipe[0]);
		close(outpipe[1]);
		fn();
		_exit(99);
	}

	close(inpipe[0]);
	close(outpipe[1]);

	if (input_len > 0) {
		const char *p = input;
		std::size_t left = input_len;
		while (left > 0) {
			ssize_t n = write(inpipe[1], p, left);
			if (n < 0) {
				if (errno == EINTR)
					continue;
				break;
			}
			p += (std::size_t)n;
			left -= (std::size_t)n;
		}
	}
	close(inpipe[1]);

	RunOutcome out;
	char chunk[4096];
	for (;;) {
		ssize_t n = read(outpipe[0], chunk, sizeof chunk);
		if (n < 0) {
			if (errno == EINTR)
				continue;
			break;
		}
		if (n == 0)
			break;
		out.output.append(chunk, (std::size_t)n);
	}
	close(outpipe[0]);

	if (waitpid(pid, &out.wait_status, 0) < 0)
		std::abort();
	return (out);
}

bool
status_eq(int a, int b)
{
	if (WIFEXITED(a) && WIFEXITED(b))
		return (WEXITSTATUS(a) == WEXITSTATUS(b));
	if (WIFSIGNALED(a) && WIFSIGNALED(b))
		return (WTERMSIG(a) == WTERMSIG(b));
	return (false);
}

void
fail_msg(Stat &s, const char *detail)
{
	s.fails++;
	if (s.reported < kMaxReport) {
		s.reported++;
		std::printf("  %s: MISMATCH %s\n", s.name, detail);
	}
}

void
check_pair(Stat &s, MainFn port_fn, MainFn ref_fn, const char *input,
    std::size_t input_len, u64 case_count)
{
	RunOutcome port = run_in_child(port_fn, input, input_len);
	RunOutcome ref = run_in_child(ref_fn, input, input_len);

	s.cases += case_count;
	if (port.output != ref.output || !status_eq(port.wait_status, ref.wait_status))
		fail_msg(s, "");
}

void
check_mul_transcript(const char *input, std::size_t input_len, u64 case_count)
{
	check_pair(st_mul, [] { P::mul_main(); }, [] { ref_mul_main(); },
	    input, input_len, case_count);
}

void
check_divrem_transcript(const char *input, std::size_t input_len,
    u64 case_count)
{
	check_pair(st_divrem, [] { P::divrem_main(); },
	    [] { ref_divrem_main(); }, input, input_len, case_count);
}

void
check_mul(const char *input)
{
	check_mul_transcript(input, std::strlen(input), 1);
}

void
check_divrem(const char *input)
{
	check_divrem_transcript(input, std::strlen(input), 1);
}

const unsigned long kWords[] = {
	0UL, 1UL, 2UL, 3UL, 0x7fUL, 0x80UL, 0xffUL,
	0x7fffUL, 0x8000UL, 0xffffUL,
	0x7fffffffUL, 0x80000000UL, 0xffffffffUL,
	0x55555555UL, 0xaaaaaaaaUL, 0x7f7f7f7fUL, 0x80808080UL,
	0xdeadbeefUL, 0x12345678UL,
};
const int kNWords = (int)(sizeof kWords / sizeof kWords[0]);

char line_buf[512];

void
fmt_decimal(unsigned long a0, unsigned long a1, unsigned long b0,
    unsigned long b1, char *out, std::size_t outsz)
{
	std::snprintf(out, outsz, "%lu:%lu %lu:%lu\n", a0, a1, b0, b1);
}

void
fmt_hex(unsigned long a0, unsigned long a1, unsigned long b0,
    unsigned long b1, char *out, std::size_t outsz)
{
	std::snprintf(out, outsz, "0x%lx:%lx 0x%lx:%lx\n", a0, a1, b0, b1);
}

void
run_hand_cases(void)
{
	check_mul("");
	check_mul("0:0 0:0\n");
	check_mul("1:0 1:0\n");
	check_mul("0:1 0:1\n");
	check_mul("0xffffffff:0xffffffff 0xffffffff:0xffffffff\n");
	check_mul("0x80000000:0 0x80000000:0\n");
	check_mul("0x0:0 0x0:0\n");
	check_mul("eh?\n");
	check_mul("foo\n");
	check_mul("1:2\n");
	check_mul("1:2 3\n");
	check_mul("1:2 3:4\n");
	check_mul("0x1:0x2 0x3:0x4\n");
	check_mul("0xdeadbeef:0xcafebabe 0x1:0x2\n");
	check_mul("bad\nstill bad\n0:0 0:0\n");
	check_mul("0:0 0:0\n0:1 0:1\n");

	check_divrem("");
	check_divrem("0:0 0:0\n");
	check_divrem("1:0 1:0\n");
	check_divrem("0:1 0:1\n");
	check_divrem("10:0 3:0\n");
	check_divrem("0xffffffff:0 0x2:0\n");
	check_divrem("0x80000000:0 0x80000000:0\n");
	check_divrem("eh?\n");
	check_divrem("foo\n");
	check_divrem("1:2 3:4\n");
	check_divrem("0x1:0x2 0x3:0x4\n");
	check_divrem("0xdeadbeef:0 0x10:0\n");
	check_divrem("bad\n0:1 0:2\n");
	check_divrem("0:0 0:1\n");
	check_divrem("0:0 0:2\n");
}

void
run_random_cases(void)
{
	std::string mul_batch;
	std::string div_batch;
	constexpr int kBatchLines = 250;
	int batch_lines = 0;
	long generated = 0;

	mul_batch.reserve(65536);
	div_batch.reserve(65536);

	while (generated < kRandIters) {
		unsigned long a0 = kWords[(int)(nextrand() % kNWords)];
		unsigned long a1 = kWords[(int)(nextrand() % kNWords)];
		unsigned long b0 = kWords[(int)(nextrand() % kNWords)];
		unsigned long b1 = kWords[(int)(nextrand() % kNWords)];

		a0 ^= (unsigned long)(nextrand() & 0xffffffffUL);
		a1 ^= (unsigned long)(nextrand() & 0xffffffffUL);
		b0 ^= (unsigned long)(nextrand() & 0xffffffffUL);
		b1 ^= (unsigned long)(nextrand() & 0xffffffffUL);

		if ((nextrand() & 1U) != 0U)
			fmt_decimal(a0, a1, b0, b1, line_buf, sizeof line_buf);
		else
			fmt_hex(a0, a1, b0, b1, line_buf, sizeof line_buf);

		mul_batch.append(line_buf);
		div_batch.append(line_buf);
		generated++;
		batch_lines++;

		if ((nextrand() & 0xffU) == 0U) {
			mul_batch.append("garbage\n");
			div_batch.append("garbage\n");
		}

		if (batch_lines >= kBatchLines || generated >= kRandIters) {
			check_mul_transcript(mul_batch.data(), mul_batch.size(),
			    (u64)batch_lines);
			check_divrem_transcript(div_batch.data(), div_batch.size(),
			    (u64)batch_lines);
			mul_batch.clear();
			div_batch.clear();
			batch_lines = 0;
		}
	}
}

void
print_stat(const Stat &s)
{
	std::printf("%-14s %8llu %8llu\n",
	    s.name, (unsigned long long)s.cases, (unsigned long long)s.fails);
}

} // namespace

int
main(void)
{
	run_hand_cases();
	run_random_cases();

	std::printf("\nfunction          cases   failures\n");
	print_stat(st_mul);
	print_stat(st_divrem);

	u64 fails = st_mul.fails + st_divrem.fails;
	return (fails == 0 ? 0 : 1);
}
