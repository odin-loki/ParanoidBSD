/*
 * harness.cpp -- differential test for PBSD batch b0184 (hash.tests).
 *
 * driver2.c contributes my_hash(); its main() and the main() routines in
 * tdel.c and thash4.c depend on live db(3) hash tables and were recorded in
 * skipped.txt instead of being ported.
 */

#include <climits>
#include <cstdint>
#include <cstdio>
#include <cstring>

import pbsd.lib.libc.db.test.hash.tests.b0184;

namespace port = pbsd::lib_libc_db_test_hash_tests::b0184;

extern "C" int ref_my_hash(char *key, int len);

namespace {

constexpr unsigned char GUARD = 0x7f;
constexpr std::size_t KEY_PRE = 16;
constexpr std::size_t KEY_USER = 2048;
constexpr std::size_t KEY_POST = 16;
constexpr std::size_t KEY_CAP = KEY_PRE + KEY_USER + KEY_POST;
constexpr long RANDOM_ITERS = 200000;
constexpr int MAX_SHOW = 8;

struct Stat {
	const char *name;
	long cases;
	long fails;
	int shown;
};

Stat st_my_hash = { "my_hash", 0, 0, 0 };

std::uint64_t rng_state = 0xb0184facefeedULL;

std::uint64_t
rnd_u64(void)
{
	std::uint64_t z;

	rng_state += 0x9e3779b97f4a7c15ULL;
	z = rng_state;
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return z ^ (z >> 31);
}

unsigned
rnd_u32(void)
{
	return (unsigned)(rnd_u64() & 0xffffffffu);
}

bool
fail(Stat &st, const char *label, const char *detail)
{
	st.fails++;
	if (st.shown < MAX_SHOW) {
		st.shown++;
		std::printf("  FAIL %-8s %-24s %s\n", st.name, label, detail);
	}
	return false;
}

struct GuardedKey {
	unsigned char bytes[KEY_CAP];

	void fill_guard(void)
	{
		std::memset(bytes, GUARD, KEY_CAP);
	}

	char *user(void)
	{
		return reinterpret_cast<char *>(bytes + KEY_PRE);
	}

	void set_bytes(const unsigned char *src, std::size_t n)
	{
		fill_guard();
		if (n > KEY_USER)
			n = KEY_USER;
		if (n > 0 && src != nullptr)
			std::memcpy(user(), src, n);
	}

	bool guards_intact(void) const
	{
		for (std::size_t i = 0; i < KEY_PRE; i++) {
			if (bytes[i] != GUARD)
				return false;
		}
		for (std::size_t i = KEY_PRE + KEY_USER; i < KEY_CAP; i++) {
			if (bytes[i] != GUARD)
				return false;
		}
		return true;
	}
};

bool
run_case(const char *label, char *key, int len)
{
	st_my_hash.cases++;

	int ret_r = ref_my_hash(key, len);
	int ret_p = port::my_hash(key, len);

	if (ret_r != ret_p) {
		std::printf("    %s: ret %d vs %d\n", label, ret_r, ret_p);
		return fail(st_my_hash, label, "return mismatch");
	}
	return true;
}

bool
run_guarded_case(const char *label, GuardedKey &gk, int len)
{
	bool ok = run_case(label, gk.user(), len);
	if (!gk.guards_intact())
		fail(st_my_hash, label, "key guard corrupted");
	return ok;
}

void
test_my_hash_edges(void)
{
	GuardedKey gk;
	static const unsigned char empty[] = { '\0' };
	static const unsigned char one[] = { 'a' };
	static const unsigned char nulheavy[] = { '\0', '\0', '\0', '\0', 'x' };
	static const unsigned char hibit[] = { 0x80, 0xff, 0xfe, 0x7f, '\0' };
	static const unsigned char boundary[256] = {
		0x80, 0x81, 0xfe, 0xff, 0x7f, 0x00, 0x01, 0x7e
	};

	gk.set_bytes(empty, 1);
	run_guarded_case("empty", gk, 0);
	run_guarded_case("empty_len1", gk, 1);

	gk.set_bytes(one, 1);
	run_guarded_case("single_char", gk, 1);
	run_guarded_case("single_char_len0", gk, 0);

	gk.set_bytes(nulheavy, sizeof(nulheavy));
	run_guarded_case("nul_heavy", gk, (int)sizeof(nulheavy));
	run_guarded_case("nul_heavy_len0", gk, 0);

	gk.set_bytes(hibit, sizeof(hibit));
	run_guarded_case("high_bit", gk, (int)sizeof(hibit));
	run_guarded_case("high_bit_len1", gk, 1);

	gk.set_bytes(boundary, sizeof(boundary));
	run_guarded_case("boundary_256", gk, (int)sizeof(boundary));
	run_guarded_case("boundary_len127", gk, 127);
	run_guarded_case("boundary_len128", gk, 128);
	run_guarded_case("boundary_len129", gk, 129);
	run_guarded_case("boundary_len1023", gk, 1023);
	run_guarded_case("boundary_len1024", gk, 1024);
	run_guarded_case("boundary_len1025", gk, 1025);
	run_guarded_case("boundary_len2047", gk, 2047);
	run_guarded_case("boundary_len2048", gk, 2048);

	run_case("null_key_len0", nullptr, 0);
	run_case("null_key_len1", nullptr, 1);
	run_case("null_key_neg1", nullptr, -1);
	run_case("null_key_int_min", nullptr, INT_MIN);
	run_case("null_key_int_max", nullptr, INT_MAX);

	run_guarded_case("len_neg1", gk, -1);
	run_guarded_case("len_int_min", gk, INT_MIN);
	run_guarded_case("len_int_max", gk, INT_MAX);
	run_guarded_case("len_zero", gk, 0);
}

void
test_my_hash_random(void)
{
	GuardedKey gk;

	for (long i = 0; i < RANDOM_ITERS; i++) {
		unsigned pick = rnd_u32() % 16u;
		int len;
		char *key;

		if (pick == 0) {
			key = nullptr;
			len = (int)(rnd_u32() % 7u) - 3;
		} else if (pick == 1) {
			gk.fill_guard();
			key = gk.user();
			len = 0;
		} else {
			gk.fill_guard();
			std::size_t n = rnd_u32() % (KEY_USER + 1u);
			for (std::size_t j = 0; j < n; j++) {
				unsigned v = rnd_u32();
				if ((v & 7u) == 0)
					gk.user()[j] = (char)(0x80 + (v & 0x7fu));
				else if ((v & 7u) == 1)
					gk.user()[j] = '\0';
				else
					gk.user()[j] = (char)(v & 0xffu);
			}
			key = gk.user();
			switch (rnd_u32() % 8u) {
			case 0:
				len = 0;
				break;
			case 1:
				len = 1;
				break;
			case 2:
				len = (int)n;
				break;
			case 3:
				len = (int)(n > 0 ? n - 1 : 0);
				break;
			case 4:
				len = (int)(n + 1);
				break;
			case 5:
				len = -1;
				break;
			case 6:
				len = INT_MAX;
				break;
			default:
				len = INT_MIN;
				break;
			}
		}

		char label[48];
		std::snprintf(label, sizeof(label), "rnd%ld", i);
		if (key == nullptr)
			run_case(label, key, len);
		else
			run_guarded_case(label, gk, len);
	}
}

void
report(void)
{
	std::printf("\n%-32s %12s %12s %10s\n", "function", "cases",
	    "failures", "result");
	std::printf("%-32s %12s %12s %10s\n",
	    "--------------------------------", "------------",
	    "------------", "----------");
	std::printf("%-32s %12ld %12ld %10s\n", st_my_hash.name,
	    st_my_hash.cases, st_my_hash.fails,
	    st_my_hash.fails == 0 ? "PASS" : "FAIL");
	std::printf("%-32s %12s %12s %10s\n",
	    "--------------------------------", "------------",
	    "------------", "----------");
	std::printf("%-32s %12ld %12ld %10s\n", "TOTAL", st_my_hash.cases,
	    st_my_hash.fails, st_my_hash.fails == 0 ? "PASS" : "FAIL");
	std::printf("\n%ld case(s) run, %ld failure(s).\n", st_my_hash.cases,
	    st_my_hash.fails);
	std::printf("Three main() routines skipped; see skipped.txt.\n");
}

} /* namespace */

int
main(void)
{
	std::printf("PBSD batch b0184 differential test\n");
	std::printf("(oracle.c is the specification)\n\n");

	test_my_hash_edges();
	test_my_hash_random();
	report();

	return st_my_hash.fails == 0 ? 0 : 1;
}
