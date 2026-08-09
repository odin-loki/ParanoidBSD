/*
 * harness.cpp -- differential test for PBSD batch b0047.
 */

#include <cstdint>
#include <cstdio>
#include <cstdlib>

import pbsd.lib.libc.softfloat.bits32.b0047;

namespace port = pbsd::lib_libc_softfloat_bits32::b0047;

using u32 = unsigned int;
using u64 = unsigned long long;
using i32 = int;

extern "C" {
	u32 ref_int32_to_float32(i32);
	u64 ref_int32_to_float64(i32);
	i32 ref_float32_to_int32(u32);
	i32 ref_float32_to_int32_round_to_zero(u32);
	u64 ref_float32_to_float64(u32);
	u32 ref_float32_round_to_int(u32);
	u32 ref_float32_add(u32,u32);
	u32 ref_float32_sub(u32,u32);
	u32 ref_float32_mul(u32,u32);
	u32 ref_float32_div(u32,u32);
	u32 ref_float32_rem(u32,u32);
	u32 ref_float32_sqrt(u32);
	int ref_float32_eq(u32,u32);
	int ref_float32_le(u32,u32);
	int ref_float32_lt(u32,u32);
	int ref_float32_eq_signaling(u32,u32);
	int ref_float32_le_quiet(u32,u32);
	int ref_float32_lt_quiet(u32,u32);
	i32 ref_float64_to_int32(u64);
	i32 ref_float64_to_int32_round_to_zero(u64);
	u32 ref_float64_to_float32(u64);
	u64 ref_float64_add(u64,u64);
	u64 ref_float64_sub(u64,u64);
	u64 ref_float64_mul(u64,u64);
	u64 ref_float64_div(u64,u64);
	u64 ref_float64_rem(u64,u64);
	u64 ref_float64_sqrt(u64);
	int ref_float64_eq(u64,u64);
	int ref_float64_le(u64,u64);
	int ref_float64_lt(u64,u64);
	int ref_float64_eq_signaling(u64,u64);
	int ref_float64_le_quiet(u64,u64);
	int ref_float64_lt_quiet(u64,u64);
extern int __oracle_float_rounding_mode;
extern int __oracle_float_exception_flags;
extern int __oracle_float_detect_tininess;
extern int __oracle_float_exception_mask;
}

struct Stat { const char *name; unsigned long long cases, failures; };

static Stat stats[] = {
	{ "int32_to_float32", 0, 0 },
	{ "int32_to_float64", 0, 0 },
	{ "float32_to_int32", 0, 0 },
	{ "float32_to_int32_round_to_zero", 0, 0 },
	{ "float32_to_float64", 0, 0 },
	{ "float32_round_to_int", 0, 0 },
	{ "float32_add", 0, 0 },
	{ "float32_sub", 0, 0 },
	{ "float32_mul", 0, 0 },
	{ "float32_div", 0, 0 },
	{ "float32_rem", 0, 0 },
	{ "float32_sqrt", 0, 0 },
	{ "float32_eq", 0, 0 },
	{ "float32_le", 0, 0 },
	{ "float32_lt", 0, 0 },
	{ "float32_eq_signaling", 0, 0 },
	{ "float32_le_quiet", 0, 0 },
	{ "float32_lt_quiet", 0, 0 },
	{ "float64_to_int32", 0, 0 },
	{ "float64_to_int32_round_to_zero", 0, 0 },
	{ "float64_to_float32", 0, 0 },
	{ "float64_add", 0, 0 },
	{ "float64_sub", 0, 0 },
	{ "float64_mul", 0, 0 },
	{ "float64_div", 0, 0 },
	{ "float64_rem", 0, 0 },
	{ "float64_sqrt", 0, 0 },
	{ "float64_eq", 0, 0 },
	{ "float64_le", 0, 0 },
	{ "float64_lt", 0, 0 },
	{ "float64_eq_signaling", 0, 0 },
	{ "float64_le_quiet", 0, 0 },
	{ "float64_lt_quiet", 0, 0 },
};

enum FnIdx {
	FN_INT32_TO_FLOAT32 = 0,
	FN_INT32_TO_FLOAT64 = 1,
	FN_FLOAT32_TO_INT32 = 2,
	FN_FLOAT32_TO_INT32_ROUND_TO_ZERO = 3,
	FN_FLOAT32_TO_FLOAT64 = 4,
	FN_FLOAT32_ROUND_TO_INT = 5,
	FN_FLOAT32_ADD = 6,
	FN_FLOAT32_SUB = 7,
	FN_FLOAT32_MUL = 8,
	FN_FLOAT32_DIV = 9,
	FN_FLOAT32_REM = 10,
	FN_FLOAT32_SQRT = 11,
	FN_FLOAT32_EQ = 12,
	FN_FLOAT32_LE = 13,
	FN_FLOAT32_LT = 14,
	FN_FLOAT32_EQ_SIGNALING = 15,
	FN_FLOAT32_LE_QUIET = 16,
	FN_FLOAT32_LT_QUIET = 17,
	FN_FLOAT64_TO_INT32 = 18,
	FN_FLOAT64_TO_INT32_ROUND_TO_ZERO = 19,
	FN_FLOAT64_TO_FLOAT32 = 20,
	FN_FLOAT64_ADD = 21,
	FN_FLOAT64_SUB = 22,
	FN_FLOAT64_MUL = 23,
	FN_FLOAT64_DIV = 24,
	FN_FLOAT64_REM = 25,
	FN_FLOAT64_SQRT = 26,
	FN_FLOAT64_EQ = 27,
	FN_FLOAT64_LE = 28,
	FN_FLOAT64_LT = 29,
	FN_FLOAT64_EQ_SIGNALING = 30,
	FN_FLOAT64_LE_QUIET = 31,
	FN_FLOAT64_LT_QUIET = 32,
};

static unsigned long long rng = 0xB0047D00DULL;
static int reported;

static u64 next_u64()
{
	rng ^= rng << 13;
	rng ^= rng >> 7;
	rng ^= rng << 17;
	return rng;
}

static u32 next_u32() { return (u32)next_u64(); }
static i32 next_i32() { return (i32)next_u32(); }

static void reset_oracle(int rnd, int tininess = port::float_tininess_after_rounding)
{
	__oracle_float_rounding_mode = rnd;
	__oracle_float_exception_flags = 0;
	__oracle_float_detect_tininess = tininess;
	__oracle_float_exception_mask = 0;
}

static void reset_port(int rnd, int tininess = port::float_tininess_after_rounding)
{
	port::__port_float_rounding_mode = rnd;
	port::__port_float_exception_flags = 0;
	port::__port_float_detect_tininess = tininess;
	port::__port_float_exception_mask = 0;
}

static void fail(FnIdx fn, const char *what, unsigned long long a = 0,
    unsigned long long b = 0, long long ro = 0, long long rp = 0)
{
	stats[fn].failures++;
	if (reported < 30) {
		reported++;
		std::printf("FAIL %-28s %s a=%llx b=%llx oracle=%lld port=%lld\n",
		    stats[fn].name, what,
		    (unsigned long long)a, (unsigned long long)b, ro, rp);
	}
}

#define CHECK_FLAGS(fn) \
	do { \
		if (__oracle_float_exception_flags != port::__port_float_exception_flags) \
			fail(fn, "flags", __oracle_float_exception_flags, \
			    port::__port_float_exception_flags); \
	} while (0)

static void test_int32_to_float32(i32 a, int rnd)
{
	const FnIdx fn = FN_INT32_TO_FLOAT32;
	stats[fn].cases++;
	reset_oracle(rnd); u32 ro = ref_int32_to_float32(a);
	reset_port(rnd); u32 rp = port::int32_to_float32(a);
	CHECK_FLAGS(fn);
	if (ro != rp) fail(fn, "ret", (u32)a, 0, ro, rp);
}

static void test_int32_to_float64(i32 a, int rnd)
{
	const FnIdx fn = FN_INT32_TO_FLOAT64;
	stats[fn].cases++;
	reset_oracle(rnd); u64 ro = ref_int32_to_float64(a);
	reset_port(rnd); u64 rp = port::int32_to_float64(a);
	CHECK_FLAGS(fn);
	if (ro != rp) fail(fn, "ret", (u32)a, 0, ro, rp);
}

static void test_f32_unary_i32(FnIdx fn, u32 a, int rnd,
    i32 (*ref)(u32), i32 (*pf)(u32))
{
	stats[fn].cases++;
	reset_oracle(rnd); i32 ro = ref(a);
	reset_port(rnd); i32 rp = pf(a);
	CHECK_FLAGS(fn);
	if (ro != rp) fail(fn, "ret", a, 0, ro, rp);
}

static void test_f32_unary_u32(FnIdx fn, u32 a, int rnd,
    u32 (*ref)(u32), u32 (*pf)(u32))
{
	stats[fn].cases++;
	reset_oracle(rnd); u32 ro = ref(a);
	reset_port(rnd); u32 rp = pf(a);
	CHECK_FLAGS(fn);
	if (ro != rp) fail(fn, "ret", a, 0, ro, rp);
}

static void test_f32_unary_u64(FnIdx fn, u32 a, int rnd,
    u64 (*ref)(u32), u64 (*pf)(u32))
{
	stats[fn].cases++;
	reset_oracle(rnd); u64 ro = ref(a);
	reset_port(rnd); u64 rp = pf(a);
	CHECK_FLAGS(fn);
	if (ro != rp) fail(fn, "ret", a, 0, ro, rp);
}

static void test_f32_bin_u32(FnIdx fn, u32 a, u32 b, int rnd,
    u32 (*ref)(u32,u32), u32 (*pf)(u32,u32))
{
	stats[fn].cases++;
	reset_oracle(rnd); u32 ro = ref(a,b);
	reset_port(rnd); u32 rp = pf(a,b);
	CHECK_FLAGS(fn);
	if (ro != rp) fail(fn, "ret", a, b, ro, rp);
}

static void test_f32_cmp(FnIdx fn, u32 a, u32 b, int rnd,
    int (*ref)(u32,u32), int (*pf)(u32,u32))
{
	stats[fn].cases++;
	reset_oracle(rnd); int ro = ref(a,b);
	reset_port(rnd); int rp = pf(a,b);
	CHECK_FLAGS(fn);
	if (ro != rp) fail(fn, "ret", a, b, ro, rp);
}

static void test_f64_unary_i32(FnIdx fn, u64 a, int rnd,
    i32 (*ref)(u64), i32 (*pf)(u64))
{
	stats[fn].cases++;
	reset_oracle(rnd); i32 ro = ref(a);
	reset_port(rnd); i32 rp = pf(a);
	CHECK_FLAGS(fn);
	if (ro != rp) fail(fn, "ret", a, 0, ro, rp);
}

static void test_f64_unary_u32(FnIdx fn, u64 a, int rnd,
    u32 (*ref)(u64), u32 (*pf)(u64))
{
	stats[fn].cases++;
	reset_oracle(rnd); u32 ro = ref(a);
	reset_port(rnd); u32 rp = pf(a);
	CHECK_FLAGS(fn);
	if (ro != rp) fail(fn, "ret", a, 0, ro, rp);
}

static void test_f64_unary_u64(FnIdx fn, u64 a, int rnd,
    u64 (*ref)(u64), u64 (*pf)(u64))
{
	stats[fn].cases++;
	reset_oracle(rnd); u64 ro = ref(a);
	reset_port(rnd); u64 rp = pf(a);
	CHECK_FLAGS(fn);
	if (ro != rp) fail(fn, "ret", a, 0, ro, rp);
}

static void test_f64_bin_u64(FnIdx fn, u64 a, u64 b, int rnd,
    u64 (*ref)(u64,u64), u64 (*pf)(u64,u64))
{
	stats[fn].cases++;
	reset_oracle(rnd); u64 ro = ref(a,b);
	reset_port(rnd); u64 rp = pf(a,b);
	CHECK_FLAGS(fn);
	if (ro != rp) fail(fn, "ret", a, b, ro, rp);
}

static void test_f64_cmp(FnIdx fn, u64 a, u64 b, int rnd,
    int (*ref)(u64,u64), int (*pf)(u64,u64))
{
	stats[fn].cases++;
	reset_oracle(rnd); int ro = ref(a,b);
	reset_port(rnd); int rp = pf(a,b);
	CHECK_FLAGS(fn);
	if (ro != rp) fail(fn, "ret", a, b, ro, rp);
}

static void run_f32_edges()
{
	static const u32 fvals[] = {
		0u, 0x80000000u, 0x3f800000u, 0xbf800000u,
		0x00000001u, 0x007fffffu, 0x00800000u,
		0x7f7fffffu, 0xff7fffffu, 0x7f800000u, 0xff800000u,
		0x7fc00000u, 0xffc00000u, 0x7f800001u, 0x7fa00000u,
		0x4f000000u, 0xcf000000u, 0x4b000000u, 0xffffffffu,
	};
	static const i32 ivals[] = { 0, 1, -1, 2, -2, 0x7fffffff, (i32)0x80000000, 0x00ffffff, -0x00ffffff };
	for (int rnd = 0; rnd < 4; rnd++) {
		for (u32 v : fvals) {
			test_int32_to_float32((i32)v, rnd);
			test_f32_unary_i32(FN_FLOAT32_TO_INT32, v, rnd, ref_float32_to_int32, port::float32_to_int32);
			test_f32_unary_i32(FN_FLOAT32_TO_INT32_ROUND_TO_ZERO, v, rnd,
			    ref_float32_to_int32_round_to_zero, port::float32_to_int32_round_to_zero);
			test_f32_unary_u64(FN_FLOAT32_TO_FLOAT64, v, rnd, ref_float32_to_float64, port::float32_to_float64);
			test_f32_unary_u32(FN_FLOAT32_ROUND_TO_INT, v, rnd, ref_float32_round_to_int, port::float32_round_to_int);
			test_f32_unary_u32(FN_FLOAT32_SQRT, v, rnd, ref_float32_sqrt, port::float32_sqrt);
			for (u32 w : fvals) {
				test_f32_bin_u32(FN_FLOAT32_ADD, v, w, rnd, ref_float32_add, port::float32_add);
				test_f32_bin_u32(FN_FLOAT32_SUB, v, w, rnd, ref_float32_sub, port::float32_sub);
				test_f32_bin_u32(FN_FLOAT32_MUL, v, w, rnd, ref_float32_mul, port::float32_mul);
				test_f32_bin_u32(FN_FLOAT32_DIV, v, w, rnd, ref_float32_div, port::float32_div);
				test_f32_bin_u32(FN_FLOAT32_REM, v, w, rnd, ref_float32_rem, port::float32_rem);
				test_f32_cmp(FN_FLOAT32_EQ, v, w, rnd, ref_float32_eq, port::float32_eq);
				test_f32_cmp(FN_FLOAT32_LE, v, w, rnd, ref_float32_le, port::float32_le);
				test_f32_cmp(FN_FLOAT32_LT, v, w, rnd, ref_float32_lt, port::float32_lt);
				test_f32_cmp(FN_FLOAT32_EQ_SIGNALING, v, w, rnd, ref_float32_eq_signaling, port::float32_eq_signaling);
				test_f32_cmp(FN_FLOAT32_LE_QUIET, v, w, rnd, ref_float32_le_quiet, port::float32_le_quiet);
				test_f32_cmp(FN_FLOAT32_LT_QUIET, v, w, rnd, ref_float32_lt_quiet, port::float32_lt_quiet);
			}
		}
		for (i32 v : ivals) {
			test_int32_to_float32(v, rnd);
			test_int32_to_float64(v, rnd);
		}
	}
}

static void run_f64_edges()
{
	static const u64 fvals[] = {
		0ull, 0x8000000000000000ull, 0x3ff0000000000000ull, 0xbff0000000000000ull,
		0x0000000000000001ull, 0x000fffffffffffffull, 0x0010000000000000ull,
		0x7fefffffffffffffull, 0xffefffffffffffffull,
		0x7ff0000000000000ull, 0xfff0000000000000ull,
		0x7ff8000000000000ull, 0xfff8000000000000ull,
		0x7ff0000000000001ull, 0x7ff4000000000000ull,
		0x41dfffffffc00000ull, 0xc1e0000000000000ull,
		0x41e0000000000000ull, 0xffffffffffffffffull,
	};
	for (int rnd = 0; rnd < 4; rnd++) {
		for (u64 v : fvals) {
			test_f64_unary_i32(FN_FLOAT64_TO_INT32, v, rnd, ref_float64_to_int32, port::float64_to_int32);
			test_f64_unary_i32(FN_FLOAT64_TO_INT32_ROUND_TO_ZERO, v, rnd,
			    ref_float64_to_int32_round_to_zero, port::float64_to_int32_round_to_zero);
			test_f64_unary_u32(FN_FLOAT64_TO_FLOAT32, v, rnd, ref_float64_to_float32, port::float64_to_float32);
			test_f64_unary_u64(FN_FLOAT64_SQRT, v, rnd, ref_float64_sqrt, port::float64_sqrt);
			for (u64 w : fvals) {
				test_f64_bin_u64(FN_FLOAT64_ADD, v, w, rnd, ref_float64_add, port::float64_add);
				test_f64_bin_u64(FN_FLOAT64_SUB, v, w, rnd, ref_float64_sub, port::float64_sub);
				test_f64_bin_u64(FN_FLOAT64_MUL, v, w, rnd, ref_float64_mul, port::float64_mul);
				test_f64_bin_u64(FN_FLOAT64_DIV, v, w, rnd, ref_float64_div, port::float64_div);
				test_f64_bin_u64(FN_FLOAT64_REM, v, w, rnd, ref_float64_rem, port::float64_rem);
				test_f64_cmp(FN_FLOAT64_EQ, v, w, rnd, ref_float64_eq, port::float64_eq);
				test_f64_cmp(FN_FLOAT64_LE, v, w, rnd, ref_float64_le, port::float64_le);
				test_f64_cmp(FN_FLOAT64_LT, v, w, rnd, ref_float64_lt, port::float64_lt);
				test_f64_cmp(FN_FLOAT64_EQ_SIGNALING, v, w, rnd, ref_float64_eq_signaling, port::float64_eq_signaling);
				test_f64_cmp(FN_FLOAT64_LE_QUIET, v, w, rnd, ref_float64_le_quiet, port::float64_le_quiet);
				test_f64_cmp(FN_FLOAT64_LT_QUIET, v, w, rnd, ref_float64_lt_quiet, port::float64_lt_quiet);
			}
		}
	}
}

static void random_sweep(unsigned long n)
{
	for (unsigned long i = 0; i < n; i++) {
		int rnd = (int)(next_u64() % 4);
		int tiny = (int)(next_u64() & 1);
		u32 a = next_u32();
		u32 b = next_u32();
		i32 ia = next_i32();
		u64 da = ((u64)next_u32() << 32) | next_u32();
		u64 db = ((u64)next_u32() << 32) | next_u32();
		if ((i & 0xff) == 0) { a = 0; b = 0x7f800000u; }
		if ((i & 0x1ff) == 0x80) { da = 0; db = 0x7ff0000000000000ull; }
		reset_oracle(rnd, tiny); reset_port(rnd, tiny);
		test_int32_to_float32(ia, rnd);
		test_int32_to_float64(ia, rnd);
		test_f32_unary_i32(FN_FLOAT32_TO_INT32, a, rnd, ref_float32_to_int32, port::float32_to_int32);
		test_f32_unary_i32(FN_FLOAT32_TO_INT32_ROUND_TO_ZERO, a, rnd,
		    ref_float32_to_int32_round_to_zero, port::float32_to_int32_round_to_zero);
		test_f32_unary_u64(FN_FLOAT32_TO_FLOAT64, a, rnd, ref_float32_to_float64, port::float32_to_float64);
		test_f32_unary_u32(FN_FLOAT32_ROUND_TO_INT, a, rnd, ref_float32_round_to_int, port::float32_round_to_int);
		test_f32_unary_u32(FN_FLOAT32_SQRT, a, rnd, ref_float32_sqrt, port::float32_sqrt);
		test_f32_bin_u32(FN_FLOAT32_ADD, a, b, rnd, ref_float32_add, port::float32_add);
		test_f32_bin_u32(FN_FLOAT32_SUB, a, b, rnd, ref_float32_sub, port::float32_sub);
		test_f32_bin_u32(FN_FLOAT32_MUL, a, b, rnd, ref_float32_mul, port::float32_mul);
		test_f32_bin_u32(FN_FLOAT32_DIV, a, b, rnd, ref_float32_div, port::float32_div);
		test_f32_bin_u32(FN_FLOAT32_REM, a, b, rnd, ref_float32_rem, port::float32_rem);
		test_f32_cmp(FN_FLOAT32_EQ, a, b, rnd, ref_float32_eq, port::float32_eq);
		test_f32_cmp(FN_FLOAT32_LE, a, b, rnd, ref_float32_le, port::float32_le);
		test_f32_cmp(FN_FLOAT32_LT, a, b, rnd, ref_float32_lt, port::float32_lt);
		test_f32_cmp(FN_FLOAT32_EQ_SIGNALING, a, b, rnd, ref_float32_eq_signaling, port::float32_eq_signaling);
		test_f32_cmp(FN_FLOAT32_LE_QUIET, a, b, rnd, ref_float32_le_quiet, port::float32_le_quiet);
		test_f32_cmp(FN_FLOAT32_LT_QUIET, a, b, rnd, ref_float32_lt_quiet, port::float32_lt_quiet);
		test_f64_unary_i32(FN_FLOAT64_TO_INT32, da, rnd, ref_float64_to_int32, port::float64_to_int32);
		test_f64_unary_i32(FN_FLOAT64_TO_INT32_ROUND_TO_ZERO, da, rnd,
		    ref_float64_to_int32_round_to_zero, port::float64_to_int32_round_to_zero);
		test_f64_unary_u32(FN_FLOAT64_TO_FLOAT32, da, rnd, ref_float64_to_float32, port::float64_to_float32);
		test_f64_unary_u64(FN_FLOAT64_SQRT, da, rnd, ref_float64_sqrt, port::float64_sqrt);
		test_f64_bin_u64(FN_FLOAT64_ADD, da, db, rnd, ref_float64_add, port::float64_add);
		test_f64_bin_u64(FN_FLOAT64_SUB, da, db, rnd, ref_float64_sub, port::float64_sub);
		test_f64_bin_u64(FN_FLOAT64_MUL, da, db, rnd, ref_float64_mul, port::float64_mul);
		test_f64_bin_u64(FN_FLOAT64_DIV, da, db, rnd, ref_float64_div, port::float64_div);
		test_f64_bin_u64(FN_FLOAT64_REM, da, db, rnd, ref_float64_rem, port::float64_rem);
		test_f64_cmp(FN_FLOAT64_EQ, da, db, rnd, ref_float64_eq, port::float64_eq);
		test_f64_cmp(FN_FLOAT64_LE, da, db, rnd, ref_float64_le, port::float64_le);
		test_f64_cmp(FN_FLOAT64_LT, da, db, rnd, ref_float64_lt, port::float64_lt);
		test_f64_cmp(FN_FLOAT64_EQ_SIGNALING, da, db, rnd, ref_float64_eq_signaling, port::float64_eq_signaling);
		test_f64_cmp(FN_FLOAT64_LE_QUIET, da, db, rnd, ref_float64_le_quiet, port::float64_le_quiet);
		test_f64_cmp(FN_FLOAT64_LT_QUIET, da, db, rnd, ref_float64_lt_quiet, port::float64_lt_quiet);
	}
}

int main()
{
	run_f32_edges();
	run_f64_edges();
	random_sweep(200000);

	unsigned long long total_fail = 0;
	std::printf("\n%-32s %12s %12s\n", "function", "cases", "failures");
	std::printf("%-32s %12s %12s\n", "--------", "-----", "--------");
	for (auto &s : stats) {
		std::printf("%-32s %12llu %12llu\n", s.name, s.cases, s.failures);
		total_fail += s.failures;
	}
	std::printf("\nTotal failures: %llu\n", total_fail);
	return total_fail ? 1 : 0;
}
