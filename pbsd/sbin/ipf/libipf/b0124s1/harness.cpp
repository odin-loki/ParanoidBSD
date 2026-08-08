/*
 * harness.cpp -- differential test for PBSD batch b0124s1.
 */

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <unistd.h>

import pbsd.sbin.ipf.libipf.b0124s1;

namespace P = pbsd::sbin_ipf_libipf::b0124s1;

extern "C" void ref_printifname(char *format, char *name, void *ifp);

static constexpr unsigned char GUARD = 0x7f;
static constexpr int MAX_REPORT = 8;
static constexpr long SWEEP = 200000;

struct Stat {
	const char *name;
	long cases;
	long fails;
	int reported;
};

static Stat st_printifname = { "printifname", 0, 0, 0 };

static std::uint64_t rng_state = 0xb0124s1feedfaceULL;

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
capture_stdout(void (*fn)(char *, char *, void *), char *format, char *name,
    void *ifp)
{
	std::fflush(stdout);
	int fds[2];
	if (pipe(fds) != 0)
		std::abort();
	int saved = dup(STDOUT_FILENO);
	dup2(fds[1], STDOUT_FILENO);
	close(fds[1]);
	fn(format, name, ifp);
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
call_port_printifname(char *format, char *name, void *ifp)
{
	P::printifname(format, name, ifp);
}

static void
fill_str(char *buf, std::size_t cap, int pattern, std::size_t len)
{
	static const unsigned char alpha[] = {
	    0x00, 0x01, 0x2d, 0x2a, 0x7e, 0x7f, 0x80, 0xfe, 0xff, 'a', '-', '*'
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
			buf[i] = (char)((i & 1) ? '-' : '*');
			break;
		default:
			buf[i] = (char)(rnd() & 0xff);
			break;
		}
	}
	buf[len] = '\0';
}

static void
printifname_case(const char *tag, const char *fmt, const char *name, void *ifp)
{
	char fa[128], fb[128], na[128], nb[128];

	std::memset(fa, GUARD, sizeof(fa));
	std::memset(fb, GUARD, sizeof(fb));
	std::memset(na, GUARD, sizeof(na));
	std::memset(nb, GUARD, sizeof(nb));

	std::memcpy(fa + 16, fmt, std::strlen(fmt) + 1);
	std::memcpy(fb + 16, fmt, std::strlen(fmt) + 1);
	std::memcpy(na + 16, name, std::strlen(name) + 1);
	std::memcpy(nb + 16, name, std::strlen(name) + 1);

	char *pfmt = fa + 16;
	char *pname = na + 16;
	char *rfmt = fb + 16;
	char *rname = nb + 16;

	std::string got = capture_stdout(call_port_printifname, pfmt, pname, ifp);
	std::string ref = capture_stdout(ref_printifname, rfmt, rname, ifp);

	st_printifname.cases++;
	int bad_out = (got != ref);
	int bad_fmt = (std::memcmp(fa, fb, sizeof(fa)) != 0);
	int bad_name = (std::memcmp(na, nb, sizeof(na)) != 0);
	if (bad_out || bad_fmt || bad_name) {
		stat_fail(&st_printifname, tag, bad_out ? "stdout" : "buf");
		if (st_printifname.reported <= MAX_REPORT) {
			std::printf("      port=%.*s ref=%.*s\n",
			    (int)got.size(), got.c_str(), (int)ref.size(),
			    ref.c_str());
		}
	}
}

static void
test_printifname_edges(void)
{
	static const char *fmts[] = { "", "fmt:", "x", "\xff\xfe", "%%" };
	static const char *names[] = { "", "-", "*", "eth0", "lo0", "\xff" };
	static void *ifps[] = { nullptr, (void *)0x1, (void *)0xdeadbeefUL };

	for (std::size_t i = 0; i < sizeof(fmts) / sizeof(fmts[0]); i++) {
		for (std::size_t j = 0; j < sizeof(names) / sizeof(names[0]);
		     j++) {
			for (std::size_t k = 0; k < sizeof(ifps) / sizeof(ifps[0]);
			     k++) {
				char tag[64];
				std::snprintf(tag, sizeof(tag), "edge f%zu n%zu i%zu",
				    i, j, k);
				printifname_case(tag, fmts[i], names[j], ifps[k]);
			}
		}
	}

	printifname_case("null-ifp-plain", "fmt:", "eth0", nullptr);
	printifname_case("null-ifp-dash", "fmt:", "-", nullptr);
	printifname_case("null-ifp-star", "fmt:", "*", nullptr);
	printifname_case("nonnull-ifp-plain", "fmt:", "eth0", (void *)1);
	printifname_case("nonnull-ifp-dash", "fmt:", "-", (void *)1);
	printifname_case("nonnull-ifp-star", "fmt:", "*", (void *)1);

	char hi_fmt[8] = { (char)0x80, (char)0xff, ':', '\0' };
	char hi_name[8] = { (char)0xfe, (char)0x81, '\0' };
	printifname_case("hibyte", hi_fmt, hi_name, nullptr);
	printifname_case("hibyte-ifp", hi_fmt, hi_name, (void *)1);

	char nul_heavy[16];
	for (int i = 0; i < 8; i++)
		nul_heavy[i] = '\0';
	nul_heavy[8] = 'a';
	nul_heavy[9] = '\0';
	printifname_case("nul-heavy", "", nul_heavy, nullptr);
}

static void
test_printifname_sweep(void)
{
	char fmt[96], name[96];
	static const char *special[] = { "-", "*", "", "a", "zz", "eth0", "lo" };

	for (long i = 0; i < SWEEP; i++) {
		int pat = (int)(rnd() % 4);
		std::size_t flen = rnd_mod(sizeof(fmt) - 1);
		std::size_t nlen = rnd_mod(sizeof(name) - 1);
		fill_str(fmt, sizeof(fmt), pat, flen);
		fill_str(name, sizeof(name), pat ^ 1, nlen);
		if ((rnd() & 7) == 0)
			std::snprintf(name, sizeof(name), "%s",
			    special[rnd_mod(sizeof(special) / sizeof(special[0]))]);
		void *ifp;
		switch (rnd() & 7) {
		case 0:
			ifp = nullptr;
			break;
		case 1:
			ifp = (void *)1;
			break;
		default:
			ifp = (void *)(uintptr_t)rnd();
			break;
		}
		char tag[48];
		std::snprintf(tag, sizeof(tag), "rnd%ld", i);
		printifname_case(tag, fmt, name, ifp);
	}
}

int
main(void)
{
	test_printifname_edges();
	test_printifname_sweep();

	std::printf("\n%-14s %8s %8s\n", "function", "cases", "fails");
	std::printf("%-14s %8ld %8ld\n", st_printifname.name, st_printifname.cases,
	    st_printifname.fails);

	return st_printifname.fails == 0 ? 0 : 1;
}
