/*
 * harness.cpp -- differential test for PBSD batch b0124s3.
 */

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

import pbsd.sbin.ipf.libipf.b0124s3;

namespace P = pbsd::sbin_ipf_libipf::b0124s3;

extern "C" void ref_alist_free(P::alist_t *hosts);

extern "C" void __real_free(void *);
extern "C" void __wrap_free(void *);

static constexpr unsigned char GUARD = 0x7f;
static constexpr int MAX_REPORT = 8;
static constexpr long SWEEP = 200000;
static constexpr int MAX_CHAIN = 64;

struct Stat {
	const char *name;
	long cases;
	long fails;
	int reported;
};

static Stat st_alist_free = { "alist_free", 0, 0, 0 };

static std::uint64_t rng_state = 0xb012403feedfaceULL;

static void *g_freed[MAX_CHAIN];
static int g_nfreed;

extern "C" void
__wrap_free(void *p)
{
	if (g_nfreed < MAX_CHAIN)
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
	P::alist_t *nodes_p[MAX_CHAIN];
	P::alist_t *nodes_r[MAX_CHAIN];
	P::alist_t *hp = nullptr;
	P::alist_t *hr = nullptr;

	if (count > 0) {
		hp = make_alist_chain(count, seed, nodes_p);
		hr = make_alist_chain(count, seed + 0x1000, nodes_r);
	}

	reset_freed();
	P::alist_free(hp);
	void *freed_p[MAX_CHAIN];
	int nfreed_p = g_nfreed;
	for (int i = 0; i < nfreed_p; i++)
		freed_p[i] = g_freed[i];

	reset_freed();
	ref_alist_free(hr);
	void *freed_r[MAX_CHAIN];
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
	if (bad) {
		stat_fail(&st_alist_free, tag, "free order/count");
		if (st_alist_free.reported <= MAX_REPORT) {
			std::printf(
			    "      count=%d port_nf=%d ref_nf=%d\n", count,
			    nfreed_p, nfreed_r);
		}
	}
}

static void
test_alist_free_edges(void)
{
	alist_free_case("null", 0, 0);
	alist_free_case("one", 1, 1);
	alist_free_case("two", 2, 2);
	alist_free_case("three", 3, 3);
	alist_free_case("boundary-31", 31, 0x31);
	alist_free_case("boundary-32", 32, 0x32);
	alist_free_case("boundary-63", 63, 0x63);
	alist_free_case("boundary-64", 64, 0x64);
	alist_free_case("hibyte-seed", 8, 0x8080808080808080ULL);
	alist_free_case("nul-heavy", 4, 0x00);
	alist_free_case("ff-heavy", 4, 0xff);
	alist_free_case("mixed-hibyte", 12, 0x7f80fe81ULL);
}

static void
test_alist_free_sweep(void)
{
	for (long i = 0; i < SWEEP; i++) {
		int count = (int)rnd_mod(MAX_CHAIN + 1);
		char tag[48];
		std::snprintf(tag, sizeof(tag), "rnd%ld", i);
		alist_free_case(tag, count, rnd());
	}
}

int
main(void)
{
	test_alist_free_edges();
	test_alist_free_sweep();

	std::printf("\n%-14s %8s %8s\n", "function", "cases", "fails");
	std::printf("%-14s %8ld %8ld\n", st_alist_free.name, st_alist_free.cases,
	    st_alist_free.fails);

	return st_alist_free.fails == 0 ? 0 : 1;
}
