/*
 * harness.cpp -- differential test for PBSD batch b0295.
 */

#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <unistd.h>

import pbsd.sbin.ipf.libipf.b0295;

namespace P = pbsd::sbin_ipf_libipf::b0295;

extern "C" {
std::uint16_t ref_tcpflags(char *flgs);
void ref_printtunable(P::ipftune_t *tup);
void ref_printlog(P::frentry_t *fp);
}

static constexpr unsigned char GUARD = 0x7f;
static constexpr int MAX_REPORT = 8;
static constexpr long SWEEP = 200000;

struct Stat {
	const char *name;
	long cases;
	long fails;
	int reported;
};

static Stat st_tcpflags = { "tcpflags", 0, 0, 0 };
static Stat st_printtunable = { "printtunable", 0, 0, 0 };
static Stat st_printlog = { "printlog", 0, 0, 0 };

static std::uint64_t rng_state = 0xb0295feedfaceULL;

static inline std::uint64_t
rnd(void)
{
	std::uint64_t z = (rng_state += 0x9e3779b97f4a7c15ULL);

	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return z ^ (z >> 31);
}

static inline std::size_t
rnd_mod(std::size_t m)
{
	if (m == 0)
		return 0;
	return (std::size_t)(rnd() % (std::uint64_t)m);
}

static void
stat_fail(Stat *st, const char *tag, const char *detail)
{
	st->fails++;
	if (st->reported < MAX_REPORT) {
		st->reported++;
		std::printf("  FAIL %s [%s] %s\n", st->name, tag, detail);
	}
}

static std::string
capture_stdout_void(void (*fn)(P::frentry_t *), P::frentry_t *fp)
{
	std::fflush(stdout);
	int fds[2];
	if (pipe(fds) != 0)
		std::abort();
	int saved = dup(STDOUT_FILENO);
	dup2(fds[1], STDOUT_FILENO);
	close(fds[1]);
	fn(fp);
	std::fflush(stdout);
	dup2(saved, STDOUT_FILENO);
	close(saved);
	char out[8192];
	ssize_t n = read(fds[0], out, sizeof(out) - 1);
	close(fds[0]);
	if (n < 0)
		n = 0;
	out[n] = '\0';
	return std::string(out);
}

static std::string
capture_stdout_tune(void (*fn)(P::ipftune_t *), P::ipftune_t *tup)
{
	std::fflush(stdout);
	int fds[2];
	if (pipe(fds) != 0)
		std::abort();
	int saved = dup(STDOUT_FILENO);
	dup2(fds[1], STDOUT_FILENO);
	close(fds[1]);
	fn(tup);
	std::fflush(stdout);
	dup2(saved, STDOUT_FILENO);
	close(saved);
	char out[8192];
	ssize_t n = read(fds[0], out, sizeof(out) - 1);
	close(fds[0]);
	if (n < 0)
		n = 0;
	out[n] = '\0';
	return std::string(out);
}

static void
call_port_printlog(P::frentry_t *fp)
{
	P::printlog(fp);
}

static void
call_ref_printlog(P::frentry_t *fp)
{
	ref_printlog(fp);
}

static void
call_port_printtunable(P::ipftune_t *tup)
{
	P::printtunable(tup);
}

static void
call_ref_printtunable(P::ipftune_t *tup)
{
	ref_printtunable(tup);
}

static void
fill_str(char *buf, std::size_t cap, int pattern, std::size_t len)
{
	static const unsigned char alpha[] = {
	    0x00, 0x01, 0x7e, 0x7f, 0x80, 0xfe, 0xff,
	    'F', 'S', 'R', 'P', 'A', 'U', 'E', 'W', 'e', 'Z', 'x'
	};
	const std::size_t na = sizeof(alpha);

	if (cap == 0)
		return;
	if (len >= cap)
		len = cap - 1;
	for (std::size_t i = 0; i < len; i++) {
		switch (pattern) {
		case 0:
			buf[i] = (char)alpha[i % na];
			break;
		case 1:
			buf[i] = (char)(0x80 + (i & 0x7f));
			break;
		case 2:
			buf[i] = (char)((i & 1) ? 'W' : 'F');
			break;
		default:
			buf[i] = (char)(rnd() & 0xff);
			break;
		}
	}
	buf[len] = '\0';
}

/* ------------------------------------------------------------------------ */
/* tcpflags                                                                  */
/* ------------------------------------------------------------------------ */

static void
tcpflags_case(const char *tag, const char *input, int pattern)
{
	char ba[256], bb[256];

	std::memset(ba, GUARD, sizeof(ba));
	std::memset(bb, GUARD, sizeof(bb));

	if (input != nullptr) {
		std::size_t n = std::strlen(input);
		if (n > sizeof(ba) - 32)
			n = sizeof(ba) - 32;
		std::memcpy(ba + 16, input, n + 1);
		std::memcpy(bb + 16, input, n + 1);
	} else {
		std::size_t len = rnd_mod(sizeof(ba) - 32);
		fill_str(ba + 16, sizeof(ba) - 16, pattern, len);
		std::memcpy(bb + 16, ba + 16, len + 1);
	}
	(void)pattern;

	char *pa = ba + 16;
	char *pb = bb + 16;

	std::uint16_t got = P::tcpflags(pa);
	std::uint16_t ref = ref_tcpflags(pb);

	st_tcpflags.cases++;
	int bad_ret = (got != ref);
	int bad_buf = (std::memcmp(ba, bb, sizeof(ba)) != 0);
	if (bad_ret || bad_buf) {
		stat_fail(&st_tcpflags, tag, bad_ret ? "return" : "buf");
		if (st_tcpflags.reported <= MAX_REPORT) {
			std::printf("      port=0x%04x ref=0x%04x str=%.*s\n", got,
			    ref, 48, pa);
		}
	}
}

static void
test_tcpflags_edges(void)
{
	static const char *valid[] = {
	    "",
	    "F",
	    "S",
	    "R",
	    "P",
	    "A",
	    "U",
	    "E",
	    "W",
	    "e",
	    "FSRPAUEWe",
	    "WWWWW",
	    "FW",
	    "WF",
	    "SA",
	    "Z",
	    "FZ",
	    "ZF",
	    "x",
	    "\x80",
	    "\xff",
	    "\x80\x81",
	    "F\x80",
	    "\x80" "F",
	    "F\x00G",
	};

	for (std::size_t i = 0; i < sizeof(valid) / sizeof(valid[0]); i++) {
		char tag[48];
		std::snprintf(tag, sizeof(tag), "edge%zu", i);
		tcpflags_case(tag, valid[i], 0);
	}

	char hi[8];
	for (int i = 0; i < 7; i++)
		hi[i] = (char)(0x80 + i);
	hi[7] = '\0';
	tcpflags_case("hibyte-run", hi, 1);
}

static void
test_tcpflags_sweep(void)
{
	char buf[192];

	for (long i = 0; i < SWEEP; i++) {
		int pat = (int)(rnd() % 4);
		std::size_t len = rnd_mod(sizeof(buf) - 1);
		fill_str(buf, sizeof(buf), pat, len);
		if ((rnd() & 15) == 0) {
			static const char *known[] = {
			    "", "F", "W", "FSRPAUEWe", "Z", "FWZ"
			};
			std::snprintf(buf, sizeof(buf), "%s",
			    known[rnd_mod(sizeof(known) / sizeof(known[0]))]);
		}
		char tag[48];
		std::snprintf(tag, sizeof(tag), "rnd%ld", i);
		tcpflags_case(tag, buf, pat);
	}
}

/* ------------------------------------------------------------------------ */
/* printtunable                                                                */
/* ------------------------------------------------------------------------ */

static void
init_tune(P::ipftune_t *t, int sz, const char *name, unsigned long vmin,
    unsigned long vmax, std::uint64_t val)
{
	std::memset(t, 0, sizeof(*t));
	t->ipft_sz = sz;
	t->ipft_min = vmin;
	t->ipft_max = vmax;
	if (name != nullptr)
		std::snprintf(t->ipft_name, sizeof(t->ipft_name), "%s", name);
	t->ipft_un.ipftu_long = (unsigned long)val;
	t->ipft_un.ipftu_int = (unsigned int)val;
	t->ipft_un.ipftu_short = (unsigned short)val;
	t->ipft_un.ipftu_char = (unsigned char)val;
}

static void
printtunable_case(const char *tag, int sz, const char *name,
    unsigned long vmin, unsigned long vmax, std::uint64_t val)
{
	unsigned char arena_p[sizeof(P::ipftune_t) + 64];
	unsigned char arena_r[sizeof(P::ipftune_t) + 64];

	std::memset(arena_p, GUARD, sizeof(arena_p));
	std::memset(arena_r, GUARD, sizeof(arena_r));

	P::ipftune_t *tp = (P::ipftune_t *)(arena_p + 32);
	P::ipftune_t *tr = (P::ipftune_t *)(arena_r + 32);

	init_tune(tp, sz, name, vmin, vmax, val);
	init_tune(tr, sz, name, vmin, vmax, val);

	std::string got = capture_stdout_tune(call_port_printtunable, tp);
	std::string ref = capture_stdout_tune(call_ref_printtunable, tr);

	st_printtunable.cases++;
	int bad_out = (got != ref);
	int bad_p = (std::memcmp(arena_p, arena_p + 32, 32) != 0) ||
	    (std::memcmp(arena_p + 32 + sizeof(P::ipftune_t),
		 arena_p + 32 + sizeof(P::ipftune_t) +
		     (sizeof(arena_p) - 32 - sizeof(P::ipftune_t)),
		 sizeof(arena_p) - 32 - sizeof(P::ipftune_t)) != 0);
	int bad_r = (std::memcmp(arena_r, arena_r + 32, 32) != 0) ||
	    (std::memcmp(arena_r + 32 + sizeof(P::ipftune_t),
		 arena_r + 32 + sizeof(P::ipftune_t) +
		     (sizeof(arena_r) - 32 - sizeof(P::ipftune_t)),
		 sizeof(arena_r) - 32 - sizeof(P::ipftune_t)) != 0);
	if (bad_out || bad_p || bad_r) {
		stat_fail(&st_printtunable, tag, bad_out ? "stdout" : "buf");
		if (st_printtunable.reported <= MAX_REPORT) {
			std::printf("      port=%.*s\n", (int)got.size(),
			    got.c_str());
			std::printf("      ref =%.*s\n", (int)ref.size(),
			    ref.c_str());
		}
	}
}

static void
test_printtunable_edges(void)
{
	printtunable_case("ulong", (int)sizeof(unsigned long), "tcp_timeout", 1,
	    1000, 42);
	printtunable_case("uint", (int)sizeof(unsigned int), "state_max", 0,
	    0xffffffffUL, 0xdeadbeefUL);
	printtunable_case("ushort", (int)sizeof(unsigned short), "frag", 0, 65535,
	    0x80ff);
	printtunable_case("uchar", (int)sizeof(unsigned char), "ttl", 0, 255, 0xff);
	printtunable_case("else0", 0, "bad", 0, 0, 0);
	printtunable_case("else3", 3, "odd", 1, 9, 7);
	printtunable_case("else7", 7, "weird", 2, 8, 5);
	printtunable_case("empty-name", (int)sizeof(unsigned int), "", 0, 0, 0);
	printtunable_case("hibyte-name", (int)sizeof(unsigned long),
	    "\xff\xfe\xfd", 0x80, 0xff, 0x80818283);
}

static void
test_printtunable_sweep(void)
{
	static const int sizes[] = {
	    0,
	    1,
	    2,
	    3,
	    4,
	    (int)sizeof(unsigned char),
	    (int)sizeof(unsigned short),
	    (int)sizeof(unsigned int),
	    (int)sizeof(unsigned long),
	    7,
	    9,
	    16
	};

	for (long i = 0; i < SWEEP; i++) {
		int sz = sizes[rnd_mod(sizeof(sizes) / sizeof(sizes[0]))];
		char name[64];
		fill_str(name, sizeof(name), (int)(rnd() % 4), rnd_mod(32));
		unsigned long vmin = (unsigned long)rnd();
		unsigned long vmax = (unsigned long)rnd();
		std::uint64_t val = rnd();
		char tag[48];
		std::snprintf(tag, sizeof(tag), "rnd%ld", i);
		printtunable_case(tag, sz, name, vmin, vmax, val);
	}
}

/* ------------------------------------------------------------------------ */
/* printlog                                                                    */
/* ------------------------------------------------------------------------ */

static void
init_frentry(P::frentry_t *fp, std::uint32_t flags, unsigned int level)
{
	fp->fr_flags = flags;
	fp->fr_loglevel = level;
}

static void
printlog_case(const char *tag, std::uint32_t flags, unsigned int level)
{
	unsigned char arena_p[sizeof(P::frentry_t) + 64];
	unsigned char arena_r[sizeof(P::frentry_t) + 64];

	std::memset(arena_p, GUARD, sizeof(arena_p));
	std::memset(arena_r, GUARD, sizeof(arena_r));

	P::frentry_t *fp = (P::frentry_t *)(arena_p + 32);
	P::frentry_t *fr = (P::frentry_t *)(arena_r + 32);

	init_frentry(fp, flags, level);
	init_frentry(fr, flags, level);

	std::string got = capture_stdout_void(call_port_printlog, fp);
	std::string ref = capture_stdout_void(call_ref_printlog, fr);

	st_printlog.cases++;
	int bad_out = (got != ref);
	int bad_p = (std::memcmp(arena_p, arena_p + 32, 32) != 0) ||
	    (std::memcmp(arena_p + 32 + sizeof(P::frentry_t),
		 arena_p + 32 + sizeof(P::frentry_t) +
		     (sizeof(arena_p) - 32 - sizeof(P::frentry_t)),
		 sizeof(arena_p) - 32 - sizeof(P::frentry_t)) != 0);
	int bad_r = (std::memcmp(arena_r, arena_r + 32, 32) != 0) ||
	    (std::memcmp(arena_r + 32 + sizeof(P::frentry_t),
		 arena_r + 32 + sizeof(P::frentry_t) +
		     (sizeof(arena_r) - 32 - sizeof(P::frentry_t)),
		 sizeof(arena_r) - 32 - sizeof(P::frentry_t)) != 0);
	if (bad_out || bad_p || bad_r) {
		stat_fail(&st_printlog, tag, bad_out ? "stdout" : "buf");
		if (st_printlog.reported <= MAX_REPORT) {
			std::printf("      port=%.*s\n", (int)got.size(),
			    got.c_str());
			std::printf("      ref =%.*s\n", (int)ref.size(),
			    ref.c_str());
		}
	}
}

static void
test_printlog_edges(void)
{
	static const std::uint32_t flagvals[] = {
	    0,
	    0x10000U,
	    0x20000U,
	    0x40000U,
	    0x10000U | 0x20000U,
	    0x10000U | 0x40000U,
	    0x20000U | 0x40000U,
	    0x10000U | 0x20000U | 0x40000U,
	    0xffffffffU
	};
	static const unsigned int levels[] = {
	    0xffffU,
	    0,
	    (8 << 3) | 6,
	    (16 << 3) | 6,
	    (23 << 3) | 7,
	    0x00ff00ffU,
	    0x80808080U,
	    0x00010002U,
	    0xdeadbeefU
	};

	for (std::size_t i = 0; i < sizeof(flagvals) / sizeof(flagvals[0]);
	     i++) {
		for (std::size_t j = 0; j < sizeof(levels) / sizeof(levels[0]);
		     j++) {
			char tag[64];
			std::snprintf(tag, sizeof(tag), "edge f%zu l%zu", i, j);
			printlog_case(tag, flagvals[i], levels[j]);
		}
	}
}

static void
test_printlog_sweep(void)
{
	for (long i = 0; i < SWEEP; i++) {
		std::uint32_t flags = (std::uint32_t)rnd();
		if ((rnd() & 7) == 0)
			flags &= 0x70000U;
		unsigned int level = (unsigned int)rnd();
		if ((rnd() & 3) == 0)
			level = 0xffffU;
		char tag[48];
		std::snprintf(tag, sizeof(tag), "rnd%ld", i);
		printlog_case(tag, flags, level);
	}
}

/* ------------------------------------------------------------------------ */

int
main(void)
{
	test_tcpflags_edges();
	test_tcpflags_sweep();
	test_printtunable_edges();
	test_printtunable_sweep();
	test_printlog_edges();
	test_printlog_sweep();

	std::printf("\n%-14s %8s %8s\n", "function", "cases", "fails");
	std::printf("%-14s %8ld %8ld\n", st_tcpflags.name, st_tcpflags.cases,
	    st_tcpflags.fails);
	std::printf("%-14s %8ld %8ld\n", st_printtunable.name,
	    st_printtunable.cases, st_printtunable.fails);
	std::printf("%-14s %8ld %8ld\n", st_printlog.name, st_printlog.cases,
	    st_printlog.fails);

	long total_fails = st_tcpflags.fails + st_printtunable.fails +
	    st_printlog.fails;
	return total_fails == 0 ? 0 : 1;
}
