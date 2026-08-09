/*
 * harness.cpp -- differential test for batch b0164.
 *
 * Compares the C++23 port in pbsd.usr.bin.false.b0164 against the untouched
 * C reference in oracle.c.  Exit status is 0 only when every single case
 * matched.
 *
 * The batch contains exactly one function, main(void): it takes no arguments
 * and writes to no caller-supplied buffer.  To keep the buffer discipline
 * demanded of this harness meaningful, every case allocates two scratch
 * buffers, fills both with the guard byte 0x7f (or another chosen fill for
 * the nominal window), deposits the value returned by the port into one and
 * the value returned by the oracle into the other, and then compares the
 * buffers in their entirety -- including every byte past the nominal write
 * window.  A difference in the returned value, or a stray write by either
 * implementation, therefore shows up as a buffer mismatch as well as a
 * return-value mismatch.
 */

import pbsd.usr.bin.false.b0164;

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

extern "C" int ref_main(void);

namespace port = pbsd::usr_bin_false::b0164;

namespace {

constexpr unsigned char GUARD = 0x7f;
constexpr std::size_t MAXBUF = 96;

struct FnStats {
	const char *name;
	unsigned long long cases;
	unsigned long long failures;
};

FnStats stats_main = { "main", 0, 0 };

unsigned long long reported;

void
report(const char *label, const char *why, int pr, int rr)
{
	if (reported++ < 20)
		std::printf("FAIL [%s] %s: port=%d ref=%d\n", label, why, pr,
		    rr);
}

/*
 * One differential case.
 *
 * bufsz	size of the nominal window at the front of the buffer;
 *		everything from bufsz to MAXBUF stays pure guard bytes.
 * off		offset at which the returned value is deposited.
 * fill		byte the nominal window is preloaded with.
 * repeats	how many times each implementation is called before its
 *		result is taken; catches any hidden state.
 * port_first	call order.
 */
bool
one_case(FnStats &st, std::size_t bufsz, std::size_t off, unsigned char fill,
    unsigned repeats, bool port_first, const char *label)
{
	unsigned char pbuf[MAXBUF];
	unsigned char rbuf[MAXBUF];

	st.cases++;

	std::memset(pbuf, GUARD, sizeof(pbuf));
	std::memset(rbuf, GUARD, sizeof(rbuf));
	if (bufsz > 0) {
		std::memset(pbuf, fill, bufsz);
		std::memset(rbuf, fill, bufsz);
	}

	int pr = 0;
	int rr = 0;
	if (port_first) {
		for (unsigned i = 0; i < repeats; i++)
			pr = port::main();
		for (unsigned i = 0; i < repeats; i++)
			rr = ref_main();
	} else {
		for (unsigned i = 0; i < repeats; i++)
			rr = ref_main();
		for (unsigned i = 0; i < repeats; i++)
			pr = port::main();
	}

	const unsigned char *pp = pbuf;
	const unsigned char *rp = rbuf;
	if (off < bufsz) {
		std::size_t n = sizeof(int);
		if (n > bufsz - off)
			n = bufsz - off;
		std::memcpy(pbuf + off, &pr, n);
		std::memcpy(rbuf + off, &rr, n);
		pp = pbuf + off;
		rp = rbuf + off;
	}

	bool ok = true;

	if (pr != rr) {
		report(label, "return value differs", pr, rr);
		ok = false;
	}
	if ((pr != 0) != (rr != 0)) {
		report(label, "exit-status truth differs", pr, rr);
		ok = false;
	}
	if ((pp - pbuf) != (rp - rbuf)) {
		report(label, "result offset differs", (int)(pp - pbuf),
		    (int)(rp - rbuf));
		ok = false;
	}
	if (std::memcmp(pbuf, rbuf, sizeof(pbuf)) != 0) {
		report(label, "buffer differs", pr, rr);
		ok = false;
	}

	if (!ok)
		st.failures++;
	return ok;
}

/*
 * Repeated-call drive: the moral equivalent of running a stateful iterator to
 * exhaustion.  The two implementations are interleaved and compared after
 * every single iteration, buffers included.
 */
bool
drive_case(FnStats &st, unsigned iterations, const char *label)
{
	unsigned char pbuf[MAXBUF];
	unsigned char rbuf[MAXBUF];
	bool all_ok = true;

	std::memset(pbuf, GUARD, sizeof(pbuf));
	std::memset(rbuf, GUARD, sizeof(rbuf));

	for (unsigned i = 0; i < iterations; i++) {
		st.cases++;

		int pr = port::main();
		int rr = ref_main();

		std::size_t off = (i * sizeof(int)) %
		    (MAXBUF - sizeof(int) + 1);
		std::memcpy(pbuf + off, &pr, sizeof(int));
		std::memcpy(rbuf + off, &rr, sizeof(int));

		bool ok = true;
		if (pr != rr) {
			report(label, "return value differs", pr, rr);
			ok = false;
		}
		if (std::memcmp(pbuf, rbuf, sizeof(pbuf)) != 0) {
			report(label, "buffer differs", pr, rr);
			ok = false;
		}
		if (!ok) {
			st.failures++;
			all_ok = false;
		}
	}
	return all_ok;
}

std::uint64_t rng_state;

std::uint64_t
next_rand(void)
{
	std::uint64_t x = rng_state;

	x ^= x << 13;
	x ^= x >> 7;
	x ^= x << 17;
	rng_state = x;
	return x;
}

} // namespace

int
main(void)
{
	static const unsigned char fills[] = {
		0x00, 0x01, 0x7f, 0x80, 0x81, 0xfe, 0xff, 0x55, 0xaa
	};
	static const std::size_t sizes[] = {
		0, 1, 2, 3, 4, 5, 7, 8, 15, 16, 31, 32, 63, 64, MAXBUF - 1,
		MAXBUF
	};

	/* Hand-written edge cases: empty and single-byte windows, NUL-heavy
	   and high-bit fills, and both sides of every offset boundary. */
	for (std::size_t si = 0; si < sizeof(sizes) / sizeof(sizes[0]); si++) {
		for (std::size_t fi = 0;
		    fi < sizeof(fills) / sizeof(fills[0]); fi++) {
			std::size_t bufsz = sizes[si];
			std::size_t offs[] = {
				0,
				bufsz,
				bufsz > 0 ? bufsz - 1 : 0,
				bufsz >= sizeof(int) ? bufsz - sizeof(int) : 0,
				bufsz >= sizeof(int) ?
				    bufsz - sizeof(int) + 1 : 0,
				MAXBUF - 1,
				MAXBUF
			};

			for (std::size_t oi = 0;
			    oi < sizeof(offs) / sizeof(offs[0]); oi++) {
				one_case(stats_main, bufsz, offs[oi],
				    fills[fi], 1, true, "edge/port-first");
				one_case(stats_main, bufsz, offs[oi],
				    fills[fi], 1, false, "edge/ref-first");
				one_case(stats_main, bufsz, offs[oi],
				    fills[fi], 3, true, "edge/repeat3");
			}
		}
	}

	drive_case(stats_main, 4096, "drive/exhaustion");

	/* Fixed-seed randomised sweep. */
	rng_state = 0x9e3779b97f4a7c15ULL;
	for (unsigned long i = 0; i < 200000UL; i++) {
		std::uint64_t r = next_rand();
		std::size_t bufsz = (std::size_t)(r % (MAXBUF + 1));
		std::size_t off = (std::size_t)((r >> 8) % (MAXBUF + 1));
		unsigned char fill = (unsigned char)((r >> 16) & 0xff);
		unsigned repeats = (unsigned)((r >> 24) % 4) + 1;
		bool port_first = ((r >> 32) & 1) != 0;

		one_case(stats_main, bufsz, off, fill, repeats, port_first,
		    "random");
	}

	std::printf("\nPBSD batch b0164 differential test "
	    "(oracle.c is the specification)\n\n");
	std::printf("%-12s %14s %14s %8s\n", "function", "cases", "failures",
	    "result");
	std::printf("%-12s %14llu %14llu %8s\n", stats_main.name,
	    stats_main.cases, stats_main.failures,
	    stats_main.failures == 0 ? "PASS" : "FAIL");

	unsigned long long total_cases = stats_main.cases;
	unsigned long long total_failures = stats_main.failures;

	std::printf("%-12s %14llu %14llu %8s\n", "TOTAL", total_cases,
	    total_failures, total_failures == 0 ? "PASS" : "FAIL");

	return total_failures == 0 ? 0 : 1;
}
