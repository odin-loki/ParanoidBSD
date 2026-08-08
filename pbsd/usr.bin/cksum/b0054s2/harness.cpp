/*
 * Differential test harness for pbsd/usr.bin/cksum/b0054s2.
 *
 * Compares the C++23 module port of usr.bin/cksum/crc.c against the
 * unmodified C original (oracle.c, ref_ prefixed).
 *
 * crc() communicates through three channels:
 *   - its int return value,
 *   - the two out-parameters *cval and *clen,
 *   - the file-static running accumulator crc_total, which is *not* reset
 *     between calls and which is deliberately left inverted when crc()
 *     takes its error return.
 * All three are compared after every single case.  The out-parameters live
 * inside a 64-byte buffer pre-filled with the guard byte 0x7f, and the whole
 * buffer is compared afterwards, so a port that writes the wrong number of
 * bytes, writes to the wrong slot, or writes on the error path is caught.
 */

#include <cstdio>
#include <cstring>
#include <cstdint>
#include <cstdlib>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

import pbsd.usr.bin.cksum.b0054s2;

namespace port = pbsd::usr_bin_cksum::b0054s2;

extern "C" {
int ref_crc(int fd, uint32_t *cval, off_t *clen);
uint32_t ref_crc_total_value(void);
}

#define GUARD		0x7f
#define OUTBUF_SIZE	64
#define CVAL_OFF	16
#define CLEN_OFF	32

struct Stats {
	const char *name;
	long cases;
	long fails;
};

static Stats st_patterns = { "edge/patterns", 0, 0 };
static Stats st_sizes = { "edge/sizes", 0, 0 };
static Stats st_errors = { "edge/error-fds", 0, 0 };
static Stats st_random = { "random sweep", 0, 0 };

static long reported;

static int tmpfd = -1;
static int wronly_fd = -1;
static int dir_fd = -1;

#define MAXDATA		80000
static unsigned char data[MAXDATA];

static void
die(const char *what)
{
	perror(what);
	exit(2);
}

static void
setup_fds(void)
{
	char tmpl[] = "/tmp/pbsd_b0054s2_XXXXXX";
	char tmpl2[] = "/tmp/pbsd_b0054s2_w_XXXXXX";
	int t;

	tmpfd = mkstemp(tmpl);
	if (tmpfd < 0)
		die("mkstemp");
	if (unlink(tmpl) != 0)
		die("unlink");

	/* A write-only descriptor: read(2) on it fails with EBADF. */
	t = mkstemp(tmpl2);
	if (t < 0)
		die("mkstemp");
	wronly_fd = open(tmpl2, O_WRONLY);
	if (wronly_fd < 0)
		die("open O_WRONLY");
	close(t);
	if (unlink(tmpl2) != 0)
		die("unlink");

	/* A directory descriptor: read(2) on it fails with EISDIR. */
	dir_fd = open(".", O_RDONLY | O_DIRECTORY);
	if (dir_fd < 0)
		die("open .");
}

/* Load the scratch file with n bytes of d and rewind it. */
static void
load(const unsigned char *d, size_t n)
{
	size_t off;
	ssize_t w;

	if (ftruncate(tmpfd, 0) != 0)
		die("ftruncate");
	if (lseek(tmpfd, 0, SEEK_SET) != 0)
		die("lseek");
	for (off = 0; off < n; off += (size_t)w) {
		w = write(tmpfd, d + off, n - off);
		if (w <= 0)
			die("write");
	}
	if (lseek(tmpfd, 0, SEEK_SET) != 0)
		die("lseek");
}

static void
describe(const char *label, const unsigned char *A, int r, uint32_t total)
{
	uint32_t cval;
	off_t clen;
	int i;

	memcpy(&cval, A + CVAL_OFF, sizeof(cval));
	memcpy(&clen, A + CLEN_OFF, sizeof(clen));
	printf("      %s: ret=%d cval=0x%08lx clen=%lld crc_total=0x%08lx guard=",
	    label, r, (unsigned long)cval, (long long)clen,
	    (unsigned long)total);
	for (i = 0; i < OUTBUF_SIZE; i++)
		printf("%s%02x", (i == CVAL_OFF || i == CLEN_OFF) ? "|" : "",
		    A[i]);
	printf("\n");
}

/*
 * Run one case against both implementations.  fd is used as-is (only rewound
 * when it is seekable) so that error descriptors can be exercised too.
 */
static void
check(Stats *st, const char *what, long detail, int fd, bool seekable)
{
	alignas(16) unsigned char A[OUTBUF_SIZE];
	alignas(16) unsigned char B[OUTBUF_SIZE];
	uint32_t *cvalA, *cvalB;
	off_t *clenA, *clenB;
	uint32_t totA, totB;
	int rA, rB;
	bool ok;

	memset(A, GUARD, sizeof(A));
	memset(B, GUARD, sizeof(B));
	cvalA = reinterpret_cast<uint32_t *>(A + CVAL_OFF);
	clenA = reinterpret_cast<off_t *>(A + CLEN_OFF);
	cvalB = reinterpret_cast<uint32_t *>(B + CVAL_OFF);
	clenB = reinterpret_cast<off_t *>(B + CLEN_OFF);

	if (seekable && lseek(fd, 0, SEEK_SET) != 0)
		die("lseek");
	rA = port::crc(fd, cvalA, clenA);
	totA = port::crc_total_value();

	if (seekable && lseek(fd, 0, SEEK_SET) != 0)
		die("lseek");
	rB = ref_crc(fd, cvalB, clenB);
	totB = ref_crc_total_value();

	st->cases++;
	ok = rA == rB && totA == totB &&
	    memcmp(A, B, sizeof(A)) == 0;
	if (ok)
		return;

	st->fails++;
	if (reported < 20) {
		reported++;
		printf("  FAIL [%s] %s (detail=%ld)\n", st->name, what, detail);
		describe("port", A, rA, totA);
		describe("ref ", B, rB, totB);
	}
}

static void
run_data(Stats *st, const char *what, long detail, const unsigned char *d,
    size_t n)
{
	load(d, n);
	check(st, what, detail, tmpfd, true);
}

/* ------------------------------------------------------------------ */

static uint64_t rngstate;

static inline uint64_t
rnd(void)
{
	uint64_t x = rngstate;

	x ^= x << 13;
	x ^= x >> 7;
	x ^= x << 17;
	rngstate = x;
	return (x);
}

/* ------------------------------------------------------------------ */

static void
fill_const(size_t n, unsigned char c)
{
	memset(data, c, n);
}

static void
patterns(void)
{
	size_t i;
	static const unsigned char empty[1] = { 0 };

	/* Empty input: no data loop, no length loop, *cval == ~0. */
	run_data(&st_patterns, "empty", 0, empty, 0);

	/* Every possible single byte, including the whole high half. */
	for (i = 0; i < 256; i++) {
		data[0] = (unsigned char)i;
		run_data(&st_patterns, "single byte", (long)i, data, 1);
	}

	/* Two bytes: exercises the ++p advance (one byte cannot). */
	for (i = 0; i < 256; i++) {
		data[0] = (unsigned char)i;
		data[1] = (unsigned char)(255 - i);
		run_data(&st_patterns, "two bytes", (long)i, data, 2);
	}

	/* NUL-heavy: drives crctab[0] on the very first byte. */
	for (i = 0; i <= 40; i++) {
		fill_const(i, 0x00);
		run_data(&st_patterns, "all NUL", (long)i, data, i);
	}

	/* All bits set. */
	for (i = 0; i <= 40; i++) {
		fill_const(i, 0xff);
		run_data(&st_patterns, "all 0xff", (long)i, data, i);
	}

	/* All bytes with the high bit set, one value at a time. */
	for (i = 0x80; i <= 0xff; i++) {
		fill_const(37, (unsigned char)i);
		run_data(&st_patterns, "high-bit run", (long)i, data, 37);
	}

	/* 0x00..0xff, and the high half only. */
	for (i = 0; i < 256; i++)
		data[i] = (unsigned char)i;
	run_data(&st_patterns, "0x00..0xff", 0, data, 256);
	for (i = 0; i < 128; i++)
		data[i] = (unsigned char)(0x80 + i);
	run_data(&st_patterns, "0x80..0xff", 0, data, 128);
	for (i = 0; i < 128; i++)
		data[i] = (unsigned char)(0xff - i);
	run_data(&st_patterns, "0xff..0x80", 0, data, 128);

	/* Alternating extremes. */
	for (i = 0; i < 512; i++)
		data[i] = (i & 1) ? 0xff : 0x00;
	run_data(&st_patterns, "alt 00/ff", 0, data, 512);
	for (i = 0; i < 512; i++)
		data[i] = (i & 1) ? 0x00 : 0x80;
	run_data(&st_patterns, "alt 80/00", 0, data, 512);

	/* Embedded NULs inside text. */
	{
		static const unsigned char nulmix[] = {
			'a', 0x00, 'b', 0x00, 0x00, 0xff, 0x80, 0x7f,
			0x00, 0x01, 0x00, 0xfe, 0x00, 0x00, 0x00, 0x00
		};
		run_data(&st_patterns, "text with NULs", 0, nulmix,
		    sizeof(nulmix));
	}
	{
		static const unsigned char txt[] =
		    "The quick brown fox jumps over the lazy dog\n";
		run_data(&st_patterns, "ascii text", 0, txt, sizeof(txt) - 1);
		run_data(&st_patterns, "ascii text +NUL", 0, txt, sizeof(txt));
	}
}

static void
sizes(void)
{
	static const long list[] = {
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 15, 16, 17, 31, 32, 33,
		63, 64, 65, 127, 128, 129,
		254, 255, 256, 257, 258,		/* length loop 1 -> 2 */
		511, 512, 513, 1023, 1024, 1025,
		4095, 4096, 4097, 8191, 8192, 8193,
		16383, 16384, 16385, 16386,		/* one read -> two */
		32767, 32768, 32769,
		49151, 49152, 49153,
		65534, 65535, 65536, 65537,		/* length loop 2 -> 3 */
		79999
	};
	size_t i, j;

	for (i = 0; i < sizeof(list) / sizeof(list[0]); i++) {
		size_t n = (size_t)list[i];

		/* Deterministic non-trivial content, all 256 byte values. */
		for (j = 0; j < n; j++)
			data[j] = (unsigned char)((j * 31u + (j >> 8) * 7u) &
			    0xff);
		run_data(&st_sizes, "mixed", list[i], data, n);

		fill_const(n, 0x00);
		run_data(&st_sizes, "zeros", list[i], data, n);

		fill_const(n, 0xff);
		run_data(&st_sizes, "ones", list[i], data, n);
	}
}

static void
errors(void)
{
	int i;

	/*
	 * read(2) fails immediately: crc() must return 1, must not touch
	 * either out-parameter, and must leave crc_total inverted.  Repeat so
	 * that the inversion parity is observed on both sides.
	 */
	for (i = 0; i < 5; i++) {
		check(&st_errors, "fd = -1", i, -1, false);
		check(&st_errors, "write-only fd", i, wronly_fd, false);
		check(&st_errors, "directory fd", i, dir_fd, false);

		/* A good case in between, to check the parity recovers. */
		fill_const(7, (unsigned char)(0x80 + i));
		run_data(&st_errors, "good after error", i, data, 7);
	}
}

/*
 * Cases that start from a chosen crc_total instead of whatever the running
 * history happens to be.  crc() inverts crc_total on entry, so presetting v
 * makes the accumulator start at ~v; picking the first data byte equal to
 * (~v >> 24) drives the crc_total COMPUTE straight into crctab[0].
 */
static void
state(void)
{
	size_t i, j;

	for (i = 0; i < 256; i++) {
		uint32_t notv = ((uint32_t)i << 24) | 0x5a5a5au;

		preset = (long long)(uint32_t)~notv;
		data[0] = (unsigned char)i;
		for (j = 1; j < 24; j++)
			data[j] = (unsigned char)(0xff - j);
		run_data(&st_state, "crctab[0] via crc_total", (long)i, data,
		    24);
	}

	{
		static const uint32_t presets[] = {
			0x00000000u, 0xffffffffu, 0x00000001u, 0xfffffffeu,
			0x80000000u, 0x7fffffffu, 0x000000ffu, 0xff000000u,
			0x12345678u, 0xdeadbeefu
		};

		for (i = 0; i < sizeof(presets) / sizeof(presets[0]); i++) {
			preset = (long long)presets[i];

			run_data(&st_state, "preset, empty", (long)i, data, 0);

			data[0] = 0x00;
			run_data(&st_state, "preset, one NUL", (long)i, data, 1);

			data[0] = 0xff;
			run_data(&st_state, "preset, one 0xff", (long)i, data,
			    1);

			for (j = 0; j < 300; j++)
				data[j] = (unsigned char)(j * 17u);
			run_data(&st_state, "preset, 300 bytes", (long)i, data,
			    300);

			memset(data, 0, 300);
			run_data(&st_state, "preset, 300 NULs", (long)i, data,
			    300);

			/* Error return from a known state: no re-inversion. */
			check(&st_state, "preset, fd = -1", (long)i, -1, false);
			check(&st_state, "preset, write-only fd", (long)i,
			    wronly_fd, false);
		}
	}

	preset = -1;
}

static void
sweep(void)
{
	const long iters = 200000;
	long i;
	size_t n, j;
	uint64_t r;

	rngstate = 0x0123456789abcdefULL;

	for (i = 0; i < iters; i++) {
		unsigned bucket = (unsigned)(i & 1023);

		r = rnd();
		if (bucket == 1023)
			n = 16000 + (size_t)(r % 60000);
		else if (bucket >= 1016)
			n = 1024 + (size_t)(r % 12000);
		else if (bucket >= 960)
			n = 129 + (size_t)(r % 1024);
		else
			n = (size_t)(r % 129);

		switch ((unsigned)(rnd() % 8)) {
		case 0:
			for (j = 0; j < n; j++)
				data[j] = (unsigned char)(rnd() & 0xff);
			break;
		case 1:
			fill_const(n, 0x00);
			break;
		case 2:
			fill_const(n, 0xff);
			break;
		case 3: {
			static const unsigned char pick[6] = {
				0x00, 0x01, 0x7f, 0x80, 0xfe, 0xff
			};
			for (j = 0; j < n; j++)
				data[j] = pick[rnd() % 6];
			break;
		}
		case 4:
			for (j = 0; j < n; j++)
				data[j] = (unsigned char)(0x20 + rnd() % 95);
			break;
		case 5: {
			unsigned char c = (unsigned char)(rnd() & 0xff);

			for (j = 0; j < n; j++)
				data[j] = (unsigned char)(c + j);
			break;
		}
		case 6:
			memset(data, 0, n);
			for (j = 0; j < n; j++)
				if ((rnd() & 7) == 0)
					data[j] = (unsigned char)(rnd() & 0xff);
			break;
		default:
			for (j = 0; j < n; j++)
				data[j] = (unsigned char)(0x80 | (rnd() & 0x7f));
			break;
		}

		run_data(&st_random, "random", i, data, n);
	}
}

static void
row(const Stats *s)
{
	printf("  %-14s %-18s %10ld %10ld\n", "crc", s->name, s->cases,
	    s->fails);
}

int
main(void)
{
	long cases, fails;

	/*
	 * Watchdog: a broken port can loop forever (e.g. a flipped test in
	 * the read loop or the length loop).  Dying on SIGALRM still yields a
	 * non-zero exit status, which is what a failure must look like.
	 */
	alarm(600);

	setup_fds();

	patterns();
	sizes();
	errors();
	state();
	sweep();

	cases = st_patterns.cases + st_sizes.cases + st_errors.cases +
	    st_state.cases + st_random.cases;
	fails = st_patterns.fails + st_sizes.fails + st_errors.fails +
	    st_state.fails + st_random.fails;

	printf("\n");
	printf("  %-14s %-18s %10s %10s\n", "function", "group", "cases",
	    "failures");
	printf("  ---------------------------------------------------------\n");
	row(&st_patterns);
	row(&st_sizes);
	row(&st_errors);
	row(&st_state);
	row(&st_random);
	printf("  ---------------------------------------------------------\n");
	printf("  %-14s %-18s %10ld %10ld\n", "crc", "TOTAL", cases, fails);
	printf("\n");
	printf("  (every case also compares crc_total_get() against\n");
	printf("   ref_crc_total_get(), and the entire 64-byte guarded\n");
	printf("   out-parameter buffer, not just the return value)\n");
	printf("\n");
	printf("  RESULT: %s\n", fails == 0 ? "PASS" : "FAIL");

	return (fails == 0 ? 0 : 1);
}
