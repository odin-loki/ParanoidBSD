/*
 * Differential test harness for batch b0004 -- lib/libc_nonshared/__stub.c
 *
 * The source defines a single BSS global, __stub_N8TwezWFyocUB.  Every case
 * reads or writes that symbol in both the C++23 port and the C oracle, then
 * compares the observable int value.  Pointer stores go through the address of
 * each symbol so a mistaken write is visible immediately.
 */

#include <climits>
#include <cstdint>
#include <cstdio>
#include <cstring>

import pbsd.lib.libc.nonshared.b0004;

namespace P = pbsd::lib_libc_nonshared::b0004;

extern "C" int __stub_N8TwezWFyocUB;

static const char *const FN_NAME = "__stub_N8TwezWFyocUB";

static unsigned long long cases;
static unsigned long long fails;
static unsigned reported;

static void
fail_case(const char *what, int port_val, int oracle_val)
{
	fails++;
	if (reported++ >= 5)
		return;
	std::printf("  FAIL %s: %s (port=%d oracle=%d)\n", FN_NAME, what,
	    port_val, oracle_val);
}

static void
check_read(const char *what)
{
	cases++;
	int port_val = P::__stub_N8TwezWFyocUB;
	int oracle_val = __stub_N8TwezWFyocUB;
	if (port_val != oracle_val)
		fail_case(what, port_val, oracle_val);
}

static void
check_write_both(int value, const char *what)
{
	cases++;
	P::__stub_N8TwezWFyocUB = value;
	__stub_N8TwezWFyocUB = value;

	int port_val = P::__stub_N8TwezWFyocUB;
	int oracle_val = __stub_N8TwezWFyocUB;
	if (port_val != oracle_val || port_val != value)
		fail_case(what, port_val, oracle_val);
}

static void
check_ptr_write_both(int value, const char *what)
{
	cases++;
	*(&P::__stub_N8TwezWFyocUB) = value;
	*(&__stub_N8TwezWFyocUB) = value;

	int port_val = P::__stub_N8TwezWFyocUB;
	int oracle_val = __stub_N8TwezWFyocUB;
	if (port_val != oracle_val || port_val != value)
		fail_case(what, port_val, oracle_val);
}

static void
check_char_assign(unsigned char byte, const char *what)
{
	cases++;
	char c = (char)byte;
	P::__stub_N8TwezWFyocUB = c;
	__stub_N8TwezWFyocUB = c;

	int port_val = P::__stub_N8TwezWFyocUB;
	int oracle_val = __stub_N8TwezWFyocUB;
	if (port_val != oracle_val)
		fail_case(what, port_val, oracle_val);
}

static std::uint64_t rng_state = 0xb0004deadbeefULL;

static std::uint64_t
next_u64(void)
{
	std::uint64_t x = rng_state;

	x ^= x >> 12;
	x ^= x << 25;
	x ^= x >> 27;
	rng_state = x;
	return x * 0x2545F4914F6CDD1DULL;
}

static int
next_int(void)
{
	std::uint64_t r = next_u64();
	return (int)(r ^ (r >> 32));
}

int
main(void)
{
	/* ---- hand-written edge cases ---- */

	check_read("bss_initial_read");

	check_write_both(0, "store_zero");
	check_write_both(1, "store_one");
	check_write_both(-1, "store_minus_one");
	check_write_both(42, "store_comment_42");
	check_write_both(INT_MAX, "store_int_max");
	check_write_both(INT_MIN, "store_int_min");
	check_write_both(INT_MAX - 1, "store_int_max_minus_one");
	check_write_both(INT_MIN + 1, "store_int_min_plus_one");

	check_ptr_write_both(0, "ptr_store_zero");
	check_ptr_write_both(1, "ptr_store_one");
	check_ptr_write_both(-1, "ptr_store_minus_one");
	check_ptr_write_both(INT_MAX, "ptr_store_int_max");
	check_ptr_write_both(INT_MIN, "ptr_store_int_min");

	/* high-bit single-byte loads assigned to int (sign extension) */
	check_char_assign(0x00, "char_0x00");
	check_char_assign(0x01, "char_0x01");
	check_char_assign(0x7f, "char_0x7f");
	check_char_assign(0x80, "char_0x80");
	check_char_assign(0xfe, "char_0xfe");
	check_char_assign(0xff, "char_0xff");

	for (unsigned b = 0x80; b <= 0xff; b++) {
		char label[32];
		std::snprintf(label, sizeof(label), "char_0x%02x", b);
		check_char_assign((unsigned char)b, label);
	}

	/* boundary around zero after prior writes */
	check_write_both(0, "reset_zero");
	check_read("read_after_reset");

	/* ---- fixed-seed randomised sweep ---- */

	static const int extremes[] = {
		0, 1, -1, 42, 0x7f, 0x80, 0xff, INT_MAX, INT_MIN,
		INT_MAX - 1, INT_MIN + 1, 0x7fffffff, (int)0x80000000U,
		(int)0xffffffffU
	};
	const unsigned long ITERS = 200000UL;

	for (unsigned long it = 0; it < ITERS; it++) {
		int value;
		std::uint64_t r = next_u64();

		if ((r & 7U) == 0U)
			value = extremes[(unsigned)(r >> 3) % 14U];
		else if ((r & 7U) == 1U) {
			unsigned char byte = (unsigned char)(r >> 8);
			if ((byte & 3U) == 0U)
				byte = (unsigned char)(0x80U + (byte & 0x7fU));
			value = (int)(char)byte;
		} else
			value = next_int();

		if ((r & 15U) == 0U)
			check_ptr_write_both(value, "random_ptr_store");
		else
			check_write_both(value, "random_store");

		if ((it & 1023U) == 0U) {
			check_write_both(0, "random_periodic_zero");
			check_read("random_periodic_read");
		}
	}

	/* restore BSS-like zero for cleanliness */
	P::__stub_N8TwezWFyocUB = 0;
	__stub_N8TwezWFyocUB = 0;
	check_read("final_bss_read");

	/* ---- report ---- */

	std::printf("\n");
	std::printf("  %-28s %14s %14s  %s\n", "function", "cases",
	    "failures", "result");
	std::printf("  ------------------------------------------------------------\n");
	std::printf("  %-28s %14llu %14llu  %s\n", FN_NAME, cases, fails,
	    fails == 0 ? "PASS" : "FAIL");
	std::printf("  ------------------------------------------------------------\n");
	std::printf("  %-28s %14llu %14llu  %s\n", "TOTAL", cases, fails,
	    fails == 0 ? "PASS" : "FAIL");
	std::printf("\n");

	return (fails == 0) ? 0 : 1;
}
