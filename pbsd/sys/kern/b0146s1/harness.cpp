// Differential test for PBSD batch b0146s1 (sys_getrandom).

import pbsd.sys.kern.b0146s1;

#include <cerrno>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace port = pbsd::sys_kern::b0146s1;

#define GUARD     0x7f
#define PAD       32u
#define SWEEP     200000L
#define MAX_PRINT 12

struct stat_row {
	const char *name;
	long cases;
	long failures;
	long printed;
};

static stat_row rows[] = {
	{ "kern_getrandom", 0, 0, 0 },
	{ "sys_getrandom",  0, 0, 0 },
};

enum { R_KERN = 0, R_SYS = 1 };

static uint64_t rng_state = 0x00b0146001faceULL;

static inline uint64_t
rnd64(void)
{
	uint64_t z = (rng_state += 0x9e3779b97f4a7c15ULL);

	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return (z ^ (z >> 31));
}

static inline uint32_t
rnd32(void)
{
	return ((uint32_t)(rnd64() >> 32));
}

static void
fail_row(int row, const char *label, const char *detail)
{
	stat_row &r = rows[row];

	r.failures++;
	if (r.printed < MAX_PRINT) {
		r.printed++;
		std::printf("  FAIL %-24s %-24s %s\n", r.name, label, detail);
	}
}

static void
case_row(int row)
{
	rows[row].cases++;
}

extern "C" {
struct thread {
	long td_retval[2];
};
struct getrandom_args {
	void *buf;
	size_t buflen;
	unsigned int flags;
};

int ref_kern_getrandom(struct thread *, void *, size_t, unsigned int);
int ref_sys_getrandom(struct thread *, struct getrandom_args *);

void oracle_read_random_reset(void);
void oracle_read_random_configure(int, int, ssize_t);
}

static bool
bufs_match(const unsigned char *a, const unsigned char *b, std::size_t n)
{
	for (std::size_t i = 0; i < n; i++) {
		if (a[i] != b[i])
			return (false);
	}
	return (true);
}

static void
test_kern_getrandom_one(std::size_t buflen, unsigned int flags, int rr_err,
    int rr_block, ssize_t rr_xfer)
{
	case_row(R_KERN);

	const std::size_t total = buflen + PAD * 2;
	unsigned char *pb = static_cast<unsigned char *>(std::malloc(total));
	unsigned char *rb = static_cast<unsigned char *>(std::malloc(total));

	std::memset(pb, GUARD, total);
	std::memset(rb, GUARD, total);

	port::thread ptd = {};
	struct thread rtd = {};

	port::read_random_reset();
	oracle_read_random_reset();
	port::read_random_configure(rr_err, rr_block, rr_xfer);
	oracle_read_random_configure(rr_err, rr_block, rr_xfer);

	port::getrandom_args puap = { pb + PAD, buflen, flags };

	int pr = port::sys_getrandom(&ptd, &puap);
	int rr = ref_kern_getrandom(&rtd, rb + PAD, buflen, flags);

	if (pr != rr)
		fail_row(R_KERN, "retval", "error code");
	if (ptd.td_retval[0] != rtd.td_retval[0])
		fail_row(R_KERN, "td_retval", "bytes returned");
	if (!bufs_match(pb, rb, total))
		fail_row(R_KERN, "buffer", "guard/data mismatch");

	std::free(pb);
	std::free(rb);
}

static void
test_sys_getrandom_one(std::size_t buflen, unsigned int flags, int rr_err,
    int rr_block, ssize_t rr_xfer)
{
	case_row(R_SYS);

	const std::size_t total = buflen + PAD * 2;
	unsigned char *pb = static_cast<unsigned char *>(std::malloc(total));
	unsigned char *rb = static_cast<unsigned char *>(std::malloc(total));

	std::memset(pb, GUARD, total);
	std::memset(rb, GUARD, total);

	port::thread ptd = {};
	struct thread rtd = {};
	port::getrandom_args puap = { pb + PAD, buflen, flags };
	struct getrandom_args ruap = { rb + PAD, buflen, flags };

	port::read_random_reset();
	oracle_read_random_reset();
	port::read_random_configure(rr_err, rr_block, rr_xfer);
	oracle_read_random_configure(rr_err, rr_block, rr_xfer);

	int pr = port::sys_getrandom(&ptd, &puap);
	int rr = ref_sys_getrandom(&rtd, &ruap);

	if (pr != rr)
		fail_row(R_SYS, "retval", "error code");
	if (ptd.td_retval[0] != rtd.td_retval[0])
		fail_row(R_SYS, "td_retval", "bytes returned");
	if (!bufs_match(pb, rb, total))
		fail_row(R_SYS, "buffer", "guard/data mismatch");

	std::free(pb);
	std::free(rb);
}

static void
test_hand(void)
{
	const unsigned int flag_sets[] = {
		0,
		0x0001, /* GRND_NONBLOCK */
		0x0002, /* GRND_RANDOM */
		0x0003,
		0x0004, /* GRND_INSECURE */
		0x0005,
		0x0006,
		0x0007,
		0x0008, /* invalid bit */
		0x0010,
		0x80000000U,
		0x80000007U,
	};
	const std::size_t lens[] = {
		0, 1, 2, 3, 7, 8, 15, 16, 31, 32, 63, 64, 127, 128, 255, 256,
		1024, 4096,
		(size_t)INT_MAX,
		(size_t)INT_MAX + 1,
		(size_t)INT_MAX + 2,
		0x80000000UL,
		0x80000001UL,
	};

	for (std::size_t len : lens) {
		for (unsigned int fl : flag_sets) {
			test_kern_getrandom_one(len, fl, 0, 0, -1);
			test_sys_getrandom_one(len, fl, 0, 0, -1);
		}
	}

	/* EINVAL from read_random_uio */
	test_kern_getrandom_one(64, 0, EINVAL, 0, -1);
	test_sys_getrandom_one(64, 0, EINVAL, 0, -1);

	/* EWOULDBLOCK on nonblocking */
	test_kern_getrandom_one(64, 0x0001, 0, 1, -1);
	test_sys_getrandom_one(64, 0x0001, 0, 1, -1);

	/* GRND_INSECURE implies nonblock: block path */
	test_kern_getrandom_one(64, 0x0004, 0, 1, -1);
	test_sys_getrandom_one(64, 0x0004, 0, 1, -1);

	/* GRND_INSECURE | GRND_NONBLOCK */
	test_kern_getrandom_one(64, 0x0005, 0, 1, -1);
	test_sys_getrandom_one(64, 0x0005, 0, 1, -1);

	/* Partial transfer: zero bytes */
	test_kern_getrandom_one(64, 0, 0, 0, 0);
	test_sys_getrandom_one(64, 0, 0, 0, 0);

	/* Partial transfer: short read */
	test_kern_getrandom_one(64, 0, 0, 0, 32);
	test_sys_getrandom_one(64, 0, 0, 0, 32);
	test_kern_getrandom_one(1, 0, 0, 0, 32);
	test_sys_getrandom_one(1, 0, 0, 0, 32);

	/* Single byte with high-bit pattern in buffer pre-fill */
	{
		const std::size_t len = 1;
		const std::size_t total = len + PAD * 2;
		unsigned char *pb = static_cast<unsigned char *>(std::malloc(total));
		unsigned char *rb = static_cast<unsigned char *>(std::malloc(total));

		for (int ri = 0; ri < 2; ri++) {
			int row = (ri == 0) ? R_KERN : R_SYS;
			case_row(row);
			std::memset(pb, GUARD, total);
			std::memset(rb, GUARD, total);
			*(pb + PAD) = 0x80;
			*(rb + PAD) = 0x80;

			port::thread ptd = {};
			struct thread rtd = {};
			port::getrandom_args puap = { pb + PAD, len, 0 };
			struct getrandom_args ruap = { rb + PAD, len, 0 };

			port::read_random_reset();
			oracle_read_random_reset();

			int pr, rr;
			if (row == R_KERN) {
				pr = port::sys_getrandom(&ptd, &puap);
				rr = ref_kern_getrandom(&rtd, rb + PAD, len, 0);
			} else {
				pr = port::sys_getrandom(&ptd, &puap);
				rr = ref_sys_getrandom(&rtd, &ruap);
			}
			if (pr != rr || ptd.td_retval[0] != rtd.td_retval[0] ||
			    !bufs_match(pb, rb, total))
				fail_row(row, "high-bit", "mismatch");
		}
		std::free(pb);
		std::free(rb);
	}
}

static void
test_sweep(void)
{
	for (long i = 0; i < SWEEP; i++) {
		std::size_t len = rnd32() % 512;
		unsigned int flags = rnd32() & 0xff;
		int err = 0;
		int block = (rnd32() & 3) == 0 ? 1 : 0;
		ssize_t xfer = (rnd32() & 7) == 0 ? (ssize_t)(rnd32() % 256) : -1;

		if ((rnd32() % 50) == 0)
			err = EWOULDBLOCK;
		if ((rnd32() % 100) == 0)
			len = (size_t)INT_MAX + (rnd32() % 16);
		if ((rnd32() % 200) == 0)
			len = 0;

		test_kern_getrandom_one(len, flags, err, block, xfer);
		test_sys_getrandom_one(len, flags, err, block, xfer);
	}
}

int
main(void)
{
	test_hand();
	test_sweep();

	long total_cases = 0;
	long total_fail = 0;

	std::printf("\n%-24s %12s %12s\n", "function", "cases", "failures");
	for (const auto &r : rows) {
		std::printf("%-24s %12ld %12ld\n", r.name, r.cases, r.failures);
		total_cases += r.cases;
		total_fail += r.failures;
	}
	std::printf("%-24s %12ld %12ld\n", "TOTAL", total_cases, total_fail);

	return (total_fail == 0 ? 0 : 1);
}
