// PBSD batch b0054 -- differential test: port.cppm vs oracle.c.
//
// Both crc(3)-style routines in this batch read from a file descriptor and
// write through two out-parameters, while also mutating a hidden running
// total.  Every case therefore compares:
//
//   * the return value;
//   * the ENTIRE out-parameter region (64 guard bytes preset to 0x7f in two
//     independent buffers, one per implementation), so a write past the
//     nominal window is caught;
//   * the running total after the call.
//
// The input is served from a regular file so that read(2) returns full
// buffers; lengths straddle both BUFSIZ (crc32's buffer) and 16*1024 (crc's
// buffer) so the multi-read path, the "len += nr" accumulation and the
// trailing length-folding loop in crc() are all exercised on both sides of
// every boundary.  A bad (write-only) descriptor drives the read() == -1
// error return.

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <random>
#include <vector>

#include <sys/types.h>

#include <fcntl.h>
#include <unistd.h>

import pbsd.usr.bin.cksum.b0054;

namespace port = pbsd::usr_bin_cksum::b0054;

extern "C" {
int ref_crc32(int, std::uint32_t *, off_t *);
int ref_crc(int, std::uint32_t *, off_t *);
std::uint32_t ref_crc32_total_get(void);
void ref_crc32_total_set(std::uint32_t);
std::uint32_t ref_crc_total_get(void);
void ref_crc_total_set(std::uint32_t);
}

/* ------------------------------------------------------------------ */

namespace {

struct Stats {
	const char *name;
	long long cases = 0;
	long long fails = 0;
	int printed = 0;
};

Stats st_crc32{"crc32"};
Stats st_crc{"crc"};

constexpr std::size_t REGION = 64;
constexpr std::size_t CVAL_OFF = 8;
constexpr std::size_t CLEN_OFF = 24;
constexpr unsigned char GUARD = 0x7f;

struct Out {
	alignas(16) unsigned char raw[REGION];

	void arm() { std::memset(raw, GUARD, REGION); }
	std::uint32_t *cval() { return reinterpret_cast<std::uint32_t *>(raw + CVAL_OFF); }
	off_t *clen() { return reinterpret_cast<off_t *>(raw + CLEN_OFF); }

	std::uint32_t cval_v() const
	{
		std::uint32_t v;
		std::memcpy(&v, raw + CVAL_OFF, sizeof v);
		return v;
	}
	long long clen_v() const
	{
		off_t v;
		std::memcpy(&v, raw + CLEN_OFF, sizeof v);
		return static_cast<long long>(v);
	}
};

int datafd = -1;
int badfd = -1;

[[noreturn]] void die(const char *what)
{
	std::fprintf(stderr, "harness: %s: %s\n", what, std::strerror(errno));
	std::exit(2);
}

void setup_fds()
{
	char tmpl[] = "/tmp/pbsd-b0054-XXXXXX";
	datafd = mkstemp(tmpl);
	if (datafd < 0)
		die("mkstemp");
	if (unlink(tmpl) != 0)
		die("unlink");
	/* Opened write-only: read(2) fails with EBADF, driving nr < 0. */
	badfd = open("/dev/null", O_WRONLY);
	if (badfd < 0)
		die("open /dev/null");
}

void feed(const unsigned char *d, std::size_t n)
{
	if (ftruncate(datafd, 0) != 0)
		die("ftruncate");
	if (lseek(datafd, 0, SEEK_SET) != 0)
		die("lseek");
	std::size_t done = 0;
	while (done < n) {
		ssize_t w = write(datafd, d + done, n - done);
		if (w <= 0)
			die("write");
		done += static_cast<std::size_t>(w);
	}
}

int rewound()
{
	if (lseek(datafd, 0, SEEK_SET) != 0)
		die("lseek");
	return datafd;
}

void hexdump_region(const char *tag, const Out &o)
{
	std::fprintf(stderr, "      %s:", tag);
	for (std::size_t i = 0; i < REGION; ++i)
		std::fprintf(stderr, " %02x", o.raw[i]);
	std::fputc('\n', stderr);
}

void report(Stats &st, const char *label, std::size_t n, std::uint32_t init, bool bad, int ra,
    int rb, std::uint32_t ta, std::uint32_t tb, const Out &a, const Out &b)
{
	if (st.printed >= 8)
		return;
	++st.printed;
	std::fprintf(stderr,
	    "  FAIL %s [%s] len=%zu init_total=0x%08x bad_fd=%d\n"
	    "      ret   ref=%d port=%d\n"
	    "      cval  ref=0x%08x port=0x%08x\n"
	    "      clen  ref=%lld port=%lld\n"
	    "      total ref=0x%08x port=0x%08x\n",
	    st.name, label, n, init, bad ? 1 : 0, ra, rb, a.cval_v(), b.cval_v(), a.clen_v(),
	    b.clen_v(), ta, tb);
	if (std::memcmp(a.raw, b.raw, REGION) != 0) {
		hexdump_region("ref ", a);
		hexdump_region("port", b);
	}
}

/*
 * One differential case.  preset == false leaves the running totals alone so
 * that a chain of calls exercises the carried-over state.
 */
void case_crc32(const unsigned char *d, std::size_t n, std::uint32_t init, bool bad,
    const char *label, bool preset = true)
{
	Out a, b;
	a.arm();
	b.arm();
	if (!bad)
		feed(d, n);
	if (preset) {
		ref_crc32_total_set(init);
		port::crc32_total_set(init);
	}

	int ra = ref_crc32(bad ? badfd : rewound(), a.cval(), a.clen());
	std::uint32_t ta = ref_crc32_total_get();

	int rb = port::crc32(bad ? badfd : rewound(), b.cval(), b.clen());
	std::uint32_t tb = port::crc32_total_get();

	++st_crc32.cases;
	if (ra != rb || ta != tb || std::memcmp(a.raw, b.raw, REGION) != 0) {
		++st_crc32.fails;
		report(st_crc32, label, n, init, bad, ra, rb, ta, tb, a, b);
	}
}

void case_crc(const unsigned char *d, std::size_t n, std::uint32_t init, bool bad,
    const char *label, bool preset = true)
{
	Out a, b;
	a.arm();
	b.arm();
	if (!bad)
		feed(d, n);
	if (preset) {
		ref_crc_total_set(init);
		port::crc_total_set(init);
	}

	int ra = ref_crc(bad ? badfd : rewound(), a.cval(), a.clen());
	std::uint32_t ta = ref_crc_total_get();

	int rb = port::crc(bad ? badfd : rewound(), b.cval(), b.clen());
	std::uint32_t tb = port::crc_total_get();

	++st_crc.cases;
	if (ra != rb || ta != tb || std::memcmp(a.raw, b.raw, REGION) != 0) {
		++st_crc.fails;
		report(st_crc, label, n, init, bad, ra, rb, ta, tb, a, b);
	}
}

void both(const unsigned char *d, std::size_t n, std::uint32_t init, const char *label)
{
	case_crc32(d, n, init, false, label);
	case_crc(d, n, init, false, label);
}

/* ------------------------------------------------------------------ */
/* Hand-written edge cases                                            */
/* ------------------------------------------------------------------ */

enum Pattern {
	P_ZERO,
	P_FF,
	P_80,
	P_7F,
	P_RAMP,
	P_RAMP_HIGH,
	P_ALT,
	P_ASCII,
	P_NUL_HEAVY,
	P_DESC,
	P_NPAT
};

const char *pattern_name(int p)
{
	switch (p) {
	case P_ZERO:
		return "zeros";
	case P_FF:
		return "0xff";
	case P_80:
		return "0x80";
	case P_7F:
		return "0x7f";
	case P_RAMP:
		return "ramp";
	case P_RAMP_HIGH:
		return "ramp-high";
	case P_ALT:
		return "alt-00-ff";
	case P_ASCII:
		return "ascii";
	case P_NUL_HEAVY:
		return "nul-heavy";
	default:
		return "descending";
	}
}

void fill_pattern(std::vector<unsigned char> &v, int p)
{
	const char *ascii = "The quick brown fox jumps over the lazy dog 0123456789";
	std::size_t alen = std::strlen(ascii);
	for (std::size_t i = 0; i < v.size(); ++i) {
		switch (p) {
		case P_ZERO:
			v[i] = 0x00;
			break;
		case P_FF:
			v[i] = 0xff;
			break;
		case P_80:
			v[i] = 0x80;
			break;
		case P_7F:
			v[i] = 0x7f;
			break;
		case P_RAMP:
			v[i] = static_cast<unsigned char>(i & 0xff);
			break;
		case P_RAMP_HIGH:
			v[i] = static_cast<unsigned char>(0x80 + (i & 0x7f));
			break;
		case P_ALT:
			v[i] = (i & 1) ? 0xff : 0x00;
			break;
		case P_ASCII:
			v[i] = static_cast<unsigned char>(ascii[i % alen]);
			break;
		case P_NUL_HEAVY:
			v[i] = (i % 7 == 3) ? static_cast<unsigned char>(0x80 | (i & 0x7f))
					   : 0x00;
			break;
		default:
			v[i] = static_cast<unsigned char>(255 - (i & 0xff));
			break;
		}
	}
}

void hand_cases()
{
	static const std::uint32_t inits[] = { 0x00000000u, 0xffffffffu, 0xdeadbeefu,
		0x00000001u, 0x80000000u, 0x000000ffu, 0x7fffffffu };
	static const std::size_t small_lens[] = { 0, 1, 2, 3, 4, 5, 7, 8, 9, 15, 16, 17, 31, 32,
		33, 63, 64, 65, 127, 128, 129, 254, 255, 256, 257, 511, 512, 513, 1023, 1024,
		1025, 4095, 4096, 4097 };
	/* Straddle BUFSIZ (crc32) and 16*1024 (crc) and the len>>=8 byte steps. */
	static const std::size_t big_lens[] = { BUFSIZ - 1, BUFSIZ, BUFSIZ + 1, 2 * BUFSIZ,
		2 * BUFSIZ + 1, 16 * 1024 - 1, 16 * 1024, 16 * 1024 + 1, 24 * 1024,
		32 * 1024 - 1, 32 * 1024, 32 * 1024 + 1, 65535, 65536, 65537, 40000 };

	std::vector<unsigned char> v;
	char label[64];

	/* Every byte value on its own, so single-byte high-bit input is covered. */
	for (int b = 0; b < 256; ++b) {
		unsigned char one = static_cast<unsigned char>(b);
		std::snprintf(label, sizeof label, "single-byte-0x%02x", b);
		for (std::uint32_t init : inits)
			both(&one, 1, init, label);
	}

	/* Two-byte pairs around the sign boundary: catches ++p/--p and ordering. */
	static const unsigned char edge_bytes[] = { 0x00, 0x01, 0x7e, 0x7f, 0x80, 0x81, 0xfe,
		0xff };
	for (unsigned char x : edge_bytes)
		for (unsigned char y : edge_bytes) {
			unsigned char pair[2] = { x, y };
			std::snprintf(label, sizeof label, "pair-%02x%02x", x, y);
			both(pair, 2, 0u, label);
			both(pair, 2, 0xffffffffu, label);
			both(pair, 2, 0xdeadbeefu, label);
		}

	/* Empty input: exercises the "no read", "len == 0" and skipped
	 * length-folding paths. */
	for (std::uint32_t init : inits)
		both(nullptr, 0, init, "empty");

	for (int p = 0; p < P_NPAT; ++p) {
		for (std::size_t n : small_lens) {
			v.assign(n, 0);
			fill_pattern(v, p);
			std::snprintf(label, sizeof label, "%s", pattern_name(p));
			for (std::uint32_t init : inits)
				both(v.data(), n, init, label);
		}
		for (std::size_t n : big_lens) {
			v.assign(n, 0);
			fill_pattern(v, p);
			std::snprintf(label, sizeof label, "%s-big", pattern_name(p));
			both(v.data(), n, 0x00000000u, label);
			both(v.data(), n, 0xffffffffu, label);
			both(v.data(), n, 0xdeadbeefu, label);
		}
	}

	/* Error return: read(2) fails, so nothing may be stored and the running
	 * total must be left in its inverted state. */
	for (std::uint32_t init : inits) {
		case_crc32(nullptr, 0, init, true, "bad-fd");
		case_crc(nullptr, 0, init, true, "bad-fd");
	}

	/* Chained calls with carried-over totals, mixing good and bad fds. */
	ref_crc32_total_set(0u);
	port::crc32_total_set(0u);
	ref_crc_total_set(~0u);
	port::crc_total_set(~0u);
	for (int i = 0; i < 64; ++i) {
		std::size_t n = static_cast<std::size_t>(i * 37 % 300);
		v.assign(n, 0);
		fill_pattern(v, i % P_NPAT);
		bool bad = (i % 11 == 5);
		case_crc32(v.data(), n, 0u, bad, "chain", false);
		case_crc(v.data(), n, 0u, bad, "chain", false);
	}
}

/* ------------------------------------------------------------------ */
/* Fixed-seed randomised sweep                                        */
/* ------------------------------------------------------------------ */

constexpr long long SWEEP = 200000;

void random_cases()
{
	std::mt19937_64 rng(0x50425344b0054ULL); /* fixed seed */
	std::vector<unsigned char> v;
	v.reserve(70000);

	for (long long it = 0; it < SWEEP; ++it) {
		std::size_t n;
		unsigned bucket = static_cast<unsigned>(rng() % 1000u);
		if (bucket < 30)
			n = 0;
		else if (bucket < 120)
			n = 1 + static_cast<std::size_t>(rng() % 3u);
		else if (bucket < 900)
			n = static_cast<std::size_t>(rng() % 97u);
		else if (bucket < 995)
			n = static_cast<std::size_t>(rng() % 4096u);
		else
			/* Multi-read territory for both buffer sizes. */
			n = 8000 + static_cast<std::size_t>(rng() % 26000u);

		v.assign(n, 0);
		unsigned mode = static_cast<unsigned>(rng() % 5u);
		for (std::size_t i = 0; i < n; ++i) {
			unsigned r = static_cast<unsigned>(rng());
			switch (mode) {
			case 0:
				v[i] = static_cast<unsigned char>(r & 0xff);
				break;
			case 1: /* high-bit bytes only */
				v[i] = static_cast<unsigned char>(0x80 | (r & 0x7f));
				break;
			case 2: /* low-bit bytes only */
				v[i] = static_cast<unsigned char>(r & 0x7f);
				break;
			case 3: /* NUL heavy */
				v[i] = (r & 7u) ? 0x00 : static_cast<unsigned char>(r >> 8);
				break;
			default: /* sign-boundary heavy */
				v[i] = static_cast<unsigned char>((r & 1u) ? 0x7f : 0x80);
				break;
			}
		}

		std::uint32_t init = static_cast<std::uint32_t>(rng());
		if ((rng() & 7u) == 0)
			init = (rng() & 1u) ? 0u : ~0u;
		bool bad = (rng() % 500u) == 0;

		case_crc32(v.data(), n, init, bad, "random");
		case_crc(v.data(), n, init, bad, "random");
	}
}

} // namespace

int main()
{
	setup_fds();

	hand_cases();
	random_cases();

	std::printf("\n%-12s %12s %12s  %s\n", "function", "cases", "failures", "result");
	std::printf("%-12s %12s %12s  %s\n", "------------", "------------", "------------",
	    "------");
	const Stats *all[] = { &st_crc32, &st_crc };
	long long total_cases = 0, total_fails = 0;
	for (const Stats *s : all) {
		std::printf("%-12s %12lld %12lld  %s\n", s->name, s->cases, s->fails,
		    s->fails == 0 ? "PASS" : "FAIL");
		total_cases += s->cases;
		total_fails += s->fails;
	}
	std::printf("%-12s %12lld %12lld  %s\n", "TOTAL", total_cases, total_fails,
	    total_fails == 0 ? "PASS" : "FAIL");

	if (total_cases == 0) {
		std::fprintf(stderr, "harness: no cases ran\n");
		return 1;
	}
	return total_fails == 0 ? 0 : 1;
}
