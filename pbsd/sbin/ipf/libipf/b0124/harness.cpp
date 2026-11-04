/*
 * harness.cpp -- differential test for PBSD batch b0124.
 */

#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <unistd.h>

import pbsd.sbin.ipf.libipf.b0124;

namespace P = pbsd::sbin_ipf_libipf::b0124;

extern "C" {
void ref_printifname(char *format, char *name, void *ifp);
void ref_resetlexer(void);
void ref_alist_free(P::alist_t *hosts);
size_t ref_msgdsize(P::mb_t *orig);
extern long string_start;
extern long string_end;
extern char *string_val;
extern long pos;
}

extern "C" void __real_free(void *);
extern "C" void __wrap_free(void *);

/* ------------------------------------------------------------------------ */

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
static Stat st_resetlexer = { "resetlexer", 0, 0, 0 };
static Stat st_alist_free = { "alist_free", 0, 0, 0 };
static Stat st_msgdsize = { "msgdsize", 0, 0, 0 };

static std::uint64_t rng_state = 0xb0124feedfaceULL;

static void *g_freed[65536];
static int g_nfreed;

extern "C" void
__wrap_free(void *p)
{
	if (g_nfreed < (int)(sizeof(g_freed) / sizeof(g_freed[0])))
		g_freed[g_nfreed++] = p;
	__real_free(p);
}

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

static void
reset_freed(void)
{
	g_nfreed = 0;
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

/* ------------------------------------------------------------------------ */
/* printifname                                                               */
/* ------------------------------------------------------------------------ */

static void
printifname_case(const char *tag, const char *fmt, const char *name, void *ifp,
    int pattern)
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
	(void)pattern;

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
				printifname_case(tag, fmts[i], names[j], ifps[k],
				    0);
			}
		}
	}

	char hi_fmt[8] = { (char)0x80, (char)0xff, ':', '\0' };
	char hi_name[8] = { (char)0xfe, (char)0x81, '\0' };
	printifname_case("hibyte", hi_fmt, hi_name, nullptr, 1);
	printifname_case("hibyte-ifp", hi_fmt, hi_name, (void *)1, 1);
}

static void
test_printifname_sweep(void)
{
	char fmt[96], name[96];
	static const char *special[] = { "-", "*", "", "a", "zz" };

	for (long i = 0; i < SWEEP; i++) {
		int pat = (int)(rnd() % 4);
		std::size_t flen = rnd_mod(sizeof(fmt) - 1);
		std::size_t nlen = rnd_mod(sizeof(name) - 1);
		fill_str(fmt, sizeof(fmt), pat, flen);
		fill_str(name, sizeof(name), pat ^ 1, nlen);
		if ((rnd() & 7) == 0)
			std::snprintf(name, sizeof(name), "%s",
			    special[rnd_mod(sizeof(special) / sizeof(special[0]))]);
		void *ifp = ((rnd() & 1) == 0) ? nullptr : (void *)(uintptr_t)rnd();
		char tag[48];
		std::snprintf(tag, sizeof(tag), "rnd%ld", i);
		printifname_case(tag, fmt, name, ifp, pat);
	}
}

/* ------------------------------------------------------------------------ */
/* resetlexer                                                                */
/* ------------------------------------------------------------------------ */

static void
resetlexer_check(const char *tag, long ss, long se, char *sv, long p)
{
	P::string_start = ss;
	P::string_end = se;
	P::string_val = sv;
	P::pos = p;

	string_start = ss;
	string_end = se;
	string_val = sv;
	pos = p;

	P::resetlexer();
	ref_resetlexer();

	st_resetlexer.cases++;
	if (P::string_start != string_start || P::string_end != string_end ||
	    P::string_val != string_val || P::pos != pos ||
	    P::string_start != -1 || P::string_end != -1 ||
	    P::string_val != nullptr || P::pos != 0) {
		stat_fail(&st_resetlexer, tag, "globals");
		if (st_resetlexer.reported <= MAX_REPORT) {
			std::printf(
			    "      port ss=%ld se=%ld sv=%p pos=%ld ref ss=%ld se=%ld sv=%p pos=%ld\n",
			    P::string_start, P::string_end,
			    (void *)P::string_val, P::pos, string_start,
			    string_end, (void *)string_val, pos);
		}
	}
}

static void
test_resetlexer_edges(void)
{
	char blob[32];

	std::memset(blob, GUARD, sizeof(blob));
	resetlexer_check("default", -1, -1, nullptr, 0);
	resetlexer_check("zeros", 0, 0, nullptr, 0);
	resetlexer_check("pos1", 1, 2, nullptr, 1);
	resetlexer_check("max", LONG_MAX, LONG_MIN, blob, LONG_MAX);
	resetlexer_check("neg", -2, -3, blob + 8, -99);
	resetlexer_check("ptr", 42, 43, blob + 4, 7);
}

static void
test_resetlexer_sweep(void)
{
	char arena[256];

	for (long i = 0; i < SWEEP; i++) {
		long ss = (long)(std::int64_t)rnd();
		long se = (long)(std::int64_t)rnd();
		long p = (long)(std::int64_t)rnd();
		char *sv = ((rnd() & 3) == 0) ?
		    nullptr :
		    arena + rnd_mod(sizeof(arena));
		char tag[48];
		std::snprintf(tag, sizeof(tag), "rnd%ld", i);
		resetlexer_check(tag, ss, se, sv, p);
	}
}

/* ------------------------------------------------------------------------ */
/* alist_free                                                                */
/* ------------------------------------------------------------------------ */

static P::alist_t *
make_alist_chain(int count, std::uint64_t seed, P::alist_t **nodes_out)
{
	P::alist_t *head = nullptr;
	P::alist_t *tail = nullptr;

	for (int i = 0; i < count; i++) {
		P::alist_t *a =
		    (P::alist_t *)std::malloc(sizeof(P::alist_t));
		if (a == nullptr)
			std::abort();
		std::memset(a, (int)((seed + (std::uint64_t)i) & 0xff),
		    sizeof(P::alist_t));
		a->al_next = nullptr;
		if (nodes_out != nullptr)
			nodes_out[i] = a;
		if (head == nullptr)
			head = tail = a;
		else {
			tail->al_next = a;
			tail = a;
		}
	}
	return head;
}

static void
alist_free_case(const char *tag, int count, std::uint64_t seed)
{
	P::alist_t *nodes_p[64];
	P::alist_t *nodes_r[64];
	P::alist_t *hp = nullptr;
	P::alist_t *hr = nullptr;

	if (count > 0) {
		hp = make_alist_chain(count, seed, nodes_p);
		hr = make_alist_chain(count, seed + 0x1000, nodes_r);
	}

	reset_freed();
	P::alist_free(hp);
	void *freed_p[64];
	int nfreed_p = g_nfreed;
	for (int i = 0; i < nfreed_p; i++)
		freed_p[i] = g_freed[i];

	reset_freed();
	ref_alist_free(hr);
	void *freed_r[64];
	int nfreed_r = g_nfreed;
	for (int i = 0; i < nfreed_r; i++)
		freed_r[i] = g_freed[i];

	st_alist_free.cases++;
	int bad = 0;
	if (nfreed_p != nfreed_r || nfreed_p != count)
		bad = 1;
	for (int i = 0; !bad && i < nfreed_p; i++) {
		if (freed_p[i] != nodes_p[i] || freed_r[i] != nodes_r[i])
			bad = 1;
	}
	if (bad)
		stat_fail(&st_alist_free, tag, "free order/count");
}

static void
test_alist_free_edges(void)
{
	alist_free_case("null", 0, 0);
	alist_free_case("one", 1, 1);
	alist_free_case("two", 2, 2);
	alist_free_case("many", 16, 0xabc);
}

static void
test_alist_free_sweep(void)
{
	for (long i = 0; i < SWEEP; i++) {
		int count = (int)(rnd_mod(32) + 1);
		char tag[48];
		std::snprintf(tag, sizeof(tag), "rnd%ld", i);
		alist_free_case(tag, count, rnd());
	}
}

/* ------------------------------------------------------------------------ */
/* msgdsize                                                                  */
/* ------------------------------------------------------------------------ */

static P::mb_t *
make_mb_chain(int count, const int *lens)
{
	P::mb_t *head = nullptr;
	P::mb_t *tail = nullptr;

	for (int i = 0; i < count; i++) {
		P::mb_t *m = (P::mb_t *)std::calloc(1, sizeof(P::mb_t));
		if (m == nullptr)
			std::abort();
		m->mb_len = lens[i];
		m->mb_next = nullptr;
		if (head == nullptr)
			head = tail = m;
		else {
			tail->mb_next = m;
			tail = m;
		}
	}
	return head;
}

static void
free_mb_chain(P::mb_t *m)
{
	while (m != nullptr) {
		P::mb_t *n = m->mb_next;
		std::free(m);
		m = n;
	}
}

static void
msgdsize_case(const char *tag, int count, const int *lens)
{
	P::mb_t *hp = (count > 0) ? make_mb_chain(count, lens) : nullptr;
	P::mb_t *hr = (count > 0) ? make_mb_chain(count, lens) : nullptr;

	std::size_t got = P::msgdsize(hp);
	std::size_t ref = ref_msgdsize(hr);

	st_msgdsize.cases++;
	if (got != ref) {
		stat_fail(&st_msgdsize, tag, "return");
		if (st_msgdsize.reported <= MAX_REPORT)
			std::printf("      port=%zu ref=%zu\n", got, ref);
	}

	free_mb_chain(hp);
	free_mb_chain(hr);
}

static void
test_msgdsize_edges(void)
{
	static const int z[] = { 0 };
	static const int one[] = { 5 };
	static const int two[] = { 3, 4 };
	static const int neg[] = { -1 };
	static const int mix[] = { 0, -2, 7, 0x7fffffff, -0x7fffffff };
	static const int hi[] = { 0x80, 0xff, 0x100 };

	msgdsize_case("null", 0, nullptr);
	msgdsize_case("zero", 1, z);
	msgdsize_case("one", 1, one);
	msgdsize_case("two", 2, two);
	msgdsize_case("neg", 1, neg);
	msgdsize_case("mix", (int)(sizeof(mix) / sizeof(mix[0])), mix);
	msgdsize_case("hibyte", (int)(sizeof(hi) / sizeof(hi[0])), hi);
}

static void
test_msgdsize_sweep(void)
{
	int lens[48];

	for (long i = 0; i < SWEEP; i++) {
		int count = (int)(rnd_mod(32) + 1);
		for (int j = 0; j < count; j++)
			lens[j] = (int)(std::int32_t)rnd();
		char tag[48];
		std::snprintf(tag, sizeof(tag), "rnd%ld", i);
		msgdsize_case(tag, count, lens);
	}
}

/* ------------------------------------------------------------------------ */

int
main(void)
{
	test_printifname_edges();
	test_printifname_sweep();
	test_resetlexer_edges();
	test_resetlexer_sweep();
	test_alist_free_edges();
	test_alist_free_sweep();
	test_msgdsize_edges();
	test_msgdsize_sweep();

	std::printf("\n%-14s %8s %8s\n", "function", "cases", "fails");
	std::printf("%-14s %8ld %8ld\n", st_printifname.name, st_printifname.cases,
	    st_printifname.fails);
	std::printf("%-14s %8ld %8ld\n", st_resetlexer.name, st_resetlexer.cases,
	    st_resetlexer.fails);
	std::printf("%-14s %8ld %8ld\n", st_alist_free.name, st_alist_free.cases,
	    st_alist_free.fails);
	std::printf("%-14s %8ld %8ld\n", st_msgdsize.name, st_msgdsize.cases,
	    st_msgdsize.fails);

	long total_fails = st_printifname.fails + st_resetlexer.fails +
	    st_alist_free.fails + st_msgdsize.fails;
	return total_fails == 0 ? 0 : 1;
}
