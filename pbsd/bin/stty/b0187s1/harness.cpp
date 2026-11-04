// Differential harness for PBSD batch b0187s1 (util.c: checkredirect).

import pbsd.bin.stty.b0187s1;

#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <sys/stat.h>
#include <unistd.h>

namespace P = pbsd::bin_stty::b0187s1;

extern "C" void ref_checkredirect(void);

namespace {

constexpr long SWEEP = 200000L;
constexpr int MAX_SHOW = 8;

struct Stat {
	const char *name;
	long cases;
	long fails;
	int shown;
};

struct MockCfg {
	int isatty_stdout;
	int isatty_stderr;
	int fstat_stdout;
	int fstat_stderr;
	dev_t rdev_stdout;
	dev_t rdev_stderr;
};

struct Rng {
	std::uint64_t s;

	explicit Rng(std::uint64_t seed) : s(seed) {}

	std::uint64_t next()
	{
		s += 0x9E3779B97F4A7C15ull;
		std::uint64_t z = s;
		z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
		z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
		return z ^ (z >> 31);
	}

	int bits(int lo, int hi)
	{
		if (hi <= lo)
			return lo;
		return lo + (int)(next() % (std::uint64_t)(hi - lo + 1));
	}

	bool coin()
	{
		return (next() & 1u) != 0;
	}
};

Rng rng(0xb0187s1faceULL);
Stat st_checkredirect = { "checkredirect", 0, 0, 0 };

static bool g_test_active;
static MockCfg g_mock;
static int g_warnx_count;
static char g_warnx_msg[256];

extern "C" int __real_isatty(int);
extern "C" int __real_fstat(int, struct stat *);

static void
reset_observed(void)
{
	g_warnx_count = 0;
	std::memset(g_warnx_msg, 0, sizeof(g_warnx_msg));
}

static void
apply_mock(const MockCfg &cfg)
{
	g_mock = cfg;
}

static bool
fail(Stat &st, const char *what)
{
	st.fails++;
	if (st.shown < MAX_SHOW) {
		st.shown++;
		std::printf("  FAIL %s: %s\n", st.name, what);
	}
	return false;
}

static bool
run_checkredirect_case(const MockCfg &cfg, const char *tag)
{
	int ref_warn;
	int port_warn;

	(void)tag;
	st_checkredirect.cases++;

	apply_mock(cfg);
	reset_observed();
	g_test_active = true;
	ref_checkredirect();
	ref_warn = g_warnx_count;

	apply_mock(cfg);
	reset_observed();
	g_test_active = true;
	P::checkredirect();
	port_warn = g_warnx_count;
	g_test_active = false;

	if (ref_warn != port_warn)
		return fail(st_checkredirect, "warnx call count mismatch");

	return true;
}

extern "C" int
__wrap_isatty(int fd)
{
	if (g_test_active) {
		if (fd == STDOUT_FILENO)
			return g_mock.isatty_stdout;
		if (fd == STDERR_FILENO)
			return g_mock.isatty_stderr;
		return 0;
	}
	return __real_isatty(fd);
}

extern "C" int
__wrap_fstat(int fd, struct stat *sb)
{
	if (g_test_active) {
		if (fd == STDOUT_FILENO) {
			if (g_mock.fstat_stdout != 0)
				return g_mock.fstat_stdout;
			sb->st_rdev = g_mock.rdev_stdout;
			return 0;
		}
		if (fd == STDERR_FILENO) {
			if (g_mock.fstat_stderr != 0)
				return g_mock.fstat_stderr;
			sb->st_rdev = g_mock.rdev_stderr;
			return 0;
		}
		return -1;
	}
	return __real_fstat(fd, sb);
}

extern "C" void
__wrap_warnx(const char *fmt, ...)
{
	if (g_test_active) {
		va_list ap;
		va_start(ap, fmt);
		g_warnx_count++;
		std::vsnprintf(g_warnx_msg, sizeof(g_warnx_msg), fmt, ap);
		va_end(ap);
		return;
	}
	va_list ap;
	va_start(ap, fmt);
	std::vfprintf(stderr, fmt, ap);
	va_end(ap);
	std::fputc('\n', stderr);
}

static void
test_checkredirect_edges(void)
{
	const MockCfg edges[] = {
		/* stdout not a tty */
		{ 0, 0, 0, 0, 1, 2 },
		{ 0, 1, 0, 0, 1, 2 },
		/* stderr not a tty */
		{ 1, 0, 0, 0, 1, 2 },
		{ 2, 0, -1, 0, 3, 4 },
		/* both tty, stdout fstat fails */
		{ 1, 1, -1, 0, 5, 6 },
		{ 1, 1, 1, 0, 5, 6 },
		{ 1, 1, 0x80, 0, 5, 6 },
		/* both tty, stderr fstat fails */
		{ 1, 1, 0, -1, 5, 6 },
		{ 1, 1, 0, 1, 5, 6 },
		{ 1, 1, 0, 0x80, 5, 6 },
		/* both fstat succeed, same rdev */
		{ 1, 1, 0, 0, 0, 0 },
		{ 1, 1, 0, 0, 7, 7 },
		{ 2, 2, 0, 0, 0xdeadbeefULL, 0xdeadbeefULL },
		/* both fstat succeed, different rdev -> warn */
		{ 1, 1, 0, 0, 0, 1 },
		{ 1, 1, 0, 0, 5, 6 },
		{ 1, 1, 0, 0, 0x80, 0x81 },
		{ 1, 1, 0, 0, 0xff, 0xfe },
		{ 2, 1, 0, 0, 3, 4 },
		/* short-circuit vs || mutation: stdout not tty but stderr path would warn */
		{ 0, 1, 0, 0, 1, 2 },
		/* short-circuit vs || mutation: stderr not tty but stdout path would warn */
		{ 1, 0, 0, 0, 1, 2 },
		/* short-circuit vs || on fstat(stdout): out fails, err ok, rdev differ */
		{ 1, 1, -1, 0, 9, 10 },
		/* short-circuit vs || on fstat(stderr): out ok, err fails, rdev differ */
		{ 1, 1, 0, -1, 9, 10 },
		/* negated fstat mutation: success with equal rdev should not warn */
		{ 1, 1, 0, 0, 42, 42 },
	};

	for (const MockCfg &cfg : edges)
		run_checkredirect_case(cfg, "edge");
}

static void
test_checkredirect_sweep(void)
{
	for (long i = 0; i < SWEEP; i++) {
		MockCfg cfg;

		switch (rng.bits(0, 5)) {
		case 0:
			cfg.isatty_stdout = 0;
			break;
		case 1:
			cfg.isatty_stdout = 1;
			break;
		default:
			cfg.isatty_stdout = rng.bits(2, 255);
			break;
		}

		switch (rng.bits(0, 5)) {
		case 0:
			cfg.isatty_stderr = 0;
			break;
		case 1:
			cfg.isatty_stderr = 1;
			break;
		default:
			cfg.isatty_stderr = rng.bits(2, 255);
			break;
		}

		switch (rng.bits(0, 4)) {
		case 0:
			cfg.fstat_stdout = 0;
			break;
		case 1:
			cfg.fstat_stdout = -1;
			break;
		default:
			cfg.fstat_stdout = rng.bits(1, 255);
			break;
		}

		switch (rng.bits(0, 4)) {
		case 0:
			cfg.fstat_stderr = 0;
			break;
		case 1:
			cfg.fstat_stderr = -1;
			break;
		default:
			cfg.fstat_stderr = rng.bits(1, 255);
			break;
		}

		cfg.rdev_stdout = (dev_t)rng.next();
		if (rng.coin())
			cfg.rdev_stderr = cfg.rdev_stdout;
		else
			cfg.rdev_stderr = (dev_t)rng.next();

		run_checkredirect_case(cfg, "sweep");
	}
}

static void
print_table(void)
{
	const Stat *rows[] = { &st_checkredirect };

	std::printf("\n%-16s %10s %10s\n", "function", "cases", "failures");
	std::printf("%-16s %10ld %10ld\n", "----------", 0L, 0L);
	for (const Stat *st : rows)
		std::printf("%-16s %10ld %10ld\n", st->name, st->cases, st->fails);
}

} // namespace

int
main()
{
	test_checkredirect_edges();
	test_checkredirect_sweep();
	print_table();

	if (st_checkredirect.fails != 0)
		return 1;
	return 0;
}
