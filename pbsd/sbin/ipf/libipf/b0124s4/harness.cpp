/*
 * harness.cpp -- differential test for PBSD batch b0124s4.
 */

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

import pbsd.sbin.ipf.libipf.b0124s4;

namespace P = pbsd::sbin_ipf_libipf::b0124s4;

extern "C" {
size_t ref_msgdsize(P::mb_t *orig);
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

static Stat st_msgdsize = { "msgdsize", 0, 0, 0 };

static std::uint64_t rng_state = 0xb012404feedfaceULL;

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

static P::mb_t *
make_mb_chain(int count, const int *lens)
{
	P::mb_t *head = nullptr;
	P::mb_t *tail = nullptr;

	for (int i = 0; i < count; i++) {
		P::mb_t *m = (P::mb_t *)std::calloc(1, sizeof(P::mb_t));
		if (m == nullptr)
			std::abort();
		std::memset(m, GUARD, sizeof(P::mb_t));
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
	int bad_ret = (got != ref);
	int bad_buf = 0;

	if (count > 0) {
		P::mb_t *mp = hp;
		P::mb_t *mr = hr;
		for (int i = 0; i < count; i++) {
			if (std::memcmp(mp, mr, sizeof(P::mb_t)) != 0) {
				bad_buf = 1;
				break;
			}
			mp = mp->mb_next;
			mr = mr->mb_next;
		}
	}

	if (bad_ret || bad_buf) {
		stat_fail(&st_msgdsize, tag, bad_ret ? "return" : "buffer");
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
	static const int boundary[] = { 1, -1, 0, 1, -1 };
	static const int single_neg[] = { -5 };
	static const int single_max[] = { 0x7fffffff };
	static const int single_min[] = { -0x7fffffff };

	msgdsize_case("null", 0, nullptr);
	msgdsize_case("zero", 1, z);
	msgdsize_case("one", 1, one);
	msgdsize_case("two", 2, two);
	msgdsize_case("neg", 1, neg);
	msgdsize_case("mix", (int)(sizeof(mix) / sizeof(mix[0])), mix);
	msgdsize_case("hibyte", (int)(sizeof(hi) / sizeof(hi[0])), hi);
	msgdsize_case("boundary", (int)(sizeof(boundary) / sizeof(boundary[0])),
	    boundary);
	msgdsize_case("single_neg", 1, single_neg);
	msgdsize_case("single_max", 1, single_max);
	msgdsize_case("single_min", 1, single_min);

	static const int nul_heavy[] = { 0, 0, 0, 0, 0 };
	msgdsize_case("nul_heavy",
	    (int)(sizeof(nul_heavy) / sizeof(nul_heavy[0])), nul_heavy);

	static const int hibit[] = { 0x80, 0x81, 0xfe, 0xff };
	msgdsize_case("hibit", (int)(sizeof(hibit) / sizeof(hibit[0])), hibit);

	msgdsize_case("len_0", 1, z);
	msgdsize_case("len_neg1", 1, neg);
	msgdsize_case("len_1", 1, one);
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

int
main(void)
{
	test_msgdsize_edges();
	test_msgdsize_sweep();

	std::printf("\n%-14s %8s %8s\n", "function", "cases", "fails");
	std::printf("%-14s %8ld %8ld\n", st_msgdsize.name, st_msgdsize.cases,
	    st_msgdsize.fails);

	return st_msgdsize.fails == 0 ? 0 : 1;
}
