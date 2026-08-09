// Differential harness for PBSD batch b0187s3 (cchar.c: c_cchar, csearch).

import pbsd.bin.stty.b0187s3;

#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <sys/ioctl.h>
#include <termios.h>

namespace P = pbsd::bin_stty::b0187s3;

extern "C" {
int ref_c_cchar(const void *, const void *);
int ref_csearch(char ***, P::info *);
}

namespace {

constexpr unsigned char GUARD = 0x7f;
constexpr long SWEEP = 200000L;
constexpr int MAX_SHOW = 8;

struct Stat {
	const char *name;
	long cases;
	long fails;
	int shown;
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

	unsigned char byte()
	{
		return (unsigned char)(next() & 0xffu);
	}
};

Rng rng((0xb0187s3ULL << 32) | 0xccharULL);
Stat st_c_cchar = { "c_cchar", 0, 0, 0 };
Stat st_csearch = { "csearch", 0, 0, 0 };

static bool g_test_active;
static int g_warnx_count;
static int g_usage_count;

static const char *k_names1[] = {
	"discard", "dsusp", "eof", "eol", "eol2", "erase", "erase2", "intr",
	"kill", "lnext", "min", "quit", "reprint", "start", "status", "stop",
	"susp", "time", "werase",
};
static const char *k_names2[] = { "brk", "flush", "rprnt" };

static void
fill_guard(void *p, size_t n)
{
	std::memset(p, GUARD, n);
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
run_c_cchar_case(const P::cchar *a, const P::cchar *b)
{
	int ref_r;
	int port_r;

	st_c_cchar.cases++;

	ref_r = ref_c_cchar(a, b);
	port_r = P::c_cchar(a, b);

	if (ref_r != port_r)
		return fail(st_c_cchar, "return mismatch");

	return true;
}

struct CsearchSetup {
	char name_buf[96];
	char arg_buf[96];
	char *av[4];
	char **ap;
	char ***argvp;
	P::info ip;
};

static void
init_csearch_setup(CsearchSetup &s)
{
	fill_guard(s.name_buf, sizeof(s.name_buf));
	fill_guard(s.arg_buf, sizeof(s.arg_buf));
	fill_guard(&s.ip, sizeof(s.ip));
	s.av[0] = s.name_buf;
	s.av[1] = s.arg_buf;
	s.av[2] = nullptr;
	s.av[3] = nullptr;
	s.ap = s.av;
	s.argvp = &s.ap;
}

static bool
run_csearch_pair(CsearchSetup &ref_s, CsearchSetup &port_s, bool expect_usage)
{
	int ref_r;
	int port_r;
	ptrdiff_t ref_off;
	ptrdiff_t port_off;

	st_csearch.cases++;

	g_test_active = true;
	g_warnx_count = 0;
	g_usage_count = 0;
	ref_r = ref_csearch(ref_s.argvp, &ref_s.ip);
	int ref_warn = g_warnx_count;
	int ref_usage = g_usage_count;

	g_test_active = true;
	g_warnx_count = 0;
	g_usage_count = 0;
	port_r = P::csearch(port_s.argvp, &port_s.ip);
	int port_warn = g_warnx_count;
	int port_usage = g_usage_count;
	g_test_active = false;

	if (ref_r != port_r)
		return fail(st_csearch, "return mismatch");

	if (ref_warn != port_warn)
		return fail(st_csearch, "warnx count mismatch");

	if (ref_usage != port_usage)
		return fail(st_csearch, "usage count mismatch");

	if (expect_usage && ref_usage == 0)
		return fail(st_csearch, "expected usage not called");

	ref_off = ref_s.ap - ref_s.av;
	port_off = port_s.ap - port_s.av;
	if (ref_off != port_off)
		return fail(st_csearch, "argvp offset mismatch");

	if (std::memcmp(&ref_s.ip, &port_s.ip, sizeof(P::info)) != 0)
		return fail(st_csearch, "info buffer mismatch");

	return true;
}

static bool
run_csearch_case(const char *name, const char *arg, bool has_arg)
{
	CsearchSetup ref_s;
	CsearchSetup port_s;

	init_csearch_setup(ref_s);
	init_csearch_setup(port_s);

	std::strncpy(ref_s.name_buf, name, sizeof(ref_s.name_buf) - 1);
	std::strncpy(port_s.name_buf, name, sizeof(port_s.name_buf) - 1);

	if (has_arg && arg != nullptr) {
		std::strncpy(ref_s.arg_buf, arg, sizeof(ref_s.arg_buf) - 1);
		std::strncpy(port_s.arg_buf, arg, sizeof(port_s.arg_buf) - 1);
	} else if (!has_arg) {
		ref_s.av[1] = nullptr;
		port_s.av[1] = nullptr;
	}

	return run_csearch_pair(ref_s, port_s, !has_arg);
}

static void
test_c_cchar_edges(void)
{
	P::cchar a;
	P::cchar b;
	P::cchar z;

	fill_guard(&a, sizeof(a));
	fill_guard(&b, sizeof(b));
	fill_guard(&z, sizeof(z));

	a.name = "eof";
	b.name = "eof";
	run_c_cchar_case(&a, &b);

	a.name = "eof";
	b.name = "eol";
	run_c_cchar_case(&a, &b);

	a.name = "";
	b.name = "";
	run_c_cchar_case(&a, &b);

	a.name = "a";
	b.name = "b";
	run_c_cchar_case(&a, &b);

	a.name = "eof";
	b.name = "eo";
	run_c_cchar_case(&a, &b);

	a.name = "discard";
	b.name = "flush";
	run_c_cchar_case(&a, &b);

	a.name = "brk";
	b.name = "flush";
	run_c_cchar_case(&a, &b);

	a.name = "min";
	b.name = "time";
	run_c_cchar_case(&a, &b);

	for (const char *n : k_names1) {
		a.name = n;
		b.name = n;
		run_c_cchar_case(&a, &b);
	}
	for (const char *n : k_names2) {
		a.name = n;
		z.name = n;
		run_c_cchar_case(&a, &z);
	}

	a.name = "undef";
	b.name = "<undef>";
	run_c_cchar_case(&a, &b);

	a.name = "status";
	b.name = "start";
	run_c_cchar_case(&a, &b);
}

static void
test_c_cchar_sweep(void)
{
	char buf_a[32];
	char buf_b[32];
	P::cchar a;
	P::cchar b;

	for (long i = 0; i < SWEEP; i++) {
		int la = rng.bits(0, 16);
		int lb = rng.bits(0, 16);

		fill_guard(buf_a, sizeof(buf_a));
		fill_guard(buf_b, sizeof(buf_b));
		for (int j = 0; j < la; j++)
			buf_a[j] = (char)rng.byte();
		buf_a[la] = '\0';
		for (int j = 0; j < lb; j++)
			buf_b[j] = (char)rng.byte();
		buf_b[lb] = '\0';

		fill_guard(&a, sizeof(a));
		fill_guard(&b, sizeof(b));
		a.name = buf_a;
		b.name = buf_b;
		run_c_cchar_case(&a, &b);

		if (rng.bits(0, 7) == 0) {
			const char *pick = k_names1[rng.bits(0, 18)];
			a.name = pick;
			b.name = pick;
			run_c_cchar_case(&a, &b);
		}
	}
}

static void
test_csearch_edges(void)
{
	/* not found */
	run_csearch_case("notfound", "x", true);
	run_csearch_case("", "x", true);
	run_csearch_case("eo", "x", true);
	run_csearch_case("EOF", "x", true);
	run_csearch_case("discardx", "x", true);
	run_csearch_case("undef", "x", true); /* name undef is not a cchar name */

	/* missing argument */
	run_csearch_case("eof", nullptr, false);
	run_csearch_case("min", nullptr, false);
	run_csearch_case("brk", nullptr, false);

	/* undef paths */
	run_csearch_case("eof", "undef", true);
	run_csearch_case("eof", "<undef>", true);
	run_csearch_case("min", "undef", true);
	run_csearch_case("time", "<undef>", true);
	run_csearch_case("brk", "undef", true);

	/* VMIN / VTIME numeric */
	run_csearch_case("min", "0", true);
	run_csearch_case("min", "1", true);
	run_csearch_case("min", "255", true);
	run_csearch_case("time", "0", true);
	run_csearch_case("time", "255", true);
	run_csearch_case("min", "256", true);
	run_csearch_case("time", "300", true);
	run_csearch_case("min", "12x", true);
	run_csearch_case("time", "1a", true);
	run_csearch_case("min", "", true);
	run_csearch_case("time", " 1", true);

	/* caret escapes */
	run_csearch_case("eof", "^?", true);
	run_csearch_case("intr", "^-", true);
	run_csearch_case("intr", "^A", true);
	run_csearch_case("intr", "^@", true);
	run_csearch_case("intr", "^a", true);
	run_csearch_case("intr", "^Z", true);
	run_csearch_case("intr", "^", true);
	run_csearch_case("eof", "^?", true);
	run_csearch_case("kill", "^-", true);

	/* plain char */
	run_csearch_case("eof", "x", true);
	run_csearch_case("intr", "\x03", true);
	run_csearch_case("eof", "\xff", true);
	run_csearch_case("eof", "\x80", true);
	run_csearch_case("quit", "q", true);

	/* cchars2 aliases */
	run_csearch_case("brk", "x", true);
	run_csearch_case("flush", "undef", true);
	run_csearch_case("rprnt", "^C", true);

	/* every cchars1 name with distinct arg kinds */
	for (const char *n : k_names1) {
		run_csearch_case(n, "a", true);
		if (std::strcmp(n, "min") == 0 || std::strcmp(n, "time") == 0)
			run_csearch_case(n, "42", true);
		else
			run_csearch_case(n, "^B", true);
	}

	/* boundary: eof uses char path, min uses numeric */
	run_csearch_case("eof", "0", true);
	run_csearch_case("min", "x", true);

	/* CHK first-char boundary */
	run_csearch_case("eof", "udef", true);
	run_csearch_case("eof", "<undeff>", true);
}

static void
test_csearch_sweep(void)
{
	for (long i = 0; i < SWEEP; i++) {
		char name[48];
		char arg[48];
		bool has_arg;
		int pick;

		fill_guard(name, sizeof(name));
		fill_guard(arg, sizeof(arg));

		switch (rng.bits(0, 9)) {
		case 0:
			std::strcpy(name, k_names1[rng.bits(0, 18)]);
			break;
		case 1:
			std::strcpy(name, k_names2[rng.bits(0, 2)]);
			break;
		case 2:
			name[0] = '\0';
			break;
		default:
			int ln = rng.bits(0, 20);
			for (int j = 0; j < ln; j++)
				name[j] = (char)rng.byte();
			name[ln] = '\0';
			break;
		}

		has_arg = rng.coin();
		if (has_arg) {
			pick = rng.bits(0, 11);
			switch (pick) {
			case 0:
				std::strcpy(arg, "undef");
				break;
			case 1:
				std::strcpy(arg, "<undef>");
				break;
			case 2:
				std::sprintf(arg, "%d", rng.bits(0, 400));
				break;
			case 3:
				arg[0] = '^';
				arg[1] = (char)rng.byte();
				arg[2] = '\0';
				break;
			case 4:
				arg[0] = '^';
				arg[1] = '?';
				arg[2] = '\0';
				break;
			case 5:
				arg[0] = '^';
				arg[1] = '-';
				arg[2] = '\0';
				break;
			case 6:
				arg[0] = (char)rng.byte();
				arg[1] = '\0';
				break;
			default:
				int la = rng.bits(0, 12);
				for (int j = 0; j < la; j++)
					arg[j] = (char)rng.byte();
				arg[la] = '\0';
				break;
			}
		}

		run_csearch_case(name, has_arg ? arg : nullptr, has_arg);
	}
}

static void
print_table(void)
{
	const Stat *rows[] = { &st_c_cchar, &st_csearch };

	std::printf("\n%-16s %10s %10s\n", "function", "cases", "failures");
	std::printf("%-16s %10s %10s\n", "----------", "----", "--------");
	for (const Stat *st : rows)
		std::printf("%-16s %10ld %10ld\n", st->name, st->cases, st->fails);
}

} // namespace

extern "C" void
__wrap_warnx(const char *fmt, ...)
{
	if (g_test_active) {
		g_warnx_count++;
		return;
	}
	va_list ap;
	va_start(ap, fmt);
	std::vfprintf(stderr, fmt, ap);
	va_end(ap);
	std::fputc('\n', stderr);
}

extern "C" void
__wrap_usage(void)
{
	if (g_test_active) {
		g_usage_count++;
		return;
	}
	std::abort();
}

int
main()
{
	test_c_cchar_edges();
	test_c_cchar_sweep();
	test_csearch_edges();
	test_csearch_sweep();
	print_table();

	if (st_c_cchar.fails != 0 || st_csearch.fails != 0)
		return 1;
	return 0;
}
