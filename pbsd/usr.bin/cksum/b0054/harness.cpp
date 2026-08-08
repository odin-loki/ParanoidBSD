/*
 * harness.cpp -- differential test for batch b0054 (crc32, crc).
 *
 * Each case feeds identical bytes to ref_crc32/ref_crc and port::crc32/port::crc
 * via separate pipe file descriptors.  Output pointers sit between 0x7f guard
 * bytes; the running totals are compared via delta from their pre-call values.
 */

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

import pbsd.usr.bin.cksum.b0054;

namespace port = pbsd::usr_bin_cksum::b0054;

extern "C" {
int ref_crc32(int, uint32_t *, off_t *);
int ref_crc(int, uint32_t *, off_t *);
uint32_t ref_crc32_total_value(void);
uint32_t ref_crc_total_value(void);
}

struct Stat {
	const char *name;
	long cases;
	long fails;
};

static Stat st_crc32 = { "crc32", 0, 0 };
static Stat st_crc = { "crc", 0, 0 };

static long mismatch_prints = 0;

struct GuardedU32 {
	uint8_t pre[4];
	uint32_t val;
	uint8_t post[4];

	void init(void)
	{
		std::memset(pre, 0x7f, sizeof(pre));
		std::memset(post, 0x7f, sizeof(post));
		val = 0x7f7f7f7fU;
	}

	bool guards_ok(void) const
	{
		for (size_t i = 0; i < sizeof(pre); ++i)
			if (pre[i] != 0x7f)
				return false;
		for (size_t i = 0; i < sizeof(post); ++i)
			if (post[i] != 0x7f)
				return false;
		return true;
	}

	uint32_t *ptr(void)
	{
		return &val;
	}
};

struct GuardedOff {
	uint8_t pre[4];
	off_t val;
	uint8_t post[4];

	void init(void)
	{
		std::memset(pre, 0x7f, sizeof(pre));
		std::memset(post, 0x7f, sizeof(post));
		val = static_cast<off_t>(0x7f7f7f7f7f7f7f7fLL);
	}

	bool guards_ok(void) const
	{
		for (size_t i = 0; i < sizeof(pre); ++i)
			if (pre[i] != 0x7f)
				return false;
		for (size_t i = 0; i < sizeof(post); ++i)
			if (post[i] != 0x7f)
				return false;
		return true;
	}

	off_t *ptr(void)
	{
		return &val;
	}
};

struct Outcome {
	int ret;
	uint32_t cval;
	off_t clen;
	uint32_t total;
	bool cval_written;
	bool clen_written;
};

static int make_pipe_fd(const uint8_t *data, size_t len)
{
	int fds[2];

	if (pipe(fds) != 0)
		return -1;
	if (len > 0) {
		const uint8_t *p = data;
		size_t left = len;
		while (left > 0) {
			ssize_t n = write(fds[1], p, left);
			if (n <= 0) {
				close(fds[0]);
				close(fds[1]);
				return -1;
			}
			p += static_cast<size_t>(n);
			left -= static_cast<size_t>(n);
		}
	}
	close(fds[1]);
	return fds[0];
}

static int make_bad_fd(void)
{
	int fds[2];

	if (pipe(fds) != 0)
		return -1;
	close(fds[0]);
	close(fds[1]);
	return fds[0];
}

static Outcome run_ref_crc32(int fd, bool expect_write)
{
	GuardedU32 gc;
	GuardedOff gl;
	uint32_t tot_before;
	Outcome o;

	gc.init();
	gl.init();
	tot_before = ref_crc32_total_value();
	o.ret = ref_crc32(fd, gc.ptr(), gl.ptr());
	o.total = ref_crc32_total_value();
	o.cval = gc.val;
	o.clen = gl.val;
	o.cval_written = expect_write && o.ret == 0;
	o.clen_written = expect_write && o.ret == 0;
	if (!gc.guards_ok() || !gl.guards_ok())
		o.ret = -2;
	(void)tot_before;
	return o;
}

static Outcome run_port_crc32(int fd, bool expect_write)
{
	GuardedU32 gc;
	GuardedOff gl;
	uint32_t tot_before;
	Outcome o;

	gc.init();
	gl.init();
	tot_before = port::crc32_total_value();
	o.ret = port::crc32(fd, gc.ptr(), gl.ptr());
	o.total = port::crc32_total_value();
	o.cval = gc.val;
	o.clen = gl.val;
	o.cval_written = expect_write && o.ret == 0;
	o.clen_written = expect_write && o.ret == 0;
	if (!gc.guards_ok() || !gl.guards_ok())
		o.ret = -2;
	(void)tot_before;
	return o;
}

static Outcome run_ref_crc(int fd, bool expect_write)
{
	GuardedU32 gc;
	GuardedOff gl;
	uint32_t tot_before;
	Outcome o;

	gc.init();
	gl.init();
	tot_before = ref_crc_total_value();
	o.ret = ref_crc(fd, gc.ptr(), gl.ptr());
	o.total = ref_crc_total_value();
	o.cval = gc.val;
	o.clen = gl.val;
	o.cval_written = expect_write && o.ret == 0;
	o.clen_written = expect_write && o.ret == 0;
	if (!gc.guards_ok() || !gl.guards_ok())
		o.ret = -2;
	(void)tot_before;
	return o;
}

static Outcome run_port_crc(int fd, bool expect_write)
{
	GuardedU32 gc;
	GuardedOff gl;
	uint32_t tot_before;
	Outcome o;

	gc.init();
	gl.init();
	tot_before = port::crc_total_value();
	o.ret = port::crc(fd, gc.ptr(), gl.ptr());
	o.total = port::crc_total_value();
	o.cval = gc.val;
	o.clen = gl.val;
	o.cval_written = expect_write && o.ret == 0;
	o.clen_written = expect_write && o.ret == 0;
	if (!gc.guards_ok() || !gl.guards_ok())
		o.ret = -2;
	(void)tot_before;
	return o;
}

static bool same_outcome(const Outcome &a, const Outcome &b)
{
	if (a.ret != b.ret)
		return false;
	if (a.total != b.total)
		return false;
	if (a.cval_written != b.cval_written || a.clen_written != b.clen_written)
		return false;
	if (a.cval_written && a.cval != b.cval)
		return false;
	if (a.clen_written && a.clen != b.clen)
		return false;
	return true;
}

static void report(const char *fn, const char *tag, const Outcome &p,
    const Outcome &r)
{
	if (mismatch_prints++ < 20) {
		std::printf("  MISMATCH [%s] %s: port ret=%d cval=0x%08x clen=%lld "
		    "total=0x%08x | ref ret=%d cval=0x%08x clen=%lld total=0x%08x\n",
		    tag, fn, p.ret, p.cval, (long long)p.clen, p.total,
		    r.ret, r.cval, (long long)r.clen, r.total);
	}
}

static void check_crc32(Stat &st, const char *tag, const uint8_t *data,
    size_t len, bool bad_fd)
{
	int fd_ref = bad_fd ? make_bad_fd() : make_pipe_fd(data, len);
	int fd_port = bad_fd ? make_bad_fd() : make_pipe_fd(data, len);
	Outcome r, p;
	bool expect_write = !bad_fd;

	++st.cases;
	if (fd_ref < 0 || fd_port < 0) {
		++st.fails;
		return;
	}

	r = run_ref_crc32(fd_ref, expect_write);
	p = run_port_crc32(fd_port, expect_write);
	close(fd_ref);
	close(fd_port);

	if (!same_outcome(p, r)) {
		++st.fails;
		report(st.name, tag, p, r);
	}
}

static void check_crc(Stat &st, const char *tag, const uint8_t *data,
    size_t len, bool bad_fd)
{
	int fd_ref = bad_fd ? make_bad_fd() : make_pipe_fd(data, len);
	int fd_port = bad_fd ? make_bad_fd() : make_pipe_fd(data, len);
	Outcome r, p;
	bool expect_write = !bad_fd;

	++st.cases;
	if (fd_ref < 0 || fd_port < 0) {
		++st.fails;
		return;
	}

	r = run_ref_crc(fd_ref, expect_write);
	p = run_port_crc(fd_port, expect_write);
	close(fd_ref);
	close(fd_port);

	if (!same_outcome(p, r)) {
		++st.fails;
		report(st.name, tag, p, r);
	}
}

static uint32_t rng_state = 0x243f6a88U;

static uint32_t rng_next(void)
{
	uint32_t x = rng_state;

	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	rng_state = x;
	return x;
}

static uint8_t rng_byte(void)
{
	return static_cast<uint8_t>(rng_next() & 0xffU);
}

static void fill_pattern(std::vector<uint8_t> &v, size_t len, unsigned pat)
{
	v.resize(len);
	for (size_t i = 0; i < len; ++i)
		v[i] = static_cast<uint8_t>((pat + i) & 0xffU);
}

static void run_edge_cases(void)
{
	std::vector<uint8_t> data;
	static const size_t bounds[] = {
		0, 1, 2, 3, 15, 16, 17, 31, 32, 33, 63, 64, 65,
		127, 128, 129, 254, 255, 256, 257, 511, 512, 513,
		1023, 1024, 1025, 4095, 4096, 4097,
		8191, 8192, 8193, 16383, 16384, 16385,
		32767, 32768, 32769, 65535, 65536, 65537, 131072
	};

	check_crc32(st_crc32, "bad-fd", nullptr, 0, true);
	check_crc(st_crc, "bad-fd", nullptr, 0, true);

	check_crc32(st_crc32, "empty", nullptr, 0, false);
	check_crc(st_crc, "empty", nullptr, 0, false);

	data = { 0x00 };
	check_crc32(st_crc32, "single-nul", data.data(), 1, false);
	check_crc(st_crc, "single-nul", data.data(), 1, false);

	data = { 0x41 };
	check_crc32(st_crc32, "single-A", data.data(), 1, false);
	check_crc(st_crc, "single-A", data.data(), 1, false);

	for (unsigned b = 0x80; b != 0; b += 0x11) {
		data = { static_cast<uint8_t>(b) };
		char tag[32];
		std::snprintf(tag, sizeof(tag), "single-0x%02x", b);
		check_crc32(st_crc32, tag, data.data(), 1, false);
		check_crc(st_crc, tag, data.data(), 1, false);
	}

	data = { 0x00, 0x00, 0x00, 0x00 };
	check_crc32(st_crc32, "nul4", data.data(), data.size(), false);
	check_crc(st_crc, "nul4", data.data(), data.size(), false);

	data.assign(256, 0xff);
	check_crc32(st_crc32, "ff256", data.data(), data.size(), false);
	check_crc(st_crc, "ff256", data.data(), data.size(), false);

	data.assign(256, 0x00);
	for (int i = 0; i < 256; ++i)
		data[static_cast<size_t>(i)] = static_cast<uint8_t>(i);
	check_crc32(st_crc32, "iota256", data.data(), data.size(), false);
	check_crc(st_crc, "iota256", data.data(), data.size(), false);

	for (size_t i = 0; i < sizeof(bounds) / sizeof(bounds[0]); ++i) {
		size_t n = bounds[i];
		char tag[48];

		fill_pattern(data, n, 0xa5);
		std::snprintf(tag, sizeof(tag), "pat-a5-%zu", n);
		check_crc32(st_crc32, tag, data.data(), n, false);
		check_crc(st_crc, tag, data.data(), n, false);

		fill_pattern(data, n, 0x80);
		std::snprintf(tag, sizeof(tag), "pat-80-%zu", n);
		check_crc32(st_crc32, tag, data.data(), n, false);
		check_crc(st_crc, tag, data.data(), n, false);
	}
}

static void run_random_sweep(void)
{
	std::vector<uint8_t> data;

	for (long i = 0; i < 200000; ++i) {
		size_t len = static_cast<size_t>(rng_next() % 70000U);
		char tag[32];

		data.resize(len);
		for (size_t j = 0; j < len; ++j)
			data[j] = rng_byte();

		std::snprintf(tag, sizeof(tag), "rng-%ld", i);
		check_crc32(st_crc32, tag, data.data(), len, false);
		check_crc(st_crc, tag, data.data(), len, false);
	}
}

int
main(void)
{
	run_edge_cases();
	run_random_sweep();

	std::printf("\n%-8s  %10s  %10s\n", "function", "cases", "failures");
	std::printf("%-8s  %10ld  %10ld\n", st_crc32.name, st_crc32.cases,
	    st_crc32.fails);
	std::printf("%-8s  %10ld  %10ld\n", st_crc.name, st_crc.cases,
	    st_crc.fails);

	if (st_crc32.fails != 0 || st_crc.fails != 0)
		return 1;
	return 0;
}
