/*
 * Differential test harness for batch b0019 -- lib/libc/uuid/uuid_stream.c
 *
 * Every case is executed twice: once against the C++23 port and once against
 * the ref_ oracle compiled from the untouched C sources.  Encoders are given
 * two independent 0x7f-guarded buffers and the ENTIRE buffer is compared
 * afterwards, so a write that lands outside the nominal 16 byte window is a
 * failure.  Decoders are given two byte-identical input buffers and two
 * sentinel-filled uuid structures; both the resulting structures and both
 * input buffers are compared in full.
 */

#include <cstdint>
#include <cstdio>
#include <cstring>

import pbsd.lib.libc.uuid.b0019;

namespace P = pbsd::lib_libc_uuid::b0019;

/* Layout-compatible view of the oracle's uuid_t (C has no name mangling). */
extern "C" {
struct oracle_uuid {
	std::uint32_t	time_low;
	std::uint16_t	time_mid;
	std::uint16_t	time_hi_and_version;
	std::uint8_t	clock_seq_hi_and_reserved;
	std::uint8_t	clock_seq_low;
	std::uint8_t	node[6];
};

void ref_uuid_enc_le(void *buf, const struct oracle_uuid *uuid);
void ref_uuid_dec_le(const void *buf, struct oracle_uuid *uuid);
void ref_uuid_enc_be(void *buf, const struct oracle_uuid *uuid);
void ref_uuid_dec_be(const void *buf, struct oracle_uuid *uuid);
}

static_assert(sizeof(P::uuid) == 16, "port uuid must be 16 bytes");
static_assert(sizeof(oracle_uuid) == 16, "oracle uuid must be 16 bytes");
static_assert(P::_UUID_NODE_LEN == 6, "node length");

/* ------------------------------------------------------------------ */

static constexpr std::size_t BUFSZ = 64;
static constexpr std::size_t WINDOW = 16;
static constexpr std::uint8_t GUARD = 0x7f;
static constexpr std::uint8_t SENTINEL = 0x5a;

enum Fn { FN_ENC_LE = 0, FN_DEC_LE, FN_ENC_BE, FN_DEC_BE, FN_COUNT };

static const char *const fn_name[FN_COUNT] = {
	"uuid_enc_le", "uuid_dec_le", "uuid_enc_be", "uuid_dec_be"
};

static unsigned long long cases[FN_COUNT];
static unsigned long long fails[FN_COUNT];
static unsigned reported[FN_COUNT];

static void
hexdump(const char *tag, const std::uint8_t *p, std::size_t n)
{
	std::printf("      %-10s", tag);
	for (std::size_t i = 0; i < n; i++)
		std::printf("%02x", p[i]);
	std::printf("\n");
}

static void
fail(Fn f, const char *what, std::size_t off, const std::uint8_t in[WINDOW],
    const std::uint8_t *pa, std::size_t na, const std::uint8_t *pb,
    std::size_t nb)
{
	fails[f]++;
	if (reported[f]++ >= 5)
		return;
	std::printf("  FAIL %s: %s (off=%zu)\n", fn_name[f], what, off);
	hexdump("input", in, WINDOW);
	hexdump("port", pa, na);
	hexdump("oracle", pb, nb);
}

/* ------------------------------------------------------------------ */

/*
 * Build both uuid flavours from the same 16 logical bytes so that a single
 * "hot byte" walk exercises every field, every shift and every node index.
 */
static void
build(const std::uint8_t b[WINDOW], P::uuid &pu, oracle_uuid &ou)
{
	std::memset(&pu, SENTINEL, sizeof pu);
	std::memset(&ou, SENTINEL, sizeof ou);

	std::uint32_t tl = ((std::uint32_t)b[0] << 24) | ((std::uint32_t)b[1] << 16) |
	    ((std::uint32_t)b[2] << 8) | (std::uint32_t)b[3];
	std::uint16_t tm = (std::uint16_t)(((unsigned)b[4] << 8) | b[5]);
	std::uint16_t th = (std::uint16_t)(((unsigned)b[6] << 8) | b[7]);

	pu.time_low = tl;
	pu.time_mid = tm;
	pu.time_hi_and_version = th;
	pu.clock_seq_hi_and_reserved = b[8];
	pu.clock_seq_low = b[9];
	for (int i = 0; i < 6; i++)
		pu.node[i] = b[10 + i];

	ou.time_low = tl;
	ou.time_mid = tm;
	ou.time_hi_and_version = th;
	ou.clock_seq_hi_and_reserved = b[8];
	ou.clock_seq_low = b[9];
	for (int i = 0; i < 6; i++)
		ou.node[i] = b[10 + i];
}

static void
run_enc(Fn f, const std::uint8_t b[WINDOW], std::size_t off)
{
	std::uint8_t a[BUFSZ], c[BUFSZ];
	P::uuid pu;
	oracle_uuid ou;

	cases[f]++;

	std::memset(a, GUARD, BUFSZ);
	std::memset(c, GUARD, BUFSZ);
	build(b, pu, ou);

	P::uuid saved_pu = pu;
	oracle_uuid saved_ou = ou;

	if (f == FN_ENC_LE) {
		P::uuid_enc_le(a + off, &pu);
		ref_uuid_enc_le(c + off, &ou);
	} else {
		P::uuid_enc_be(a + off, &pu);
		ref_uuid_enc_be(c + off, &ou);
	}

	if (std::memcmp(a, c, BUFSZ) != 0) {
		fail(f, "output buffer mismatch", off, b, a, BUFSZ, c, BUFSZ);
		return;
	}
	/* The source uuid must not have been touched by either side. */
	if (std::memcmp(&pu, &saved_pu, sizeof pu) != 0 ||
	    std::memcmp(&ou, &saved_ou, sizeof ou) != 0 ||
	    std::memcmp(&pu, &ou, sizeof pu) != 0) {
		fail(f, "source uuid modified", off, b,
		    (const std::uint8_t *)&pu, sizeof pu,
		    (const std::uint8_t *)&ou, sizeof ou);
	}
}

static void
run_dec(Fn f, const std::uint8_t b[WINDOW], std::size_t off,
    const std::uint8_t bg[BUFSZ])
{
	std::uint8_t a[BUFSZ], c[BUFSZ], orig[BUFSZ];
	P::uuid pu;
	oracle_uuid ou;

	cases[f]++;

	std::memcpy(orig, bg, BUFSZ);
	std::memcpy(orig + off, b, WINDOW);
	std::memcpy(a, orig, BUFSZ);
	std::memcpy(c, orig, BUFSZ);

	std::memset(&pu, SENTINEL, sizeof pu);
	std::memset(&ou, SENTINEL, sizeof ou);

	if (f == FN_DEC_LE) {
		P::uuid_dec_le(a + off, &pu);
		ref_uuid_dec_le(c + off, &ou);
	} else {
		P::uuid_dec_be(a + off, &pu);
		ref_uuid_dec_be(c + off, &ou);
	}

	if (std::memcmp(&pu, &ou, sizeof pu) != 0) {
		fail(f, "decoded uuid mismatch", off, b,
		    (const std::uint8_t *)&pu, sizeof pu,
		    (const std::uint8_t *)&ou, sizeof ou);
		return;
	}
	if (std::memcmp(a, c, BUFSZ) != 0 || std::memcmp(a, orig, BUFSZ) != 0) {
		fail(f, "input buffer modified", off, b, a, BUFSZ, c, BUFSZ);
	}
}

static void
run_all(const std::uint8_t b[WINDOW], std::size_t off,
    const std::uint8_t bg[BUFSZ])
{
	run_enc(FN_ENC_LE, b, off);
	run_enc(FN_ENC_BE, b, off);
	run_dec(FN_DEC_LE, b, off, bg);
	run_dec(FN_DEC_BE, b, off, bg);
}

/* ------------------------------------------------------------------ */

static std::uint64_t rng_state;

static std::uint64_t
next_u64(void)
{
	std::uint64_t z = (rng_state += 0x9e3779b97f4a7c15ULL);

	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return (z ^ (z >> 31));
}

static std::uint32_t
next_below(std::uint32_t n)
{
	return (std::uint32_t)(next_u64() % n);
}

/* ------------------------------------------------------------------ */

int
main(void)
{
	/*
	 * Every offset keeps at least four guard bytes below the window and
	 * eight above it, so both under- and over-writes stay inside the
	 * compared buffer instead of corrupting the harness.
	 */
	static const std::size_t offsets[] = { 4, 5, 8, 9, 15, 16, 17, 23, 24,
	    31, 32, 40 };
	static const std::size_t noff = sizeof offsets / sizeof offsets[0];

	std::uint8_t bg_guard[BUFSZ], bg_pat[BUFSZ], bg_zero[BUFSZ],
	    bg_ones[BUFSZ];

	std::memset(bg_guard, GUARD, BUFSZ);
	std::memset(bg_zero, 0x00, BUFSZ);
	std::memset(bg_ones, 0xff, BUFSZ);
	for (std::size_t i = 0; i < BUFSZ; i++)
		bg_pat[i] = (std::uint8_t)(0xa5 ^ (i * 31));

	const std::uint8_t *bgs[] = { bg_guard, bg_pat, bg_zero, bg_ones };
	const std::size_t nbg = sizeof bgs / sizeof bgs[0];

	std::uint8_t b[WINDOW];

	/* ---- hand written edge cases ---- */

	static const std::uint8_t fills[] = { 0x00, 0x01, 0x7f, 0x80, 0xfe,
	    0xff, 0x55, 0xaa };

	for (std::size_t fi = 0; fi < sizeof fills / sizeof fills[0]; fi++) {
		std::memset(b, fills[fi], WINDOW);
		for (std::size_t oi = 0; oi < noff; oi++)
			for (std::size_t gi = 0; gi < nbg; gi++)
				run_all(b, offsets[oi], bgs[gi]);
	}

	/* ascending / descending / multiplicative ramps */
	for (std::size_t i = 0; i < WINDOW; i++)
		b[i] = (std::uint8_t)i;
	for (std::size_t oi = 0; oi < noff; oi++)
		for (std::size_t gi = 0; gi < nbg; gi++)
			run_all(b, offsets[oi], bgs[gi]);

	for (std::size_t i = 0; i < WINDOW; i++)
		b[i] = (std::uint8_t)(0xff - i);
	for (std::size_t oi = 0; oi < noff; oi++)
		for (std::size_t gi = 0; gi < nbg; gi++)
			run_all(b, offsets[oi], bgs[gi]);

	for (std::size_t i = 0; i < WINDOW; i++)
		b[i] = (std::uint8_t)(i * 17 + 3);
	for (std::size_t oi = 0; oi < noff; oi++)
		for (std::size_t gi = 0; gi < nbg; gi++)
			run_all(b, offsets[oi], bgs[gi]);

	/*
	 * Hot/cold byte walks: exactly one of the sixteen logical bytes
	 * differs from a uniform background.  Every field, every shift
	 * position and every node index is therefore individually observable
	 * in the encoded stream and in the decoded structure.
	 */
	static const std::uint8_t bases[] = { 0x00, 0xff, 0x7f, 0x80 };
	static const std::uint8_t hots[] = { 0x00, 0x01, 0x02, 0x7f, 0x80,
	    0x81, 0xfe, 0xff };

	for (std::size_t bi = 0; bi < sizeof bases / sizeof bases[0]; bi++) {
		for (std::size_t k = 0; k < WINDOW; k++) {
			for (std::size_t hi = 0; hi < sizeof hots / sizeof hots[0];
			    hi++) {
				std::memset(b, bases[bi], WINDOW);
				b[k] = hots[hi];
				for (std::size_t oi = 0; oi < noff; oi++)
					for (std::size_t gi = 0; gi < nbg; gi++)
						run_all(b, offsets[oi],
						    bgs[gi]);
			}
		}
	}

	/* every byte value in every position, guard background */
	for (unsigned v = 0; v < 256; v++) {
		for (std::size_t k = 0; k < WINDOW; k++) {
			std::memset(b, 0x00, WINDOW);
			b[k] = (std::uint8_t)v;
			run_all(b, 16, bg_pat);
			std::memset(b, 0xff, WINDOW);
			b[k] = (std::uint8_t)v;
			run_all(b, 17, bg_guard);
		}
	}

	/* ---- fixed seed randomised sweep ---- */

	rng_state = 0xb0019c0ffee12345ULL;

	static const std::uint8_t extremes[] = { 0x00, 0x01, 0x7f, 0x80, 0xfe,
	    0xff };
	const unsigned long ITERS = 250000UL;
	std::uint8_t bg_rand[BUFSZ];

	for (unsigned long it = 0; it < ITERS; it++) {
		std::uint64_t r = next_u64();

		for (std::size_t i = 0; i < WINDOW; i++) {
			if (i % 8 == 0)
				r = next_u64();
			std::uint8_t v = (std::uint8_t)(r >> ((i % 8) * 8));
			if ((v & 3) == 0)
				v = extremes[v % 6];
			b[i] = v;
		}

		std::size_t off = offsets[next_below((std::uint32_t)noff)];

		std::uint32_t sel = next_below(4);
		if (sel == 3) {
			for (std::size_t i = 0; i < BUFSZ; i++) {
				if (i % 8 == 0)
					r = next_u64();
				bg_rand[i] = (std::uint8_t)(r >> ((i % 8) * 8));
			}
			run_all(b, off, bg_rand);
		} else {
			run_all(b, off, bgs[sel]);
		}
	}

	/* ---- report ---- */

	unsigned long long total_fail = 0;

	std::printf("\n");
	std::printf("  %-14s %14s %14s  %s\n", "function", "cases", "failures",
	    "result");
	std::printf("  ---------------------------------------------------------\n");
	for (int f = 0; f < FN_COUNT; f++) {
		total_fail += fails[f];
		std::printf("  %-14s %14llu %14llu  %s\n", fn_name[f], cases[f],
		    fails[f], fails[f] == 0 ? "PASS" : "FAIL");
	}
	std::printf("  ---------------------------------------------------------\n");
	std::printf("  %-14s %14llu %14llu  %s\n", "TOTAL",
	    cases[0] + cases[1] + cases[2] + cases[3], total_fail,
	    total_fail == 0 ? "PASS" : "FAIL");
	std::printf("\n");

	return (total_fail == 0) ? 0 : 1;
}
