/*-
 * Differential test harness for PBSD batch b0187s2 (bin/stty/stty.c).
 *
 * Copyright (c) 1989, 1991, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 * (Original copyright retained; see port.cppm / oracle.c.)
 *
 * The single ported function, usage(), takes no arguments and never returns:
 * it writes a fixed diagnostic to stderr and then calls exit(1).  Its complete
 * observable behaviour is therefore
 *
 *	1. the exact byte sequence handed to file descriptor 2, at whatever
 *	   file offset descriptor 2 happens to sit, and
 *	2. the status value passed to exit().
 *
 * Both are captured here.  exit() is intercepted with the linker's
 * --wrap=exit (see build.sh) and turned into a longjmp back into the driver,
 * so the port and the ref_ oracle can be run alternately inside one process
 * with no fork.  Descriptor 2 is redirected at each call to a sink whose
 * contents are then read back in full.
 *
 * Buffer discipline, as required: each side owns its own capture buffer and
 * its own private scratch file.  Both buffers are filled with the guard byte
 * 0x7f before every case, both files receive a byte-identical prefill, and the
 * ENTIRE capture buffer -- 512 bytes, far past the 48 byte write window and
 * past the prefill region -- is compared afterwards, along with the byte count,
 * the resulting file offset, the stream error indicator and the exit status.
 * Nothing is compared by return value alone; usage() has no return value.
 */

#include <cerrno>
#include <csetjmp>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <fcntl.h>
#include <unistd.h>

import pbsd.bin.stty.b0187s2;

extern "C" void ref_usage(void);
extern "C" [[noreturn]] void __real_exit(int);

/* ------------------------------------------------------------------ */
/* exit() interception						      */
/* ------------------------------------------------------------------ */

static std::jmp_buf probe_jb;
static int probe_active;
static int probe_exit_called;
static int probe_exit_status;

extern "C" [[noreturn]] void
__wrap_exit(int status)
{
	if (probe_active) {
		probe_active = 0;
		probe_exit_called = 1;
		probe_exit_status = status;
		std::longjmp(probe_jb, 1);
	}
	__real_exit(status);
}

/* ------------------------------------------------------------------ */
/* capture machinery						      */
/* ------------------------------------------------------------------ */

enum {
	SINK_FILE,	/* regular file, descriptor positioned at cfg.off */
	SINK_APPEND,	/* same file, O_APPEND set (offset ignored on write) */
	SINK_PIPE,	/* pipe write end */
	SINK_RDONLY,	/* read-only descriptor: write() fails EBADF */
	SINK_CLOSED,	/* descriptor 2 closed: write() fails EBADF */
	SINK_NKINDS
};

static const int CAPSZ = 512;
static const int MAXPRE = 256;
static const unsigned char GUARD = 0x7f;

struct Cfg {
	int sink;
	int prefill_len;	/* 0 .. MAXPRE */
	int prefill_kind;	/* see fill_prefill() */
	unsigned prefill_seed;
	long off;		/* seek position before the call */
};

struct Capture {
	int exit_called;
	int exit_status;
	int io_error;		/* ferror(stderr) after the call */
	long file_off;		/* descriptor offset after the call, -1 if n/a */
	long nread;		/* bytes recovered from the sink */
	unsigned char buf[CAPSZ];
};

static int tmpfd[2] = { -1, -1 };
static int rdonly_fd = -1;
static int saved_stderr = -1;

static void
fatal(const char *what)
{
	if (saved_stderr >= 0)
		(void)dup2(saved_stderr, 2);
	std::fprintf(stderr, "harness: %s: %s\n", what, std::strerror(errno));
	__real_exit(2);
}

static void
fill_prefill(const Cfg &c, unsigned char *p)
{
	unsigned s = c.prefill_seed | 1u;

	for (int i = 0; i < c.prefill_len; i++) {
		unsigned char v;

		switch (c.prefill_kind) {
		case 0:
			v = GUARD;
			break;
		case 1:
			v = 0x00;
			break;
		case 2:
			/* high-bit bytes 0x80 .. 0xff */
			v = (unsigned char)(0x80 + (i & 0x7f));
			break;
		case 3:
			s = s * 1664525u + 1013904223u;
			v = (unsigned char)(s >> 24);
			break;
		case 4:
			/* NUL-heavy with occasional 0xff */
			v = (unsigned char)((i % 7) == 0 ? 0xff : 0x00);
			break;
		default:
			v = 'A';
			break;
		}
		p[i] = v;
	}
}

static void
setup(void)
{
	for (int i = 0; i < 2; i++) {
		char path[] = "/tmp/pbsd_b0187s2_XXXXXX";
		int fd = mkstemp(path);

		if (fd < 0)
			fatal("mkstemp");
		if (unlink(path) != 0)
			fatal("unlink");
		tmpfd[i] = fd;
	}
	rdonly_fd = open("/dev/null", O_RDONLY);
	if (rdonly_fd < 0)
		fatal("open /dev/null");
	saved_stderr = dup(2);
	if (saved_stderr < 0)
		fatal("dup");
	/*
	 * stderr is unbuffered per the C standard, but pin it down so the two
	 * sides cannot differ because of an inherited buffering mode.
	 */
	if (setvbuf(stderr, NULL, _IONBF, 0) != 0)
		fatal("setvbuf");
}

/*
 * side 0 runs the C++23 port, side 1 runs the ref_ oracle.  Each side uses its
 * own scratch file so the two runs cannot see one another's bytes.
 */
static void
run_side(int side, const Cfg &cfg, Capture &cap)
{
	unsigned char pre[MAXPRE];
	int rfd = -1;
	int pipe_rd = -1;

	fill_prefill(cfg, pre);

	std::memset(cap.buf, GUARD, sizeof(cap.buf));
	cap.exit_called = -1;
	cap.exit_status = -12345;
	cap.io_error = -1;
	cap.file_off = -1;
	cap.nread = -2;

	switch (cfg.sink) {
	case SINK_FILE:
	case SINK_APPEND: {
		int fd = tmpfd[side];

		if (ftruncate(fd, 0) != 0)
			fatal("ftruncate");
		if (fcntl(fd, F_SETFL,
		    cfg.sink == SINK_APPEND ? O_APPEND : 0) != 0)
			fatal("fcntl F_SETFL");
		if (lseek(fd, 0, SEEK_SET) < 0)
			fatal("lseek rewind");
		if (cfg.prefill_len > 0) {
			ssize_t w = write(fd, pre, (size_t)cfg.prefill_len);

			if (w != (ssize_t)cfg.prefill_len)
				fatal("write prefill");
		}
		if (lseek(fd, cfg.off, SEEK_SET) < 0)
			fatal("lseek off");
		if (dup2(fd, 2) < 0)
			fatal("dup2 file");
		rfd = fd;
		break;
	}
	case SINK_PIPE: {
		int pf[2];

		if (pipe(pf) != 0)
			fatal("pipe");
		if (fcntl(pf[0], F_SETFL, O_NONBLOCK) != 0)
			fatal("fcntl O_NONBLOCK");
		if (dup2(pf[1], 2) < 0)
			fatal("dup2 pipe");
		(void)close(pf[1]);
		pipe_rd = pf[0];
		break;
	}
	case SINK_RDONLY:
		if (dup2(rdonly_fd, 2) < 0)
			fatal("dup2 rdonly");
		break;
	case SINK_CLOSED:
		(void)close(2);
		break;
	}

	clearerr(stderr);
	errno = 0;

	probe_exit_called = 0;
	probe_exit_status = -999;
	probe_active = 1;
	if (setjmp(probe_jb) == 0) {
		if (side == 0)
			pbsd::bin_stty::b0187s2::usage();
		else
			ref_usage();
	}
	probe_active = 0;
	cap.exit_called = probe_exit_called;
	cap.exit_status = probe_exit_status;

	(void)fflush(stderr);
	cap.io_error = ferror(stderr) ? 1 : 0;
	clearerr(stderr);

	if (rfd >= 0) {
		cap.file_off = (long)lseek(rfd, 0, SEEK_CUR);
		if (lseek(rfd, 0, SEEK_SET) < 0)
			fatal("lseek readback");
		ssize_t n = read(rfd, cap.buf, sizeof(cap.buf));
		cap.nread = (long)n;
	} else if (pipe_rd >= 0) {
		ssize_t n = read(pipe_rd, cap.buf, sizeof(cap.buf));
		cap.nread = (long)n;
		(void)close(pipe_rd);
	} else {
		cap.nread = 0;
	}

	if (dup2(saved_stderr, 2) < 0)
		fatal("restore stderr");
}

static int
same(const Capture &a, const Capture &b)
{
	return a.exit_called == b.exit_called &&
	    a.exit_status == b.exit_status &&
	    a.io_error == b.io_error &&
	    a.file_off == b.file_off &&
	    a.nread == b.nread &&
	    std::memcmp(a.buf, b.buf, sizeof(a.buf)) == 0;
}

static const char *
sinkname(int s)
{
	switch (s) {
	case SINK_FILE:
		return "file";
	case SINK_APPEND:
		return "append";
	case SINK_PIPE:
		return "pipe";
	case SINK_RDONLY:
		return "rdonly";
	case SINK_CLOSED:
		return "closed";
	}
	return "?";
}

static long usage_cases;
static long usage_failures;
static int reported;

static void
dump(const char *tag, const Capture &c)
{
	std::printf("    %-4s exit_called=%d exit_status=%d io_error=%d "
	    "file_off=%ld nread=%ld\n", tag, c.exit_called, c.exit_status,
	    c.io_error, c.file_off, c.nread);
	std::printf("        buf[0..63]:");
	for (int i = 0; i < 64; i++)
		std::printf(" %02x", c.buf[i]);
	std::printf("\n");
}

static void
check(const Cfg &cfg, const char *label)
{
	Capture pc, rc;

	run_side(0, cfg, pc);
	run_side(1, cfg, rc);

	usage_cases++;
	if (same(pc, rc))
		return;

	usage_failures++;
	if (reported < 10) {
		reported++;
		std::printf("FAIL usage [%s] sink=%s prefill_len=%d kind=%d "
		    "seed=%u off=%ld\n", label, sinkname(cfg.sink),
		    cfg.prefill_len, cfg.prefill_kind, cfg.prefill_seed,
		    cfg.off);
		dump("port", pc);
		dump("ref", rc);
		for (int i = 0; i < CAPSZ; i++)
			if (pc.buf[i] != rc.buf[i]) {
				std::printf("        first byte diff at %d: "
				    "port=%02x ref=%02x\n", i, pc.buf[i],
				    rc.buf[i]);
				break;
			}
	}
}

/* ------------------------------------------------------------------ */
/* deterministic RNG						      */
/* ------------------------------------------------------------------ */

static unsigned long long rng_state;

static unsigned long long
rng_next(void)
{
	unsigned long long z;

	rng_state += 0x9e3779b97f4a7c15ULL;
	z = rng_state;
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return z ^ (z >> 31);
}

static unsigned
rng_below(unsigned n)
{
	return (unsigned)(rng_next() % n);
}

/* ------------------------------------------------------------------ */

static void
edge_cases(void)
{
	/*
	 * The diagnostic is 48 bytes ("usage: stty [-a | -e | -g] [-f file] "
	 * "[arguments]\n").  Offsets sit on both sides of every interesting
	 * boundary: empty file, one byte, one short of the message, exactly
	 * the message, one past, the prefill edge, and past the end of the
	 * prefill so that a sparse hole is created.
	 */
	static const long offs[] = {
		0, 1, 2, 46, 47, 48, 49, 50, 63, 64, 127, 128,
		255, 256, 257, 300
	};
	static const int lens[] = { 0, 1, 2, 47, 48, 49, 128, 255, 256 };

	for (int k = 0; k < SINK_NKINDS; k++)
		for (unsigned li = 0; li < sizeof(lens) / sizeof(lens[0]); li++)
			for (unsigned oi = 0;
			    oi < sizeof(offs) / sizeof(offs[0]); oi++)
				for (int kind = 0; kind <= 5; kind++) {
					Cfg c;

					c.sink = k;
					c.prefill_len = lens[li];
					c.prefill_kind = kind;
					c.prefill_seed = 0x9e3779b9u +
					    (unsigned)kind;
					c.off = offs[oi];
					check(c, "edge");
				}
}

static void
sweep(long iters)
{
	rng_state = 0x0187002ULL;	/* fixed seed */

	for (long i = 0; i < iters; i++) {
		Cfg c;
		unsigned r = rng_below(100);

		if (r < 55)
			c.sink = SINK_FILE;
		else if (r < 75)
			c.sink = SINK_APPEND;
		else if (r < 90)
			c.sink = SINK_PIPE;
		else if (r < 95)
			c.sink = SINK_RDONLY;
		else
			c.sink = SINK_CLOSED;

		c.prefill_len = (int)rng_below(MAXPRE + 1);
		c.prefill_kind = (int)rng_below(6);
		c.prefill_seed = (unsigned)rng_next();
		c.off = (long)rng_below(301);

		check(c, "sweep");
	}
}

int
main(void)
{
	setup();

	edge_cases();
	sweep(200000);

	std::printf("\n");
	std::printf("%-24s %12s %12s  %s\n", "function", "cases", "failures",
	    "result");
	std::printf("%-24s %12s %12s  %s\n", "------------------------",
	    "------------", "------------", "------");
	std::printf("%-24s %12ld %12ld  %s\n", "usage", usage_cases,
	    usage_failures, usage_failures == 0 ? "PASS" : "FAIL");
	std::printf("\n");
	std::printf("total: %ld cases, %ld failures\n",
	    usage_cases, usage_failures);
	std::printf("skipped: bin/stty/stty.c:main (see skipped.txt)\n");

	(void)fflush(stdout);
	return usage_failures == 0 ? 0 : 1;
}
