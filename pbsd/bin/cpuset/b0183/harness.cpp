/*
 * PBSD batch b0183 differential harness.
 *
 * Compares the C++23 port in port.cppm against the unmodified C oracle in
 * oracle.c for every function that was ported:
 *
 *	printset(struct bitset *, int)	- reads the mask, writes to stdout
 *	usage(void)			- writes to stderr, exits
 *
 * printset() gets hand written edge cases plus a fixed seed randomised sweep.
 * Both implementations run against their own copy of the input buffer, which
 * is surrounded by 0x7f guard bytes, and the ENTIRE buffer (guards included)
 * is compared after every call, so a port that scribbles anywhere is caught
 * even when its stdout output happens to match.
 *
 * usage() cannot return, so each side runs in a forked child with stderr on a
 * pipe; the captured text, the exit status and the terminating signal are all
 * compared.
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <cstddef>
#include <string>

#include <fcntl.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/wait.h>

import pbsd.bin.cpuset.b0183;

namespace P = pbsd::bin_cpuset::b0183;

struct ref_bitset {
	long	__bits[1];
};

extern "C" void ref_printset(struct ref_bitset *mask, int size);
extern "C" [[noreturn]] void ref_usage(void);

/* ------------------------------------------------------------------ */
/* stdout capture: fd 1 sits on a scratch file for the whole run.      */
/* ------------------------------------------------------------------ */

static int cap_fd = -1;
static int real_stdout = -1;
static char cap_iobuf[1 << 16];

static void
cap_setup(void)
{
	char tmpl[] = "/tmp/pbsd_b0183_XXXXXX";

	real_stdout = ::dup(1);
	if (real_stdout < 0) {
		std::perror("dup");
		std::exit(2);
	}
	cap_fd = ::mkstemp(tmpl);
	if (cap_fd < 0) {
		std::perror("mkstemp");
		std::exit(2);
	}
	::unlink(tmpl);
	std::fflush(stdout);
	if (::dup2(cap_fd, 1) < 0) {
		std::perror("dup2");
		std::exit(2);
	}
	std::setvbuf(stdout, cap_iobuf, _IOFBF, sizeof(cap_iobuf));
}

static void
cap_restore(void)
{

	std::fflush(stdout);
	::dup2(real_stdout, 1);
	std::setvbuf(stdout, nullptr, _IOLBF, 0);
}

static void
cap_begin(void)
{

	std::fflush(stdout);
	if (::lseek(1, 0, SEEK_SET) < 0) {
		std::perror("lseek");
		std::exit(2);
	}
}

static void
cap_end(std::string &out)
{
	off_t n, got;

	std::fflush(stdout);
	n = ::lseek(1, 0, SEEK_CUR);
	if (n < 0) {
		std::perror("lseek");
		std::exit(2);
	}
	out.assign(static_cast<std::size_t>(n), '\0');
	for (got = 0; got < n; ) {
		ssize_t r = ::pread(cap_fd, out.data() + got,
		    static_cast<std::size_t>(n - got), got);
		if (r <= 0) {
			std::perror("pread");
			std::exit(2);
		}
		got += r;
	}
}

/* ------------------------------------------------------------------ */
/* deterministic RNG (splitmix64)                                      */
/* ------------------------------------------------------------------ */

static std::uint64_t rng_state = 0x0000000000000183ULL;

static std::uint64_t
nextrand(void)
{
	std::uint64_t z = (rng_state += 0x9E3779B97F4A7C15ULL);

	z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
	z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
	return (z ^ (z >> 31));
}

/* ------------------------------------------------------------------ */
/* printset                                                            */
/* ------------------------------------------------------------------ */

static const std::size_t GUARD = 64;			/* guard bytes/side */
static const std::size_t NWORDS = 16;
static const std::size_t DATA = NWORDS * sizeof(long);	/* 1024 bits */
static const std::size_t TOTAL = GUARD + DATA + GUARD;
static const int MAXBIT = static_cast<int>(DATA * 8);	/* 1024 */

alignas(16) static unsigned char bufA[TOTAL];
alignas(16) static unsigned char bufB[TOTAL];
alignas(16) static unsigned char pristine[TOTAL];
static unsigned char payload[DATA];

static long cases_printset = 0;
static long fails_printset = 0;
static int shown_printset = 0;

static void
dumpstr(const char *what, const std::string &s)
{
	std::size_t n = s.size() < 160 ? s.size() : 160;

	std::fprintf(stderr, "    %s (%zu bytes): \"", what, s.size());
	for (std::size_t i = 0; i < n; i++) {
		unsigned char c = static_cast<unsigned char>(s[i]);

		if (c == '\n')
			std::fprintf(stderr, "\\n");
		else if (c >= 0x20 && c < 0x7f)
			std::fputc(c, stderr);
		else
			std::fprintf(stderr, "\\x%02x", c);
	}
	std::fprintf(stderr, "%s\"\n", s.size() > n ? "..." : "");
}

static void
printset_case(const unsigned char *in, int size, const char *tag)
{
	std::string outp, outr;
	bool ok;

	std::memset(bufA, 0x7f, TOTAL);
	std::memset(bufB, 0x7f, TOTAL);
	std::memcpy(bufA + GUARD, in, DATA);
	std::memcpy(bufB + GUARD, in, DATA);
	std::memcpy(pristine, bufA, TOTAL);

	cap_begin();
	P::printset(reinterpret_cast<P::bitset *>(bufA + GUARD), size);
	cap_end(outp);

	cap_begin();
	ref_printset(reinterpret_cast<struct ref_bitset *>(bufB + GUARD), size);
	cap_end(outr);

	cases_printset++;
	ok = true;
	if (outp != outr)
		ok = false;
	if (std::memcmp(bufA, bufB, TOTAL) != 0)
		ok = false;
	if (std::memcmp(bufA, pristine, TOTAL) != 0)
		ok = false;
	if (std::memcmp(bufB, pristine, TOTAL) != 0)
		ok = false;
	if (ok)
		return;

	fails_printset++;
	if (shown_printset++ < 12) {
		std::fprintf(stderr, "printset MISMATCH [%s] size=%d\n", tag,
		    size);
		dumpstr("port", outp);
		dumpstr("ref ", outr);
		for (std::size_t i = 0; i < TOTAL; i++)
			if (bufA[i] != bufB[i]) {
				std::fprintf(stderr, "    buffer differs at "
				    "byte %zu: port=%02x ref=%02x\n", i,
				    bufA[i], bufB[i]);
				break;
			}
		if (std::memcmp(bufA, pristine, TOTAL) != 0)
			std::fprintf(stderr, "    port wrote to its input\n");
		if (std::memcmp(bufB, pristine, TOTAL) != 0)
			std::fprintf(stderr, "    ref wrote to its input\n");
	}
}

static void
setbit(unsigned char *p, int bit)
{

	if (bit < 0 || bit >= MAXBIT)
		return;
	p[bit / 8] = static_cast<unsigned char>(p[bit / 8] | (1u << (bit % 8)));
}

static void
clrbit(unsigned char *p, int bit)
{

	if (bit < 0 || bit >= MAXBIT)
		return;
	p[bit / 8] = static_cast<unsigned char>(p[bit / 8] &
	    ~(1u << (bit % 8)));
}

/*
 * size is a plain int and the loop counts up from zero, so a negative size is
 * a real part of the input domain: the original prints nothing but the
 * newline.  These are the cases that separate "bit < size" from "bit != size".
 */
static const int negative_sizes[] = {
	-1, -2, -3, -7, -63, -64, -65, -128, -1000, -1000000,
	-2147483647 - 1
};
static const int NNEG = static_cast<int>(sizeof(negative_sizes) /
    sizeof(negative_sizes[0]));

static const int boundary_sizes[] = {
	0, 1, 2, 3, 4, 5, 7, 8, 9, 15, 16, 17, 31, 32, 33,
	62, 63, 64, 65, 66, 67, 95, 96, 97, 126, 127, 128, 129, 130, 131,
	191, 192, 193, 254, 255, 256, 257, 258, 319, 320, 383, 384,
	447, 448, 511, 512, 513, 575, 576, 639, 640, 703, 704, 767, 768,
	831, 832, 895, 896, 959, 960, 1022, 1023, 1024
};
static const int NBOUND = static_cast<int>(sizeof(boundary_sizes) /
    sizeof(boundary_sizes[0]));

static void
printset_handwritten(void)
{
	static const int small[] = { 0, 1, 2, 3, 4, 63, 64, 65, 66, 127, 128,
	    129, 255, 256, 257, 1023, 1024 };
	const int nsmall = static_cast<int>(sizeof(small) / sizeof(small[0]));

	/*
	 * Large sizes first.  Once size exceeds _BITSET_BITS the word index is
	 * bit / _BITSET_BITS, so a port that walks the counter the wrong way
	 * faults on the first step instead of spinning silently.
	 */
	static const int canary[] = { 1024, 513, 256, 129, 65 };

	for (int i = 0; i < static_cast<int>(sizeof(canary) / sizeof(canary[0]));
	    i++) {
		std::memset(payload, 0xff, DATA);
		printset_case(payload, canary[i], "canary-full");

		std::memset(payload, 0x00, DATA);
		setbit(payload, canary[i] - 1);
		printset_case(payload, canary[i], "canary-last");
	}

	/* nothing set, at every boundary size */
	for (int i = 0; i < NBOUND; i++) {
		std::memset(payload, 0x00, DATA);
		printset_case(payload, boundary_sizes[i], "clear");
	}

	/* everything set, at every boundary size */
	for (int i = 0; i < NBOUND; i++) {
		std::memset(payload, 0xff, DATA);
		printset_case(payload, boundary_sizes[i], "full");
	}

	/*
	 * One bit set, walked across the window edge.  A mask bit sitting at
	 * exactly `size` is what separates "bit < size" from "bit <= size";
	 * a bit at size-1 separates it from "bit < size - 1".
	 */
	for (int i = 0; i < NBOUND; i++) {
		int size = boundary_sizes[i];

		for (int d = -3; d <= 3; d++) {
			int bit = size + d;

			if (bit < 0)
				continue;
			std::memset(payload, 0x00, DATA);
			setbit(payload, bit);
			printset_case(payload, size, "one-at-boundary");
		}

		std::memset(payload, 0x00, DATA);
		setbit(payload, 0);
		printset_case(payload, size, "first-only");

		std::memset(payload, 0x00, DATA);
		setbit(payload, size - 1);
		printset_case(payload, size, "last-only");

		std::memset(payload, 0x00, DATA);
		setbit(payload, 0);
		setbit(payload, size - 1);
		printset_case(payload, size, "first+last");

		std::memset(payload, 0xff, DATA);
		clrbit(payload, 0);
		printset_case(payload, size, "full-but-first");

		std::memset(payload, 0xff, DATA);
		clrbit(payload, size - 1);
		printset_case(payload, size, "full-but-last");

		std::memset(payload, 0xff, DATA);
		clrbit(payload, size);
		printset_case(payload, size, "full-but-past-end");
	}

	/* adjacent pairs: drives the once == 0 -> once = 1 transition */
	for (int i = 0; i < NBOUND; i++) {
		int size = boundary_sizes[i];

		for (int b = 0; b + 1 < size && b < 4; b++) {
			std::memset(payload, 0x00, DATA);
			setbit(payload, b);
			setbit(payload, b + 1);
			printset_case(payload, size, "adjacent-pair");
		}
	}

	/* every single bit position, for the small sizes */
	for (int i = 0; i < nsmall; i++) {
		int size = small[i];

		for (int bit = 0; bit <= size && bit < MAXBIT; bit++) {
			std::memset(payload, 0x00, DATA);
			setbit(payload, bit);
			printset_case(payload, size, "walk-one");
		}
	}

	/* word crossing and regular patterns */
	for (int i = 0; i < NBOUND; i++) {
		int size = boundary_sizes[i];

		std::memset(payload, 0x00, DATA);
		for (int b = 60; b < 70; b++)
			setbit(payload, b);
		printset_case(payload, size, "word-cross");

		std::memset(payload, 0x00, DATA);
		for (int b = 0; b < MAXBIT; b += 64)
			setbit(payload, b);
		printset_case(payload, size, "word-firsts");

		std::memset(payload, 0x00, DATA);
		for (int b = 63; b < MAXBIT; b += 64)
			setbit(payload, b);
		printset_case(payload, size, "word-lasts");

		std::memset(payload, 0x55, DATA);
		printset_case(payload, size, "0x55");

		std::memset(payload, 0xaa, DATA);
		printset_case(payload, size, "0xaa");

		std::memset(payload, 0x80, DATA);
		printset_case(payload, size, "0x80");

		std::memset(payload, 0x01, DATA);
		printset_case(payload, size, "0x01");
	}

	/* negative sizes */
	for (int i = 0; i < NNEG; i++) {
		std::memset(payload, 0x00, DATA);
		printset_case(payload, negative_sizes[i], "negative-clear");

		std::memset(payload, 0xff, DATA);
		printset_case(payload, negative_sizes[i], "negative-full");

		std::memset(payload, 0x55, DATA);
		printset_case(payload, negative_sizes[i], "negative-0x55");

		std::memset(payload, 0x00, DATA);
		setbit(payload, 0);
		printset_case(payload, negative_sizes[i], "negative-bit0");
	}
}

static void
gen_payload(unsigned char *p, int size)
{
	unsigned mode = static_cast<unsigned>(nextrand() % 8);
	std::size_t i;

	switch (mode) {
	case 0:					/* empty mask */
		std::memset(p, 0x00, DATA);
		break;
	case 1:					/* every bit set */
		std::memset(p, 0xff, DATA);
		break;
	case 2:					/* exactly one bit */
		std::memset(p, 0x00, DATA);
		setbit(p, static_cast<int>(nextrand() % MAXBIT));
		break;
	case 3:					/* sparse */
		for (i = 0; i < DATA; i++)
			p[i] = (nextrand() % 8) == 0 ?
			    static_cast<unsigned char>(1u << (nextrand() % 8)) :
			    static_cast<unsigned char>(0);
		break;
	case 4:					/* uniform noise */
		for (i = 0; i < DATA; i++)
			p[i] = static_cast<unsigned char>(nextrand());
		break;
	case 5: {				/* regular patterns */
		static const unsigned char pat[] = {
			0x55, 0xaa, 0x0f, 0xf0, 0x01, 0x80, 0xfe, 0x7f
		};

		std::memset(p, pat[nextrand() % sizeof(pat)], DATA);
		break;
	}
	case 6:					/* exactly two bits */
		std::memset(p, 0x00, DATA);
		setbit(p, static_cast<int>(nextrand() % MAXBIT));
		setbit(p, static_cast<int>(nextrand() % MAXBIT));
		break;
	default:				/* bits astride the boundary */
		std::memset(p, 0x00, DATA);
		for (int d = -3; d <= 3; d++)
			if ((nextrand() & 1) != 0)
				setbit(p, size + d);
		if ((nextrand() & 1) != 0)
			setbit(p, 0);
		break;
	}
}

static void
printset_sweep(long iters)
{

	for (long it = 0; it < iters; it++) {
		std::uint64_t r = nextrand();
		int size;

		switch (r % 4) {
		case 0:
			size = boundary_sizes[nextrand() % NBOUND];
			break;
		case 1:
			size = static_cast<int>(nextrand() % 129);
			break;
		case 2:
			size = static_cast<int>(nextrand() % (MAXBIT + 1));
			break;
		default:
			size = static_cast<int>(nextrand() % 66);
			break;
		}
		if (nextrand() % 64 == 0)
			size = negative_sizes[nextrand() % NNEG];
		gen_payload(payload, size);
		printset_case(payload, size, "sweep");
	}
}

/* ------------------------------------------------------------------ */
/* usage                                                               */
/* ------------------------------------------------------------------ */

static long cases_usage = 0;
static long fails_usage = 0;

struct child_result {
	std::string	err;
	int		exited;
	int		status;
	int		signal;
};

static child_result
run_usage(bool use_port)
{
	child_result cr;
	int pfd[2];
	pid_t pid;
	int st = 0;

	cr.exited = 0;
	cr.status = -1;
	cr.signal = 0;
	if (::pipe(pfd) != 0) {
		std::perror("pipe");
		std::exit(2);
	}
	std::fflush(stdout);
	std::fflush(stderr);
	pid = ::fork();
	if (pid < 0) {
		std::perror("fork");
		std::exit(2);
	}
	if (pid == 0) {
		::close(pfd[0]);
		if (::dup2(pfd[1], 2) < 0)
			::_exit(120);
		::close(pfd[1]);
		if (use_port)
			P::usage();
		else
			ref_usage();
	}
	::close(pfd[1]);
	for (;;) {
		char b[4096];
		ssize_t n = ::read(pfd[0], b, sizeof(b));

		if (n > 0)
			cr.err.append(b, static_cast<std::size_t>(n));
		else
			break;
	}
	::close(pfd[0]);
	if (::waitpid(pid, &st, 0) < 0) {
		std::perror("waitpid");
		std::exit(2);
	}
	if (WIFEXITED(st)) {
		cr.exited = 1;
		cr.status = WEXITSTATUS(st);
	}
	if (WIFSIGNALED(st))
		cr.signal = WTERMSIG(st);
	return cr;
}

static void
usage_cases(int iters)
{

	for (int i = 0; i < iters; i++) {
		child_result p = run_usage(true);
		child_result r = run_usage(false);
		bool ok = true;

		cases_usage++;
		if (p.err != r.err)
			ok = false;
		if (p.exited != r.exited || p.status != r.status ||
		    p.signal != r.signal)
			ok = false;
		if (ok)
			continue;
		fails_usage++;
		std::fprintf(stderr, "usage MISMATCH (iteration %d)\n", i);
		std::fprintf(stderr,
		    "    port: exited=%d status=%d signal=%d\n",
		    p.exited, p.status, p.signal);
		std::fprintf(stderr,
		    "    ref : exited=%d status=%d signal=%d\n",
		    r.exited, r.status, r.signal);
		dumpstr("port stderr", p.err);
		dumpstr("ref  stderr", r.err);
	}
}

/* ------------------------------------------------------------------ */

int
main(void)
{
	struct rlimit rl;
	long total_cases, total_fails;

	/*
	 * A mutated loop condition can turn printset into an endless printf
	 * loop; cap CPU time and scratch file size so the run still dies
	 * (non-zero exit) instead of wedging the machine.  A clean run needs
	 * a few seconds and a few kilobytes.
	 */
	rl.rlim_cur = 90;
	rl.rlim_max = 90;
	(void)::setrlimit(RLIMIT_CPU, &rl);
	rl.rlim_cur = 256UL * 1024UL * 1024UL;
	rl.rlim_max = 256UL * 1024UL * 1024UL;
	(void)::setrlimit(RLIMIT_FSIZE, &rl);

	cap_setup();
	printset_handwritten();
	printset_sweep(200000);
	cap_restore();

	usage_cases(4);

	total_cases = cases_printset + cases_usage;
	total_fails = fails_printset + fails_usage;

	std::printf("\n");
	std::printf("%-12s %12s %12s  %s\n", "function", "cases", "failures",
	    "result");
	std::printf("--------------------------------------------------"
	    "----\n");
	std::printf("%-12s %12ld %12ld  %s\n", "printset", cases_printset,
	    fails_printset, fails_printset == 0 ? "PASS" : "FAIL");
	std::printf("%-12s %12ld %12ld  %s\n", "usage", cases_usage,
	    fails_usage, fails_usage == 0 ? "PASS" : "FAIL");
	std::printf("--------------------------------------------------"
	    "----\n");
	std::printf("%-12s %12ld %12ld  %s\n", "TOTAL", total_cases,
	    total_fails, total_fails == 0 ? "PASS" : "FAIL");
	std::fflush(stdout);

	return (total_fails == 0 ? 0 : 1);
}
