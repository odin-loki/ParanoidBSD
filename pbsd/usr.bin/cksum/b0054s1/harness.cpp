/*
 * PBSD batch b0054s1 -- differential harness.
 *
 * Drives the C++23 port (pbsd.usr.bin.cksum.b0054s1) and the untouched C
 * reference (oracle.c, ref_ prefixed) over identical file descriptors and
 * compares every observable:
 *
 *   - the int return value;
 *   - the ENTIRE output buffer holding *cval and *clen, guard-filled with
 *     0x7f, including the bytes either side of the nominal write windows,
 *     so that a port which fails to write (or over-writes) is caught;
 *   - the file-static crc32_total accumulator after every call, since it
 *     carries state across calls and is deliberately left complemented when
 *     crc32() takes the read-error return.
 *
 * crc32() has no pointer return and no caller-visible iterator state, so the
 * offset/iterator comparison rules do not apply; the cross-call accumulator
 * is the stateful part and it is compared after each and every invocation.
 */

#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

import pbsd.usr.bin.cksum.b0054s1;

namespace port = pbsd::usr_bin_cksum::b0054s1;

extern "C" {
int ref_crc32(int fd, uint32_t *cval, off_t *clen);
uint32_t ref_crc32_total_peek(void);
}

/* ------------------------------------------------------------------ */
/* bookkeeping                                                         */

struct Stat {
	const char *name;
	unsigned long cases;
	unsigned long fails;
};

static Stat st_crc32 = { "crc32", 0, 0 };
static Stat st_total = { "crc32 static accumulator", 0, 0 };

static int printed_failures = 0;
static const int MAX_PRINTED = 25;

static void
fatal(const char *m)
{
	std::perror(m);
	std::exit(2);
}

/* ------------------------------------------------------------------ */
/* output buffers                                                      */

/*
 * 96 guard bytes; *cval lands at offset 16, *clen at offset 48.  Everything
 * else must still read 0x7f when the call returns.
 */
static const std::size_t OUTBUF = 96;
static const std::size_t CVAL_OFF = 16;
static const std::size_t CLEN_OFF = 48;
static const unsigned char GUARD = 0x7f;

static bool
guards_intact(const unsigned char *b)
{
	for (std::size_t i = 0; i < OUTBUF; i++) {
		bool inside = (i >= CVAL_OFF && i < CVAL_OFF + sizeof(uint32_t)) ||
			      (i >= CLEN_OFF && i < CLEN_OFF + sizeof(off_t));
		if (!inside && b[i] != GUARD)
			return false;
	}
	return true;
}

static void
dump(const char *which, const unsigned char *b)
{
	std::printf("      %s:", which);
	for (std::size_t i = 0; i < OUTBUF; i++)
		std::printf("%s%02x", (i % 16) == 0 ? "\n        " : " ", b[i]);
	std::printf("\n");
}

/* ------------------------------------------------------------------ */
/* the differential step                                               */

static int g_tmpfd = -1;
static int g_badfd = -1;

/*
 * Run both implementations against the same descriptor.  `seekable' is false
 * for the deliberately-unreadable descriptor used to reach the error return.
 */
static void
do_case(const char *kind, long idx, long long nbytes, int fd, bool seekable)
{
	alignas(16) unsigned char bp[OUTBUF];
	alignas(16) unsigned char br[OUTBUF];

	std::memset(bp, GUARD, OUTBUF);
	std::memset(br, GUARD, OUTBUF);

	uint32_t *cvp = reinterpret_cast<uint32_t *>(bp + CVAL_OFF);
	off_t *clp = reinterpret_cast<off_t *>(bp + CLEN_OFF);
	uint32_t *cvr = reinterpret_cast<uint32_t *>(br + CVAL_OFF);
	off_t *clr = reinterpret_cast<off_t *>(br + CLEN_OFF);

	if (seekable && lseek(fd, 0, SEEK_SET) == (off_t)-1)
		fatal("lseek");
	int rp = port::crc32(fd, cvp, clp);
	uint32_t tp = port::crc32_total_peek();

	if (seekable && lseek(fd, 0, SEEK_SET) == (off_t)-1)
		fatal("lseek");
	int rr = ref_crc32(fd, cvr, clr);
	uint32_t tr = ref_crc32_total_peek();

	st_crc32.cases++;
	st_total.cases++;

	bool bad_ret = (rp != rr);
	bool bad_buf = (std::memcmp(bp, br, OUTBUF) != 0);
	bool bad_grd = !guards_intact(bp) || !guards_intact(br);
	bool bad_tot = (tp != tr);

	if (bad_ret || bad_buf || bad_grd)
		st_crc32.fails++;
	if (bad_tot)
		st_total.fails++;

	if ((bad_ret || bad_buf || bad_grd || bad_tot) &&
	    printed_failures < MAX_PRINTED) {
		printed_failures++;
		std::printf("  FAIL [%s #%ld, %lld bytes]%s%s%s%s\n",
		    kind, idx, nbytes,
		    bad_ret ? " ret" : "", bad_buf ? " buffer" : "",
		    bad_grd ? " guard" : "", bad_tot ? " accumulator" : "");
		std::printf("      port ret=%d  ref ret=%d\n", rp, rr);
		std::printf("      port total=%08x  ref total=%08x\n", tp, tr);
		dump("port buf", bp);
		dump("ref  buf", br);
	}
}

/* ------------------------------------------------------------------ */
/* input plumbing                                                      */

static void
set_content(const unsigned char *d, std::size_t n)
{
	if (ftruncate(g_tmpfd, 0) != 0)
		fatal("ftruncate");
	if (lseek(g_tmpfd, 0, SEEK_SET) == (off_t)-1)
		fatal("lseek");
	std::size_t off = 0;
	while (off < n) {
		ssize_t w = write(g_tmpfd, d + off, n - off);
		if (w <= 0)
			fatal("write");
		off += static_cast<std::size_t>(w);
	}
}

static void
data_case(const char *kind, long idx, const unsigned char *d, std::size_t n)
{
	set_content(d, n);
	do_case(kind, idx, static_cast<long long>(n), g_tmpfd, true);
}

static void
error_case(long idx)
{
	do_case("error-fd", idx, -1, g_badfd, false);
}

/* ------------------------------------------------------------------ */
/* deterministic PRNG (xorshift64*, fixed seed)                        */

static uint64_t rng_state = 0x00b0054100000001ULL;

static uint32_t
rnd(void)
{
	rng_state ^= rng_state << 13;
	rng_state ^= rng_state >> 7;
	rng_state ^= rng_state << 17;
	return static_cast<uint32_t>(rng_state >> 32);
}

static unsigned char
rnd_byte(void)
{
	uint32_t r = rnd();

	switch (r & 7) {
	case 0:
		return 0x00;
	case 1:
		return 0xff;
	case 2:
		return 0x80;
	case 3:
		return 0x7f;
	case 4:
		/* guarantee the high bit, i.e. a negative plain char */
		return static_cast<unsigned char>(0x80 | ((r >> 8) & 0x7f));
	default:
		return static_cast<unsigned char>((r >> 8) & 0xff);
	}
}

/* ------------------------------------------------------------------ */

static const std::size_t MAXN = 3 * static_cast<std::size_t>(BUFSIZ) + 64;
static unsigned char data[MAXN];

static void
fill_const(std::size_t n, unsigned char v)
{
	std::memset(data, v, n);
}

static void
fill_pattern(std::size_t n, unsigned int mul, unsigned int add)
{
	for (std::size_t i = 0; i < n; i++)
		data[i] = static_cast<unsigned char>(i * mul + add);
}

static void
fill_random(std::size_t n)
{
	for (std::size_t i = 0; i < n; i++)
		data[i] = rnd_byte();
}

int
main(void)
{
	char tmpl[] = "/tmp/pbsd_b0054s1_XXXXXX";

	rng_state = 0x00b0054100000001ULL;

	g_tmpfd = mkstemp(tmpl);
	if (g_tmpfd < 0)
		fatal("mkstemp");
	if (unlink(tmpl) != 0)
		fatal("unlink");

	/* write-only descriptor: read(2) fails with EBADF -> the nr < 0 path */
	g_badfd = open("/dev/null", O_WRONLY);
	if (g_badfd < 0)
		fatal("open /dev/null");

	long id = 0;
	const std::size_t B = static_cast<std::size_t>(BUFSIZ);

	/* ---- hand-written edge cases ---------------------------------- */

	/* empty input: no loop iterations at all */
	data_case("empty", id++, data, 0);

	/* every possible single byte, including 0x00, 0x7f, 0x80, 0xff */
	for (unsigned v = 0; v < 256; v++) {
		data[0] = static_cast<unsigned char>(v);
		data_case("single-byte", static_cast<long>(v), data, 1);
	}

	/* two bytes: ordering matters, so a reversed pointer walk shows up */
	{
		static const unsigned char pairs[][2] = {
			{ 0x00, 0xff }, { 0xff, 0x00 }, { 0x80, 0x7f },
			{ 0x7f, 0x80 }, { 0x01, 0x02 }, { 0x02, 0x01 },
			{ 0x00, 0x00 }, { 0xff, 0xff },
		};
		for (std::size_t i = 0; i < sizeof(pairs) / sizeof(pairs[0]); i++) {
			data[0] = pairs[i][0];
			data[1] = pairs[i][1];
			data_case("pair", static_cast<long>(i), data, 2);
		}
	}

	/* NUL-heavy and 0xff-heavy runs of assorted lengths */
	{
		static const std::size_t runs[] = { 3, 7, 8, 15, 16, 63, 64,
		    127, 128, 255, 256, 257, 511, 512, 1023, 1024, 4095, 4096 };
		for (std::size_t i = 0; i < sizeof(runs) / sizeof(runs[0]); i++) {
			fill_const(runs[i], 0x00);
			data_case("nul-run", static_cast<long>(runs[i]), data, runs[i]);
			fill_const(runs[i], 0xff);
			data_case("ff-run", static_cast<long>(runs[i]), data, runs[i]);
			fill_const(runs[i], 0x80);
			data_case("80-run", static_cast<long>(runs[i]), data, runs[i]);
			fill_const(runs[i], 0x7f);
			data_case("7f-run", static_cast<long>(runs[i]), data, runs[i]);
		}
	}

	/* the whole byte range, forwards and backwards, plus the high half */
	{
		for (unsigned i = 0; i < 256; i++)
			data[i] = static_cast<unsigned char>(i);
		data_case("ramp-up", id++, data, 256);
		for (unsigned i = 0; i < 256; i++)
			data[i] = static_cast<unsigned char>(255 - i);
		data_case("ramp-down", id++, data, 256);
		for (unsigned i = 0; i < 128; i++)
			data[i] = static_cast<unsigned char>(0x80 + i);
		data_case("high-bit", id++, data, 128);
		for (unsigned i = 0; i < 128; i++)
			data[i] = static_cast<unsigned char>(i);
		data_case("low-bit", id++, data, 128);
	}

	/* single high-bit byte embedded in NULs at every position of 64 */
	for (std::size_t pos = 0; pos < 64; pos++) {
		fill_const(64, 0x00);
		data[pos] = 0xff;
		data_case("embedded-ff", static_cast<long>(pos), data, 64);
	}

	/* read(2) chunk boundaries: BUFSIZ-2 .. BUFSIZ+2, 2*BUFSIZ+-1, 3*BUFSIZ+7 */
	{
		const std::size_t sizes[] = {
			B - 2, B - 1, B, B + 1, B + 2,
			2 * B - 2, 2 * B - 1, 2 * B, 2 * B + 1, 2 * B + 2,
			3 * B - 1, 3 * B, 3 * B + 1, 3 * B + 7,
		};
		for (std::size_t i = 0; i < sizeof(sizes) / sizeof(sizes[0]); i++) {
			fill_pattern(sizes[i], 31u, 7u);
			data_case("chunk-boundary",
			    static_cast<long>(sizes[i]), data, sizes[i]);
			fill_const(sizes[i], 0x00);
			data_case("chunk-boundary-nul",
			    static_cast<long>(sizes[i]), data, sizes[i]);
			fill_const(sizes[i], 0xff);
			data_case("chunk-boundary-ff",
			    static_cast<long>(sizes[i]), data, sizes[i]);
		}
	}

	/* every length 0..300, so no off-by-one in len survives */
	for (std::size_t n = 0; n <= 300; n++) {
		fill_pattern(n, 137u, 11u);
		data_case("length-sweep", static_cast<long>(n), data, n);
	}

	/*
	 * The error return leaves crc32_total complemented.  Hit it on its own,
	 * twice in a row, and sandwiched between real inputs so the corrupted
	 * accumulator has to propagate identically through later calls.
	 */
	error_case(id++);
	fill_pattern(37, 91u, 3u);
	data_case("after-error", id++, data, 37);
	error_case(id++);
	error_case(id++);
	fill_pattern(0, 1u, 0u);
	data_case("empty-after-error", id++, data, 0);
	fill_const(1, 0x80);
	data_case("single-after-error", id++, data, 1);
	error_case(id++);
	fill_pattern(B + 1, 17u, 5u);
	data_case("big-after-error", id++, data, B + 1);

	std::printf("edge cases done: %lu crc32 comparisons, %lu failures\n",
	    st_crc32.cases, st_crc32.fails);

	/* ---- fixed-seed randomised sweep ------------------------------ */

	const long ITERS = 200000;
	for (long i = 0; i < ITERS; i++) {
		if ((i % 5000) == 4999) {
			error_case(i);
			continue;
		}

		std::size_t n;
		uint32_t sel = rnd() % 1000u;
		if (sel < 700)
			n = rnd() % 33u;			/* 0..32 */
		else if (sel < 940)
			n = rnd() % 200u;			/* 0..199 */
		else if (sel < 990)
			n = rnd() % 2048u;
		else if (sel < 998)
			n = B - 3 + (rnd() % 7u);		/* around BUFSIZ */
		else
			n = 2 * B - 3 + (rnd() % 7u);		/* around 2*BUFSIZ */

		fill_random(n);
		data_case("random", i, data, n);
	}

	/* ---- report --------------------------------------------------- */

	std::printf("\n");
	std::printf("%-28s %12s %12s  %s\n", "function", "cases", "failures",
	    "result");
	std::printf("%-28s %12s %12s  %s\n", "----------------------------",
	    "------------", "------------", "------");
	const Stat *rows[] = { &st_crc32, &st_total };
	unsigned long total_fail = 0;
	for (std::size_t i = 0; i < sizeof(rows) / sizeof(rows[0]); i++) {
		std::printf("%-28s %12lu %12lu  %s\n", rows[i]->name,
		    rows[i]->cases, rows[i]->fails,
		    rows[i]->fails == 0 ? "PASS" : "FAIL");
		total_fail += rows[i]->fails;
	}
	std::printf("\n");

	close(g_tmpfd);
	close(g_badfd);

	if (total_fail != 0) {
		std::printf("b0054s1: FAILED (%lu mismatches)\n", total_fail);
		return 1;
	}
	std::printf("b0054s1: all cases matched\n");
	return 0;
}
