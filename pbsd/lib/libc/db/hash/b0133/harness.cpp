// b0133 differential test harness.
//
// Compares hash4 in the C++23 port against ref_hash4 in oracle.c.

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <sys/types.h>

import pbsd.lib.libc.db.hash.b0133;

namespace P = pbsd::lib_libc_db_hash::b0133;

extern "C" u_int32_t ref_hash4(const void *key, size_t len);

struct Stat {
	const char *name;
	unsigned long long cases;
	unsigned long long fails;
	int shown;
};

static Stat st_hash4 = { "hash4", 0, 0, 0 };

static const int MAX_SHOW = 8;

static bool
begin_fail(Stat &st, const char *what)
{
	st.fails++;
	if (st.shown >= MAX_SHOW)
		return false;
	st.shown++;
	std::printf("FAIL %s: %s\n", st.name, what);
	return true;
}

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

	std::uint32_t below(std::uint32_t n) { return (std::uint32_t)(next() % n); }
};

static const unsigned char GUARD = 0x7f;
static const size_t KEY_CAP = 512;

static void
fill_key(unsigned char *buf, size_t cap, const unsigned char *body, size_t len)
{
	std::memset(buf, GUARD, cap);
	for (size_t i = 0; i < len; i++)
		buf[i] = body[i];
}

static void
t_hash4(const unsigned char *body, size_t len, const char *tag)
{
	unsigned char ka[KEY_CAP];
	unsigned char kb[KEY_CAP];

	fill_key(ka, KEY_CAP, body, len);
	fill_key(kb, KEY_CAP, body, len);

	st_hash4.cases++;

	u_int32_t got = P::hash4(ka, len);
	u_int32_t ref = ref_hash4(kb, len);

	if (got != ref) {
		char msg[256];
		std::snprintf(msg, sizeof msg,
		    "%s len=%zu got=0x%08x ref=0x%08x", tag, len,
		    (unsigned)got, (unsigned)ref);
		begin_fail(st_hash4, msg);
	}
}

static void
edge_hash4()
{
	static const unsigned char empty[] = { 0 };
	static const unsigned char one_nul[] = { 0x00 };
	static const unsigned char one_ff[] = { 0xff };
	static const unsigned char one_80[] = { 0x80 };
	static const unsigned char one_01[] = { 0x01 };
	static const unsigned char two[] = { 'a', 'b' };
	static const unsigned char nul_heavy[] = {
		0x00, 0x00, 0x00, 0x41, 0x00, 0x42, 0x00
	};
	static const unsigned char highbit_run[] = {
		0x80, 0x81, 0xfe, 0xff, 0x7f, 0x00, 0x80, 0xff
	};
	static const unsigned char eight_zeros[8] = { 0 };
	static const unsigned char eight_ff[8] = {
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
	};
	static const unsigned char nine_mixed[] = {
		0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09
	};
	static const unsigned char fifteen[15] = {};
	static const unsigned char sixteen[16] = {};
	static const unsigned char seventeen[17] = {};
	unsigned char ramp[32];
	unsigned char alt[24];

	for (size_t i = 0; i < sizeof ramp; i++)
		ramp[i] = (unsigned char)i;
	for (size_t i = 0; i < sizeof alt; i++)
		alt[i] = (i & 1) ? 0xff : 0x00;

	/* len == 0: skip body, result must be 0 */
	t_hash4(empty, 0, "empty");

	/* each switch remainder 1..7 */
	t_hash4(one_nul, 1, "one_nul");
	t_hash4(one_ff, 1, "one_ff");
	t_hash4(one_80, 1, "one_80");
	t_hash4(one_01, 1, "one_01");
	t_hash4(two, 2, "two");
	t_hash4(nul_heavy, 3, "nul_heavy3");
	t_hash4(nul_heavy, 4, "nul_heavy4");
	t_hash4(nul_heavy, 5, "nul_heavy5");
	t_hash4(highbit_run, 6, "highbit6");
	t_hash4(highbit_run, 7, "highbit7");

	/* remainder 0: exact multiples of 8 */
	t_hash4(eight_zeros, 8, "eight_zeros");
	t_hash4(eight_ff, 8, "eight_ff");
	t_hash4(sixteen, 16, "sixteen");
	t_hash4(sixteen, 24, "twentyfour");

	/* boundary around loop count */
	t_hash4(nine_mixed, 9, "nine");
	t_hash4(fifteen, 15, "fifteen");
	t_hash4(sixteen, 16, "sixteen_exact");
	t_hash4(seventeen, 17, "seventeen");

	/* longer patterns */
	t_hash4(ramp, sizeof ramp, "ramp32");
	t_hash4(alt, sizeof alt, "alt24");
	t_hash4(highbit_run, sizeof highbit_run, "highbit8");

	/* single high-bit bytes at each offset within an 8-byte block */
	for (int pos = 0; pos < 8; pos++) {
		unsigned char block[8];
		std::memset(block, 0x55, sizeof block);
		block[pos] = 0xff;
		t_hash4(block, 8, "highbit_pos");
	}

	/* lengths 1..64 stepping every remainder class */
	for (size_t len = 1; len <= 64; len++) {
		unsigned char buf[64];
		for (size_t i = 0; i < len; i++)
			buf[i] = (unsigned char)(0xa5 ^ (unsigned char)i);
		t_hash4(buf, len, "step_len");
	}
}

static void
random_hash4(Rng &rng, unsigned long long n)
{
	unsigned char buf[KEY_CAP];

	for (unsigned long long i = 0; i < n; i++) {
		size_t len = rng.below((std::uint32_t)KEY_CAP + 1);
		for (size_t j = 0; j < len; j++)
			buf[j] = (unsigned char)(rng.next() & 0xff);
		t_hash4(buf, len, "random");
	}
}

static void
print_table()
{
	Stat all[] = { st_hash4 };
	const char *hdr = "function     cases      failures";
	const char *sep = "--------------------------------";

	std::printf("\n%s\n%s\n", hdr, sep);
	for (size_t i = 0; i < sizeof all / sizeof all[0]; i++)
		std::printf("%-12s %-10llu %-10llu\n",
		    all[i].name, all[i].cases, all[i].fails);
	std::printf("%s\n", sep);
}

int
main()
{
	edge_hash4();
	Rng rng(0xB0133C4FEULL);
	random_hash4(rng, 200000);
	print_table();

	return st_hash4.fails ? 1 : 0;
}
