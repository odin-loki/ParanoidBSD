/*
 * harness.cpp -- differential test for PBSD batch b0124s2.
 */

#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

import pbsd.sbin.ipf.libipf.b0124s2;

namespace P = pbsd::sbin_ipf_libipf::b0124s2;

extern "C" {
void ref_resetlexer(void);
extern long string_start;
extern long string_end;
extern char *string_val;
extern long pos;
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

static Stat st_resetlexer = { "resetlexer", 0, 0, 0 };

static std::uint64_t rng_state = 0xb0124002feedfaceULL;

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

static int
globals_match_expected(long ss, long se, char *sv, long p)
{
	return ss == -1 && se == -1 && sv == nullptr && p == 0;
}

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

	int bad = 0;
	if (P::string_start != string_start)
		bad = 1;
	if (P::string_end != string_end)
		bad = 1;
	if (P::string_val != string_val)
		bad = 1;
	if (P::pos != pos)
		bad = 1;
	if (!globals_match_expected(P::string_start, P::string_end,
		P::string_val, P::pos))
		bad = 1;
	if (!globals_match_expected(string_start, string_end, string_val, pos))
		bad = 1;

	if (bad) {
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
	char blob[64];
	char nul_heavy[32];

	std::memset(blob, GUARD, sizeof(blob));
	std::memset(nul_heavy, 0, sizeof(nul_heavy));

	resetlexer_check("default", -1, -1, nullptr, 0);
	resetlexer_check("zeros", 0, 0, nullptr, 0);
	resetlexer_check("pos1", 1, 2, nullptr, 1);
	resetlexer_check("max", LONG_MAX, LONG_MIN, blob, LONG_MAX);
	resetlexer_check("min", LONG_MIN, LONG_MAX, blob + 16, LONG_MIN);
	resetlexer_check("neg", -2, -3, blob + 8, -99);
	resetlexer_check("ptr", 42, 43, blob + 4, 7);
	resetlexer_check("one", 1, 1, blob + 1, 1);
	resetlexer_check("boundary_m1", -1, -1, nullptr, -1);
	resetlexer_check("boundary_0", 0, 0, nullptr, 0);
	resetlexer_check("boundary_1", 1, 1, nullptr, 1);
	resetlexer_check("nul_ptr", 5, 6, nullptr, 5);
	resetlexer_check("nul_heavy", 0x80, 0xff, nul_heavy, 0x7f);
	resetlexer_check("hibyte_ss", 0x80, 0xff, blob, 0xfe);
	resetlexer_check("hibyte_se", 0xff, 0x80, blob + 32, 0x81);
	resetlexer_check("single_char", 1, 0, blob + 2, 1);
	resetlexer_check("empty_state", -1, -1, blob, 0);
	resetlexer_check("ptr_end", 10, 20, blob + sizeof(blob) - 1, 3);
}

static void
test_resetlexer_sweep(void)
{
	char arena[512];

	for (long i = 0; i < SWEEP; i++) {
		long ss = (long)(std::int64_t)rnd();
		long se = (long)(std::int64_t)rnd();
		long p = (long)(std::int64_t)rnd();
		char *sv;

		switch ((int)(rnd() % 5)) {
		case 0:
			sv = nullptr;
			break;
		case 1:
			sv = arena;
			break;
		case 2:
			sv = arena + rnd_mod(sizeof(arena));
			break;
		case 3:
			sv = arena + sizeof(arena) - 1;
			break;
		default:
			sv = arena + rnd_mod(sizeof(arena) - 1) + 1;
			break;
		}

		char tag[48];
		std::snprintf(tag, sizeof(tag), "rnd%ld", i);
		resetlexer_check(tag, ss, se, sv, p);
	}
}

int
main(void)
{
	test_resetlexer_edges();
	test_resetlexer_sweep();

	std::printf("\n%-14s %8s %8s\n", "function", "cases", "fails");
	std::printf("%-14s %8ld %8ld\n", st_resetlexer.name, st_resetlexer.cases,
	    st_resetlexer.fails);

	return st_resetlexer.fails == 0 ? 0 : 1;
}
