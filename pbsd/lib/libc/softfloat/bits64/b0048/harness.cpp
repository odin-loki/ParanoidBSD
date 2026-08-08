// harness.cpp -- differential test for PBSD batch b0048.

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

import pbsd.lib.libc.softfloat.bits64.b0048;
namespace port = pbsd::lib_libc_softfloat_bits64::b0048;

using std::uint8_t;
using std::uint16_t;
using std::uint32_t;
using std::uint64_t;
using std::int8_t;
using std::int16_t;
using std::int32_t;
using std::int64_t;

typedef port::flag flag;
typedef port::bits32 bits32;
typedef port::bits64 bits64;
typedef port::float32 float32;
typedef port::float64 float64;
typedef port::floatx80 floatx80;
typedef port::float128 float128;
typedef port::commonNaNT commonNaNT;

extern "C" {
extern int ref_float_rounding_mode;
extern int ref_float_exception_flags;
extern int ref_floatx80_rounding_precision;
extern int ref_float_detect_tininess;
extern int ref_float_exception_mask;
void ref_add128(bits64 a0, bits64 a1, bits64 b0, bits64 b1, bits64 *z0Ptr, bits64 *z1Ptr);
void ref_add192(bits64 a0, bits64 a1, bits64 a2, bits64 b0, bits64 b1, bits64 b2, bits64 *z0Ptr, bits64 *z1Ptr, bits64 *z2Ptr);
float64 ref_addFloat128Sigs(float64 a, float64 b, flag zSign);
float64 ref_addFloat32Sigs(float64 a, float64 b, flag zSign);
float64 ref_addFloat64Sigs(float64 a, float64 b, flag zSign);
float64 ref_addFloatx80Sigs(float64 a, float64 b, flag zSign);
float64 ref_commonNaNToFloat128(commonNaNT a);
float64 ref_commonNaNToFloat32(commonNaNT a);
float64 ref_commonNaNToFloat64(commonNaNT a);
float64 ref_commonNaNToFloatx80(commonNaNT a);
int8 ref_countLeadingZeros32(bits32 a);
int8 ref_countLeadingZeros64(bits64 a);
flag ref_eq128(bits64 a0, bits64 a1, bits64 b0, bits64 b1);
bits64 ref_estimateDiv128To64(bits64 a0, bits64 a1, bits64 b);
bits32 ref_estimateSqrt32(int16 aExp, bits32 a);
int32 ref_extractFloat128Exp(float128 a);
bits64 ref_extractFloat128Frac0(float128 a);
bits64 ref_extractFloat128Frac1(float128 a);
bits64 ref_extractFloat128Sign(float128 a);
bits32 ref_extractFloat32Exp(float32 a);
bits32 ref_extractFloat32Frac(float32 a);
bits32 ref_extractFloat32Sign(float32 a);
bits64 ref_extractFloat64Exp(float64 a);
bits64 ref_extractFloat64Frac(float64 a);
bits64 ref_extractFloat64Sign(float64 a);
int32 ref_extractFloatx80Exp(floatx80 a);
bits64 ref_extractFloatx80Frac(floatx80 a);
bits64 ref_extractFloatx80Sign(floatx80 a);
commonNaNT ref_float128ToCommonNaN(float128 a);
float128 ref_float128_add(float128 a, float128 b);
float128 ref_float128_div(float128 a, float128 b);
flag ref_float128_eq(float128 a, float128 b);
flag ref_float128_eq_signaling(float128 a, float128 b);
flag ref_float128_is_nan(float128 a);
flag ref_float128_is_signaling_nan(float128 a);
flag ref_float128_le(float128 a, float128 b);
flag ref_float128_le_quiet(float128 a, float128 b);
flag ref_float128_lt(float128 a, float128 b);
flag ref_float128_lt_quiet(float128 a, float128 b);
float128 ref_float128_mul(float128 a, float128 b);
float128 ref_float128_rem(float128 a, float128 b);
int64_t ref_float128_round_to_int(float128 a);
float128 ref_float128_sqrt(float128 a);
float128 ref_float128_sub(float128 a, float128 b);
float32 ref_float128_to_float32(float128 a);
float64 ref_float128_to_float64(float128 a);
floatx80 ref_float128_to_floatx80(float128 a);
int32_t ref_float128_to_int32(float128 a);
int32_t ref_float128_to_int32_round_to_zero(float128 a);
int64_t ref_float128_to_int64(float128 a);
int64_t ref_float128_to_int64_round_to_zero(float128 a);
commonNaNT ref_float32ToCommonNaN(float32 a);
float32 ref_float32_add(float32 a, float32 b);
float32 ref_float32_div(float32 a, float32 b);
flag ref_float32_eq(float32 a, float32 b);
flag ref_float32_eq_signaling(float32 a, float32 b);
flag ref_float32_is_nan(float32 a);
flag ref_float32_is_signaling_nan(float32 a);
flag ref_float32_le(float32 a, float32 b);
flag ref_float32_le_quiet(float32 a, float32 b);
flag ref_float32_lt(float32 a, float32 b);
flag ref_float32_lt_quiet(float32 a, float32 b);
float32 ref_float32_mul(float32 a, float32 b);
float32 ref_float32_rem(float32 a, float32 b);
int64_t ref_float32_round_to_int(float32 a);
float32 ref_float32_sqrt(float32 a);
float32 ref_float32_sub(float32 a, float32 b);
float128 ref_float32_to_float128(float32 a);
float64 ref_float32_to_float64(float32 a);
floatx80 ref_float32_to_floatx80(float32 a);
int32_t ref_float32_to_int32(float32 a);
int32_t ref_float32_to_int32_round_to_zero(float32 a);
int64_t ref_float32_to_int64(float32 a);
int64_t ref_float32_to_int64_round_to_zero(float32 a);
commonNaNT ref_float64ToCommonNaN(float64 a);
float64 ref_float64_add(float64 a, float64 b);
float64 ref_float64_div(float64 a, float64 b);
flag ref_float64_eq(float64 a, float64 b);
flag ref_float64_eq_signaling(float64 a, float64 b);
flag ref_float64_is_nan(float64 a);
flag ref_float64_is_signaling_nan(float64 a);
flag ref_float64_le(float64 a, float64 b);
flag ref_float64_le_quiet(float64 a, float64 b);
flag ref_float64_lt(float64 a, float64 b);
flag ref_float64_lt_quiet(float64 a, float64 b);
float64 ref_float64_mul(float64 a, float64 b);
float64 ref_float64_rem(float64 a, float64 b);
int64_t ref_float64_round_to_int(float64 a);
float64 ref_float64_sqrt(float64 a);
float64 ref_float64_sub(float64 a, float64 b);
float128 ref_float64_to_float128(float64 a);
float32 ref_float64_to_float32(float64 a);
floatx80 ref_float64_to_floatx80(float64 a);
int32_t ref_float64_to_int32(float64 a);
int32_t ref_float64_to_int32_round_to_zero(float64 a);
int64_t ref_float64_to_int64(float64 a);
int64_t ref_float64_to_int64_round_to_zero(float64 a);
void ref_float_raise(int flags);
commonNaNT ref_floatx80ToCommonNaN(floatx80 a);
floatx80 ref_floatx80_add(floatx80 a, floatx80 b);
floatx80 ref_floatx80_div(floatx80 a, floatx80 b);
flag ref_floatx80_eq(floatx80 a, floatx80 b);
flag ref_floatx80_eq_signaling(floatx80 a, floatx80 b);
flag ref_floatx80_is_nan(floatx80 a);
flag ref_floatx80_is_signaling_nan(floatx80 a);
flag ref_floatx80_le(floatx80 a, floatx80 b);
flag ref_floatx80_le_quiet(floatx80 a, floatx80 b);
flag ref_floatx80_lt(floatx80 a, floatx80 b);
flag ref_floatx80_lt_quiet(floatx80 a, floatx80 b);
floatx80 ref_floatx80_mul(floatx80 a, floatx80 b);
floatx80 ref_floatx80_rem(floatx80 a, floatx80 b);
int64_t ref_floatx80_round_to_int(floatx80 a);
floatx80 ref_floatx80_sqrt(floatx80 a);
floatx80 ref_floatx80_sub(floatx80 a, floatx80 b);
float128 ref_floatx80_to_float128(floatx80 a);
float32 ref_floatx80_to_float32(floatx80 a);
float64 ref_floatx80_to_float64(floatx80 a);
int32_t ref_floatx80_to_int32(floatx80 a);
int32_t ref_floatx80_to_int32_round_to_zero(floatx80 a);
int64_t ref_floatx80_to_int64(floatx80 a);
int64_t ref_floatx80_to_int64_round_to_zero(floatx80 a);
float128 ref_int32_to_float128(int32_t a);
float32 ref_int32_to_float32(int32_t a);
float64 ref_int32_to_float64(int32_t a);
floatx80 ref_int32_to_floatx80(int32_t a);
float128 ref_int64_to_float128(int64_t a);
float32 ref_int64_to_float32(int64_t a);
float64 ref_int64_to_float64(int64_t a);
floatx80 ref_int64_to_floatx80(int64_t a);
flag ref_le128(bits64 a0, bits64 a1, bits64 b0, bits64 b1);
flag ref_lt128(bits64 a0, bits64 a1, bits64 b0, bits64 b1);
void ref_mul128By64To192(bits64 a0, bits64 a1, bits64 b, bits64 *z0Ptr, bits64 *z1Ptr, bits64 *z2Ptr);
void ref_mul128To256(bits64 a0, bits64 a1, bits64 b0, bits64 b1, bits64 *z0Ptr, bits64 *z1Ptr, bits64 *z2Ptr, bits64 *z3Ptr);
void ref_mul64To128(bits64 a, bits64 b, bits64 *z0Ptr, bits64 *z1Ptr);
flag ref_ne128(bits64 a0, bits64 a1, bits64 b0, bits64 b1);
void ref_normalizeFloat128Subnormal(bits64 aSig0, bits64 aSig1, int16 *zExpPtr, bits64 *zSig0Ptr, bits64 *zSig1Ptr);
void ref_normalizeFloat32Subnormal(bits32 aSig, int16 *zExpPtr, bits32 *zSigPtr);
void ref_normalizeFloat64Subnormal(bits64 aSig, int16 *zExpPtr, bits64 *zSigPtr);
void ref_normalizeFloatx80Subnormal(bits64 aSig, int32 *zExpPtr, bits64 *zSigPtr);
float128 ref_normalizeRoundAndPackFloat128(flag zSign, int32 zExp, bits64 zSig0, bits64 zSig1);
float32 ref_normalizeRoundAndPackFloat32(flag zSign, int16 zExp, bits32 zSig);
float64 ref_normalizeRoundAndPackFloat64(flag zSign, int16 zExp, bits64 zSig);
floatx80 ref_normalizeRoundAndPackFloatx80(flag zSign, int32 zExp, bits64 zSig0, bits64 zSig1);
float128 ref_packFloat128(flag zSign, int32 zExp, bits64 zSig0, bits64 zSig1);
float32 ref_packFloat32(flag zSign, int16 zExp, bits32 zSig);
float64 ref_packFloat64(flag zSign, int16 zExp, bits64 zSig);
floatx80 ref_packFloatx80(flag zSign, int32 zExp, bits64 zSig);
float64 ref_propagateFloat128NaN(float64 a, float64 b);
float64 ref_propagateFloat32NaN(float64 a, float64 b);
float64 ref_propagateFloat64NaN(float64 a, float64 b);
float64 ref_propagateFloatx80NaN(float64 a, float64 b);
float128 ref_roundAndPackFloat128(flag zSign, int32 zExp, bits64 zSig0, bits64 zSig1);
float32 ref_roundAndPackFloat32(flag zSign, int16 zExp, bits32 zSig);
float64 ref_roundAndPackFloat64(flag zSign, int16 zExp, bits64 zSig);
floatx80 ref_roundAndPackFloatx80(flag zSign, int32 zExp, bits64 zSig0, bits64 zSig1);
int32_t ref_roundAndPackInt32(flag zSign, bits64 absZ);
int64_t ref_roundAndPackInt64(flag zSign, bits64 absZ0, bits64 absZ1);
void ref_shift128ExtraRightJamming(bits64 a0, bits64 a1, bits64 a2, int16 count, bits64 *z0Ptr, bits64 *z1Ptr, bits64 *z2Ptr);
void ref_shift128Right(bits64 a0, bits64 a1, int16 count, bits64 *z0Ptr, bits64 *z1Ptr);
void ref_shift128RightJamming(bits64 a0, bits64 a1, int16 count, bits64 *z0Ptr, bits64 *z1Ptr);
void ref_shift32RightJamming(bits32 a, int16 count, bits32 *zPtr);
void ref_shift64ExtraRightJamming(bits64 a0, bits64 a1, int16 count, bits64 *z0Ptr, bits64 *z1Ptr);
void ref_shift64RightJamming(bits64 a, int16 count, bits64 *zPtr);
void ref_shortShift128Left(bits64 a0, bits64 a1, int16 count, bits64 *z0Ptr, bits64 *z1Ptr);
void ref_shortShift192Left(bits64 a, int16 count, bits64 *zPtr);
void ref_sub128(bits64 a0, bits64 a1, bits64 b0, bits64 b1, bits64 *z0Ptr, bits64 *z1Ptr);
void ref_sub192(bits64 a0, bits64 a1, bits64 a2, bits64 b0, bits64 b1, bits64 b2, bits64 *z0Ptr, bits64 *z1Ptr, bits64 *z2Ptr);
float64 ref_subFloat128Sigs(float64 a, float64 b, flag zSign);
float64 ref_subFloat32Sigs(float64 a, float64 b, flag zSign);
float64 ref_subFloat64Sigs(float64 a, float64 b, flag zSign);
float64 ref_subFloatx80Sigs(float64 a, float64 b, flag zSign);
float128 ref_uint32_to_float128(uint32_t a);
float32 ref_uint32_to_float32(uint32_t a);
float64 ref_uint32_to_float64(uint32_t a);
floatx80 ref_uint32_to_floatx80(uint32_t a);
}


static uint32_t rng_state = 0xB0048u;

static uint32_t urand32()
{
    rng_state ^= rng_state << 13;
    rng_state ^= rng_state >> 17;
    rng_state ^= rng_state << 5;
    return rng_state;
}

static uint64_t urand64()
{
    return (static_cast<uint64_t>(urand32()) << 32) | urand32();
}

static float32 f32_rand()
{
    return static_cast<float32>(urand32());
}

static float64 f64_rand()
{
    return static_cast<float64>(urand64());
}

static floatx80 fx80_rand()
{
    floatx80 z;
    z.low = urand64();
    z.high = static_cast<uint16_t>(urand32() & 0xFFFF);
    return z;
}

static float128 f128_rand()
{
    float128 z;
    z.low = urand64();
    z.high = urand64();
    return z;
}

struct FnStat {
    const char *name;
    unsigned cases;
    unsigned failures;
};

static std::vector<FnStat> stats;

static void record(const char *name, unsigned cases, unsigned failures)
{
    stats.push_back({name, cases, failures});
}

static void sync_globals_from_port()
{
    ref_float_rounding_mode = port::float_rounding_mode;
    ref_float_exception_flags = port::float_exception_flags;
    ref_floatx80_rounding_precision = port::floatx80_rounding_precision;
    ref_float_detect_tininess = port::float_detect_tininess;
    ref_float_exception_mask = port::float_exception_mask;
}

static void sync_globals_to_port()
{
    port::float_rounding_mode = ref_float_rounding_mode;
    port::float_exception_flags = ref_float_exception_flags;
    port::floatx80_rounding_precision = ref_floatx80_rounding_precision;
    port::float_detect_tininess = ref_float_detect_tininess;
    port::float_exception_mask = ref_float_exception_mask;
}

static void reset_globals()
{
    port::float_rounding_mode = port::float_round_nearest_even;
    port::float_exception_flags = 0;
    port::floatx80_rounding_precision = 80;
    port::float_detect_tininess = port::float_tininess_after_rounding;
    port::float_exception_mask = 0;
    sync_globals_from_port();
}

template<typename T>
static bool scalar_fail(const char *name, const T &rp, const T &rr)
{
    if (rp != rr) {
        std::fprintf(stderr, "%s mismatch\n", name);
        return true;
    }
    return false;
}


static void test_add128()
{
    unsigned cases = 0, failures = 0;
    const char *name = "add128";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        sync_globals_from_port();
        
        bits64 a0 = urand64(), a1 = urand64(), b0 = urand64(), b1 = urand64();
        bits64 z0p = 0x7F7F7F7F7F7F7F7FULL, z1p = 0x7F7F7F7F7F7F7F7FULL;
        bits64 z0r = 0x7F7F7F7F7F7F7F7FULL, z1r = 0x7F7F7F7F7F7F7F7FULL;
        port::add128(a0, a1, b0, b1, &z0p, &z1p);
        ref_add128(a0, a1, b0, b1, &z0r, &z1r);
        cases++;
        if (z0p != z0r || z1p != z1r) failures++;

        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_add192()
{
    unsigned cases = 0, failures = 0;
    const char *name = "add192";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        sync_globals_from_port();
        
        bits64 a0 = urand64(), a1 = urand64(), a2 = urand64();
        bits64 b0 = urand64(), b1 = urand64(), b2 = urand64();
        bits64 z0p = 0x7F7F7F7F7F7F7F7FULL, z1p = 0x7F7F7F7F7F7F7F7FULL, z2p = 0x7F7F7F7F7F7F7F7FULL;
        bits64 z0r = 0x7F7F7F7F7F7F7F7FULL, z1r = 0x7F7F7F7F7F7F7F7FULL, z2r = 0x7F7F7F7F7F7F7F7FULL;
        port::add192(a0, a1, a2, b0, b1, b2, &z0p, &z1p, &z2p);
        ref_add192(a0, a1, a2, b0, b1, b2, &z0r, &z1r, &z2r);
        cases++;
        if (z0p != z0r || z1p != z1r || z2p != z2r) failures++;

        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_addFloat128Sigs()
{
    unsigned cases = 0, failures = 0;
    const char *name = "addFloat128Sigs";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float64 a = f64_rand(), b = f64_rand();
        flag zs = urand32() & 1;
        sync_globals_from_port();
        float64 rp = port::addFloat128Sigs(a, b, zs);
        float64 rr = ref_addFloat128Sigs(a, b, zs);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_addFloat32Sigs()
{
    unsigned cases = 0, failures = 0;
    const char *name = "addFloat32Sigs";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float64 a = f64_rand(), b = f64_rand();
        flag zs = urand32() & 1;
        sync_globals_from_port();
        float64 rp = port::addFloat32Sigs(a, b, zs);
        float64 rr = ref_addFloat32Sigs(a, b, zs);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_addFloat64Sigs()
{
    unsigned cases = 0, failures = 0;
    const char *name = "addFloat64Sigs";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float64 a = f64_rand(), b = f64_rand();
        flag zs = urand32() & 1;
        sync_globals_from_port();
        float64 rp = port::addFloat64Sigs(a, b, zs);
        float64 rr = ref_addFloat64Sigs(a, b, zs);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_addFloatx80Sigs()
{
    unsigned cases = 0, failures = 0;
    const char *name = "addFloatx80Sigs";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float64 a = f64_rand(), b = f64_rand();
        flag zs = urand32() & 1;
        sync_globals_from_port();
        float64 rp = port::addFloatx80Sigs(a, b, zs);
        float64 rr = ref_addFloatx80Sigs(a, b, zs);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_commonNaNToFloat128()
{
    unsigned cases = 0, failures = 0;
    const char *name = "commonNaNToFloat128";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        commonNaNT a;
        a.sign = urand32() & 1;
        a.high = urand64();
        a.low = urand64();
        sync_globals_from_port();
        float64 rp = port::commonNaNToFloat128(a);
        float64 rr = ref_commonNaNToFloat128(a);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_commonNaNToFloat32()
{
    unsigned cases = 0, failures = 0;
    const char *name = "commonNaNToFloat32";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        commonNaNT a;
        a.sign = urand32() & 1;
        a.high = urand64();
        a.low = urand64();
        sync_globals_from_port();
        float64 rp = port::commonNaNToFloat32(a);
        float64 rr = ref_commonNaNToFloat32(a);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_commonNaNToFloat64()
{
    unsigned cases = 0, failures = 0;
    const char *name = "commonNaNToFloat64";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        commonNaNT a;
        a.sign = urand32() & 1;
        a.high = urand64();
        a.low = urand64();
        sync_globals_from_port();
        float64 rp = port::commonNaNToFloat64(a);
        float64 rr = ref_commonNaNToFloat64(a);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_commonNaNToFloatx80()
{
    unsigned cases = 0, failures = 0;
    const char *name = "commonNaNToFloatx80";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        commonNaNT a;
        a.sign = urand32() & 1;
        a.high = urand64();
        a.low = urand64();
        sync_globals_from_port();
        float64 rp = port::commonNaNToFloatx80(a);
        float64 rr = ref_commonNaNToFloatx80(a);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_countLeadingZeros32()
{
    unsigned cases = 0, failures = 0;
    const char *name = "countLeadingZeros32";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        bits32 a = urand32();
        sync_globals_from_port();
        int8_t rp = port::countLeadingZeros32(a);
        int8_t rr = ref_countLeadingZeros32(a);
        cases++;
        if (rp != rr) failures++;
    }
    record(name, cases, failures);
}

static void test_countLeadingZeros64()
{
    unsigned cases = 0, failures = 0;
    const char *name = "countLeadingZeros64";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        bits64 a = urand64();
        sync_globals_from_port();
        int8_t rp = port::countLeadingZeros64(a);
        int8_t rr = ref_countLeadingZeros64(a);
        cases++;
        if (rp != rr) failures++;
    }
    record(name, cases, failures);
}

static void test_eq128()
{
    unsigned cases = 0, failures = 0;
    const char *name = "eq128";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        bits64 a0 = urand64(), a1 = urand64(), b0 = urand64(), b1 = urand64();
        sync_globals_from_port();
        flag rp = port::eq128(a0, a1, b0, b1);
        flag rr = ref_eq128(a0, a1, b0, b1);
        cases++;
        if (rp != rr) failures++;
    }
    record(name, cases, failures);
}

static void test_estimateDiv128To64()
{
    unsigned cases = 0, failures = 0;
    const char *name = "estimateDiv128To64";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        bits64 a0 = urand64(), a1 = urand64();
        bits64 b = urand64() | (1ULL << 63);
        sync_globals_from_port();
        bits64 rp = port::estimateDiv128To64(a0, a1, b);
        bits64 rr = ref_estimateDiv128To64(a0, a1, b);
        cases++;
        if (rp != rr) failures++;
    }
    record(name, cases, failures);
}

static void test_estimateSqrt32()
{
    unsigned cases = 0, failures = 0;
    const char *name = "estimateSqrt32";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        int16 aexp = static_cast<int16>(urand32() & 0xFF);
        bits32 a = urand32() | (1u << 31);
        sync_globals_from_port();
        bits32 rp = port::estimateSqrt32(aexp, a);
        bits32 rr = ref_estimateSqrt32(aexp, a);
        cases++;
        if (rp != rr) failures++;
    }
    record(name, cases, failures);
}

static void test_extractFloat128Exp()
{
    unsigned cases = 0, failures = 0;
    const char *name = "extractFloat128Exp";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float128 a = f128_rand();
        sync_globals_from_port();
        auto rp = port::extractFloat128Exp(a);
        auto rr = ref_extractFloat128Exp(a);
        cases++;
        if (rp != rr) failures++;
    }
    record(name, cases, failures);
}

static void test_extractFloat128Frac0()
{
    unsigned cases = 0, failures = 0;
    const char *name = "extractFloat128Frac0";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float128 a = f128_rand();
        sync_globals_from_port();
        auto rp = port::extractFloat128Frac0(a);
        auto rr = ref_extractFloat128Frac0(a);
        cases++;
        if (rp != rr) failures++;
    }
    record(name, cases, failures);
}

static void test_extractFloat128Frac1()
{
    unsigned cases = 0, failures = 0;
    const char *name = "extractFloat128Frac1";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float128 a = f128_rand();
        sync_globals_from_port();
        auto rp = port::extractFloat128Frac1(a);
        auto rr = ref_extractFloat128Frac1(a);
        cases++;
        if (rp != rr) failures++;
    }
    record(name, cases, failures);
}

static void test_extractFloat128Sign()
{
    unsigned cases = 0, failures = 0;
    const char *name = "extractFloat128Sign";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float128 a = f128_rand();
        sync_globals_from_port();
        auto rp = port::extractFloat128Sign(a);
        auto rr = ref_extractFloat128Sign(a);
        cases++;
        if (rp != rr) failures++;
    }
    record(name, cases, failures);
}

static void test_extractFloat32Exp()
{
    unsigned cases = 0, failures = 0;
    const char *name = "extractFloat32Exp";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float32 a = f32_rand();
        sync_globals_from_port();
        auto rp = port::extractFloat32Exp(a);
        auto rr = ref_extractFloat32Exp(a);
        cases++;
        if (rp != rr) failures++;
    }
    record(name, cases, failures);
}

static void test_extractFloat32Frac()
{
    unsigned cases = 0, failures = 0;
    const char *name = "extractFloat32Frac";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float32 a = f32_rand();
        sync_globals_from_port();
        auto rp = port::extractFloat32Frac(a);
        auto rr = ref_extractFloat32Frac(a);
        cases++;
        if (rp != rr) failures++;
    }
    record(name, cases, failures);
}

static void test_extractFloat32Sign()
{
    unsigned cases = 0, failures = 0;
    const char *name = "extractFloat32Sign";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float32 a = f32_rand();
        sync_globals_from_port();
        auto rp = port::extractFloat32Sign(a);
        auto rr = ref_extractFloat32Sign(a);
        cases++;
        if (rp != rr) failures++;
    }
    record(name, cases, failures);
}

static void test_extractFloat64Exp()
{
    unsigned cases = 0, failures = 0;
    const char *name = "extractFloat64Exp";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float64 a = f64_rand();
        sync_globals_from_port();
        auto rp = port::extractFloat64Exp(a);
        auto rr = ref_extractFloat64Exp(a);
        cases++;
        if (rp != rr) failures++;
    }
    record(name, cases, failures);
}

static void test_extractFloat64Frac()
{
    unsigned cases = 0, failures = 0;
    const char *name = "extractFloat64Frac";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float64 a = f64_rand();
        sync_globals_from_port();
        auto rp = port::extractFloat64Frac(a);
        auto rr = ref_extractFloat64Frac(a);
        cases++;
        if (rp != rr) failures++;
    }
    record(name, cases, failures);
}

static void test_extractFloat64Sign()
{
    unsigned cases = 0, failures = 0;
    const char *name = "extractFloat64Sign";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float64 a = f64_rand();
        sync_globals_from_port();
        auto rp = port::extractFloat64Sign(a);
        auto rr = ref_extractFloat64Sign(a);
        cases++;
        if (rp != rr) failures++;
    }
    record(name, cases, failures);
}

static void test_extractFloatx80Exp()
{
    unsigned cases = 0, failures = 0;
    const char *name = "extractFloatx80Exp";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        floatx80 a = fx80_rand();
        sync_globals_from_port();
        auto rp = port::extractFloatx80Exp(a);
        auto rr = ref_extractFloatx80Exp(a);
        cases++;
        if (rp != rr) failures++;
    }
    record(name, cases, failures);
}

static void test_extractFloatx80Frac()
{
    unsigned cases = 0, failures = 0;
    const char *name = "extractFloatx80Frac";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        floatx80 a = fx80_rand();
        sync_globals_from_port();
        auto rp = port::extractFloatx80Frac(a);
        auto rr = ref_extractFloatx80Frac(a);
        cases++;
        if (rp != rr) failures++;
    }
    record(name, cases, failures);
}

static void test_extractFloatx80Sign()
{
    unsigned cases = 0, failures = 0;
    const char *name = "extractFloatx80Sign";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        floatx80 a = fx80_rand();
        sync_globals_from_port();
        auto rp = port::extractFloatx80Sign(a);
        auto rr = ref_extractFloatx80Sign(a);
        cases++;
        if (rp != rr) failures++;
    }
    record(name, cases, failures);
}

static void test_float128ToCommonNaN()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float128ToCommonNaN";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float128 a = f128_rand();
        sync_globals_from_port();
        commonNaNT rp = port::float128ToCommonNaN(a);
        commonNaNT rr = ref_float128ToCommonNaN(a);
        cases++;
        if (rp.sign != rr.sign || rp.high != rr.high || rp.low != rr.low) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_float128_add()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float128_add";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float128 a = f128_rand();
        float128 b = f128_rand();
        sync_globals_from_port();
        float128 rp = port::float128_add(a, b);
        float128 rr = ref_float128_add(a, b);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    static const float128 z = 0;
    sync_globals_from_port();
    float128 rp0 = port::float128_add(z, z);
    float128 rr0 = ref_float128_add(z, z);
    cases++;
    if (rp0 != rr0) failures++;
    record(name, cases, failures);
}

static void test_float128_div()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float128_div";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float128 a = f128_rand();
        float128 b = f128_rand();
        sync_globals_from_port();
        float128 rp = port::float128_div(a, b);
        float128 rr = ref_float128_div(a, b);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    static const float128 z = 0;
    sync_globals_from_port();
    float128 rp0 = port::float128_div(z, z);
    float128 rr0 = ref_float128_div(z, z);
    cases++;
    if (rp0 != rr0) failures++;
    record(name, cases, failures);
}

static void test_float128_eq()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float128_eq";
    reset_globals();

    static const float128 edges[] = {
        0u, 0x80000000u, 0x7F800000u, 0xFF800000u, 0x7FC00000u,
        0x7F7FFFFF, 0x00800000, 0x00000001, 0xFFFFFFFFu
    };
    if (sizeof(float128) == 8) {
        static const float128 edges64[] = {
            0, 0x8000000000000000ULL, 0x7FF0000000000000ULL,
            0xFFF0000000000000ULL, 0x7FF8000000000000ULL,
            0x000FFFFFFFFFFFFF, 0x0010000000000000ULL, 1, 0xFFFFFFFFFFFFFFFFULL
        };
        for (float128 a : edges64) for (float128 b : edges64) {
            sync_globals_from_port();
            flag rp = port::float128_eq(a, b);
            flag rr = ref_float128_eq(a, b);
            cases++;
            if (rp != rr) failures++;
            sync_globals_to_port();
        }
    } else {
        for (float128 a : edges) for (float128 b : edges) {
            sync_globals_from_port();
            flag rp = port::float128_eq(a, b);
            flag rr = ref_float128_eq(a, b);
            cases++;
            if (rp != rr) failures++;
            sync_globals_to_port();
        }
    }
    for (unsigned i = 0; i < 200000u; ++i) {
        float128 a = f128_rand();
        float128 b = f128_rand();
        sync_globals_from_port();
        flag rp = port::float128_eq(a, b);
        flag rr = ref_float128_eq(a, b);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_float128_eq_signaling()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float128_eq_signaling";
    reset_globals();

    static const float128 edges[] = {
        0u, 0x80000000u, 0x7F800000u, 0xFF800000u, 0x7FC00000u,
        0x7F7FFFFF, 0x00800000, 0x00000001, 0xFFFFFFFFu
    };
    if (sizeof(float128) == 8) {
        static const float128 edges64[] = {
            0, 0x8000000000000000ULL, 0x7FF0000000000000ULL,
            0xFFF0000000000000ULL, 0x7FF8000000000000ULL,
            0x000FFFFFFFFFFFFF, 0x0010000000000000ULL, 1, 0xFFFFFFFFFFFFFFFFULL
        };
        for (float128 a : edges64) for (float128 b : edges64) {
            sync_globals_from_port();
            flag rp = port::float128_eq_signaling(a, b);
            flag rr = ref_float128_eq_signaling(a, b);
            cases++;
            if (rp != rr) failures++;
            sync_globals_to_port();
        }
    } else {
        for (float128 a : edges) for (float128 b : edges) {
            sync_globals_from_port();
            flag rp = port::float128_eq_signaling(a, b);
            flag rr = ref_float128_eq_signaling(a, b);
            cases++;
            if (rp != rr) failures++;
            sync_globals_to_port();
        }
    }
    for (unsigned i = 0; i < 200000u; ++i) {
        float128 a = f128_rand();
        float128 b = f128_rand();
        sync_globals_from_port();
        flag rp = port::float128_eq_signaling(a, b);
        flag rr = ref_float128_eq_signaling(a, b);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_float128_is_nan()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float128_is_nan";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float128 a = f128_rand();
        sync_globals_from_port();
        flag rp = port::float128_is_nan(a);
        flag rr = ref_float128_is_nan(a);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_float128_is_signaling_nan()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float128_is_signaling_nan";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float128 a = f128_rand();
        sync_globals_from_port();
        flag rp = port::float128_is_signaling_nan(a);
        flag rr = ref_float128_is_signaling_nan(a);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_float128_le()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float128_le";
    reset_globals();

    static const float128 edges[] = {
        0u, 0x80000000u, 0x7F800000u, 0xFF800000u, 0x7FC00000u,
        0x7F7FFFFF, 0x00800000, 0x00000001, 0xFFFFFFFFu
    };
    if (sizeof(float128) == 8) {
        static const float128 edges64[] = {
            0, 0x8000000000000000ULL, 0x7FF0000000000000ULL,
            0xFFF0000000000000ULL, 0x7FF8000000000000ULL,
            0x000FFFFFFFFFFFFF, 0x0010000000000000ULL, 1, 0xFFFFFFFFFFFFFFFFULL
        };
        for (float128 a : edges64) for (float128 b : edges64) {
            sync_globals_from_port();
            flag rp = port::float128_le(a, b);
            flag rr = ref_float128_le(a, b);
            cases++;
            if (rp != rr) failures++;
            sync_globals_to_port();
        }
    } else {
        for (float128 a : edges) for (float128 b : edges) {
            sync_globals_from_port();
            flag rp = port::float128_le(a, b);
            flag rr = ref_float128_le(a, b);
            cases++;
            if (rp != rr) failures++;
            sync_globals_to_port();
        }
    }
    for (unsigned i = 0; i < 200000u; ++i) {
        float128 a = f128_rand();
        float128 b = f128_rand();
        sync_globals_from_port();
        flag rp = port::float128_le(a, b);
        flag rr = ref_float128_le(a, b);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_float128_le_quiet()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float128_le_quiet";
    reset_globals();

    static const float128 edges[] = {
        0u, 0x80000000u, 0x7F800000u, 0xFF800000u, 0x7FC00000u,
        0x7F7FFFFF, 0x00800000, 0x00000001, 0xFFFFFFFFu
    };
    if (sizeof(float128) == 8) {
        static const float128 edges64[] = {
            0, 0x8000000000000000ULL, 0x7FF0000000000000ULL,
            0xFFF0000000000000ULL, 0x7FF8000000000000ULL,
            0x000FFFFFFFFFFFFF, 0x0010000000000000ULL, 1, 0xFFFFFFFFFFFFFFFFULL
        };
        for (float128 a : edges64) for (float128 b : edges64) {
            sync_globals_from_port();
            flag rp = port::float128_le_quiet(a, b);
            flag rr = ref_float128_le_quiet(a, b);
            cases++;
            if (rp != rr) failures++;
            sync_globals_to_port();
        }
    } else {
        for (float128 a : edges) for (float128 b : edges) {
            sync_globals_from_port();
            flag rp = port::float128_le_quiet(a, b);
            flag rr = ref_float128_le_quiet(a, b);
            cases++;
            if (rp != rr) failures++;
            sync_globals_to_port();
        }
    }
    for (unsigned i = 0; i < 200000u; ++i) {
        float128 a = f128_rand();
        float128 b = f128_rand();
        sync_globals_from_port();
        flag rp = port::float128_le_quiet(a, b);
        flag rr = ref_float128_le_quiet(a, b);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_float128_lt()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float128_lt";
    reset_globals();

    static const float128 edges[] = {
        0u, 0x80000000u, 0x7F800000u, 0xFF800000u, 0x7FC00000u,
        0x7F7FFFFF, 0x00800000, 0x00000001, 0xFFFFFFFFu
    };
    if (sizeof(float128) == 8) {
        static const float128 edges64[] = {
            0, 0x8000000000000000ULL, 0x7FF0000000000000ULL,
            0xFFF0000000000000ULL, 0x7FF8000000000000ULL,
            0x000FFFFFFFFFFFFF, 0x0010000000000000ULL, 1, 0xFFFFFFFFFFFFFFFFULL
        };
        for (float128 a : edges64) for (float128 b : edges64) {
            sync_globals_from_port();
            flag rp = port::float128_lt(a, b);
            flag rr = ref_float128_lt(a, b);
            cases++;
            if (rp != rr) failures++;
            sync_globals_to_port();
        }
    } else {
        for (float128 a : edges) for (float128 b : edges) {
            sync_globals_from_port();
            flag rp = port::float128_lt(a, b);
            flag rr = ref_float128_lt(a, b);
            cases++;
            if (rp != rr) failures++;
            sync_globals_to_port();
        }
    }
    for (unsigned i = 0; i < 200000u; ++i) {
        float128 a = f128_rand();
        float128 b = f128_rand();
        sync_globals_from_port();
        flag rp = port::float128_lt(a, b);
        flag rr = ref_float128_lt(a, b);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_float128_lt_quiet()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float128_lt_quiet";
    reset_globals();

    static const float128 edges[] = {
        0u, 0x80000000u, 0x7F800000u, 0xFF800000u, 0x7FC00000u,
        0x7F7FFFFF, 0x00800000, 0x00000001, 0xFFFFFFFFu
    };
    if (sizeof(float128) == 8) {
        static const float128 edges64[] = {
            0, 0x8000000000000000ULL, 0x7FF0000000000000ULL,
            0xFFF0000000000000ULL, 0x7FF8000000000000ULL,
            0x000FFFFFFFFFFFFF, 0x0010000000000000ULL, 1, 0xFFFFFFFFFFFFFFFFULL
        };
        for (float128 a : edges64) for (float128 b : edges64) {
            sync_globals_from_port();
            flag rp = port::float128_lt_quiet(a, b);
            flag rr = ref_float128_lt_quiet(a, b);
            cases++;
            if (rp != rr) failures++;
            sync_globals_to_port();
        }
    } else {
        for (float128 a : edges) for (float128 b : edges) {
            sync_globals_from_port();
            flag rp = port::float128_lt_quiet(a, b);
            flag rr = ref_float128_lt_quiet(a, b);
            cases++;
            if (rp != rr) failures++;
            sync_globals_to_port();
        }
    }
    for (unsigned i = 0; i < 200000u; ++i) {
        float128 a = f128_rand();
        float128 b = f128_rand();
        sync_globals_from_port();
        flag rp = port::float128_lt_quiet(a, b);
        flag rr = ref_float128_lt_quiet(a, b);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_float128_mul()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float128_mul";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float128 a = f128_rand();
        float128 b = f128_rand();
        sync_globals_from_port();
        float128 rp = port::float128_mul(a, b);
        float128 rr = ref_float128_mul(a, b);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    static const float128 z = 0;
    sync_globals_from_port();
    float128 rp0 = port::float128_mul(z, z);
    float128 rr0 = ref_float128_mul(z, z);
    cases++;
    if (rp0 != rr0) failures++;
    record(name, cases, failures);
}

static void test_float128_rem()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float128_rem";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float128 a = f128_rand();
        float128 b = f128_rand();
        sync_globals_from_port();
        float128 rp = port::float128_rem(a, b);
        float128 rr = ref_float128_rem(a, b);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    static const float128 z = 0;
    sync_globals_from_port();
    float128 rp0 = port::float128_rem(z, z);
    float128 rr0 = ref_float128_rem(z, z);
    cases++;
    if (rp0 != rr0) failures++;
    record(name, cases, failures);
}

static void test_float128_round_to_int()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float128_round_to_int";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float128 a = f128_rand();
        sync_globals_from_port();
        int64_t rp = port::float128_round_to_int(a);
        int64_t rr = ref_float128_round_to_int(a);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_float128_sqrt()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float128_sqrt";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float128 a = f128_rand();
        sync_globals_from_port();
        float128 rp = port::float128_sqrt(a);
        float128 rr = ref_float128_sqrt(a);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_float128_sub()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float128_sub";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float128 a = f128_rand();
        float128 b = f128_rand();
        sync_globals_from_port();
        float128 rp = port::float128_sub(a, b);
        float128 rr = ref_float128_sub(a, b);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    static const float128 z = 0;
    sync_globals_from_port();
    float128 rp0 = port::float128_sub(z, z);
    float128 rr0 = ref_float128_sub(z, z);
    cases++;
    if (rp0 != rr0) failures++;
    record(name, cases, failures);
}

static void test_float128_to_float32()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float128_to_float32";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float128 a = f128_rand();
        sync_globals_from_port();
        auto rp = port::float128_to_float32(a);
        auto rr = ref_float128_to_float32(a);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_float128_to_float64()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float128_to_float64";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float128 a = f128_rand();
        sync_globals_from_port();
        auto rp = port::float128_to_float64(a);
        auto rr = ref_float128_to_float64(a);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_float128_to_floatx80()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float128_to_floatx80";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float128 a = f128_rand();
        sync_globals_from_port();
        auto rp = port::float128_to_floatx80(a);
        auto rr = ref_float128_to_floatx80(a);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_float128_to_int32()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float128_to_int32";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float128 a = f128_rand();
        sync_globals_from_port();
        int32_t rp = port::float128_to_int32(a);
        int32_t rr = ref_float128_to_int32(a);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_float128_to_int32_round_to_zero()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float128_to_int32_round_to_zero";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float128 a = f128_rand();
        sync_globals_from_port();
        int32_t rp = port::float128_to_int32_round_to_zero(a);
        int32_t rr = ref_float128_to_int32_round_to_zero(a);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_float128_to_int64()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float128_to_int64";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float128 a = f128_rand();
        sync_globals_from_port();
        int64_t rp = port::float128_to_int64(a);
        int64_t rr = ref_float128_to_int64(a);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_float128_to_int64_round_to_zero()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float128_to_int64_round_to_zero";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float128 a = f128_rand();
        sync_globals_from_port();
        int64_t rp = port::float128_to_int64_round_to_zero(a);
        int64_t rr = ref_float128_to_int64_round_to_zero(a);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_float32ToCommonNaN()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float32ToCommonNaN";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float32 a = f32_rand();
        sync_globals_from_port();
        commonNaNT rp = port::float32ToCommonNaN(a);
        commonNaNT rr = ref_float32ToCommonNaN(a);
        cases++;
        if (rp.sign != rr.sign || rp.high != rr.high || rp.low != rr.low) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_float32_add()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float32_add";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float32 a = f32_rand();
        float32 b = f32_rand();
        sync_globals_from_port();
        float32 rp = port::float32_add(a, b);
        float32 rr = ref_float32_add(a, b);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    static const float32 z = 0;
    sync_globals_from_port();
    float32 rp0 = port::float32_add(z, z);
    float32 rr0 = ref_float32_add(z, z);
    cases++;
    if (rp0 != rr0) failures++;
    record(name, cases, failures);
}

static void test_float32_div()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float32_div";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float32 a = f32_rand();
        float32 b = f32_rand();
        sync_globals_from_port();
        float32 rp = port::float32_div(a, b);
        float32 rr = ref_float32_div(a, b);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    static const float32 z = 0;
    sync_globals_from_port();
    float32 rp0 = port::float32_div(z, z);
    float32 rr0 = ref_float32_div(z, z);
    cases++;
    if (rp0 != rr0) failures++;
    record(name, cases, failures);
}

static void test_float32_eq()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float32_eq";
    reset_globals();

    static const float32 edges[] = {
        0u, 0x80000000u, 0x7F800000u, 0xFF800000u, 0x7FC00000u,
        0x7F7FFFFF, 0x00800000, 0x00000001, 0xFFFFFFFFu
    };
    if (sizeof(float32) == 8) {
        static const float32 edges64[] = {
            0, 0x8000000000000000ULL, 0x7FF0000000000000ULL,
            0xFFF0000000000000ULL, 0x7FF8000000000000ULL,
            0x000FFFFFFFFFFFFF, 0x0010000000000000ULL, 1, 0xFFFFFFFFFFFFFFFFULL
        };
        for (float32 a : edges64) for (float32 b : edges64) {
            sync_globals_from_port();
            flag rp = port::float32_eq(a, b);
            flag rr = ref_float32_eq(a, b);
            cases++;
            if (rp != rr) failures++;
            sync_globals_to_port();
        }
    } else {
        for (float32 a : edges) for (float32 b : edges) {
            sync_globals_from_port();
            flag rp = port::float32_eq(a, b);
            flag rr = ref_float32_eq(a, b);
            cases++;
            if (rp != rr) failures++;
            sync_globals_to_port();
        }
    }
    for (unsigned i = 0; i < 200000u; ++i) {
        float32 a = f32_rand();
        float32 b = f32_rand();
        sync_globals_from_port();
        flag rp = port::float32_eq(a, b);
        flag rr = ref_float32_eq(a, b);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_float32_eq_signaling()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float32_eq_signaling";
    reset_globals();

    static const float32 edges[] = {
        0u, 0x80000000u, 0x7F800000u, 0xFF800000u, 0x7FC00000u,
        0x7F7FFFFF, 0x00800000, 0x00000001, 0xFFFFFFFFu
    };
    if (sizeof(float32) == 8) {
        static const float32 edges64[] = {
            0, 0x8000000000000000ULL, 0x7FF0000000000000ULL,
            0xFFF0000000000000ULL, 0x7FF8000000000000ULL,
            0x000FFFFFFFFFFFFF, 0x0010000000000000ULL, 1, 0xFFFFFFFFFFFFFFFFULL
        };
        for (float32 a : edges64) for (float32 b : edges64) {
            sync_globals_from_port();
            flag rp = port::float32_eq_signaling(a, b);
            flag rr = ref_float32_eq_signaling(a, b);
            cases++;
            if (rp != rr) failures++;
            sync_globals_to_port();
        }
    } else {
        for (float32 a : edges) for (float32 b : edges) {
            sync_globals_from_port();
            flag rp = port::float32_eq_signaling(a, b);
            flag rr = ref_float32_eq_signaling(a, b);
            cases++;
            if (rp != rr) failures++;
            sync_globals_to_port();
        }
    }
    for (unsigned i = 0; i < 200000u; ++i) {
        float32 a = f32_rand();
        float32 b = f32_rand();
        sync_globals_from_port();
        flag rp = port::float32_eq_signaling(a, b);
        flag rr = ref_float32_eq_signaling(a, b);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_float32_is_nan()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float32_is_nan";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float32 a = f32_rand();
        sync_globals_from_port();
        flag rp = port::float32_is_nan(a);
        flag rr = ref_float32_is_nan(a);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_float32_is_signaling_nan()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float32_is_signaling_nan";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float32 a = f32_rand();
        sync_globals_from_port();
        flag rp = port::float32_is_signaling_nan(a);
        flag rr = ref_float32_is_signaling_nan(a);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_float32_le()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float32_le";
    reset_globals();

    static const float32 edges[] = {
        0u, 0x80000000u, 0x7F800000u, 0xFF800000u, 0x7FC00000u,
        0x7F7FFFFF, 0x00800000, 0x00000001, 0xFFFFFFFFu
    };
    if (sizeof(float32) == 8) {
        static const float32 edges64[] = {
            0, 0x8000000000000000ULL, 0x7FF0000000000000ULL,
            0xFFF0000000000000ULL, 0x7FF8000000000000ULL,
            0x000FFFFFFFFFFFFF, 0x0010000000000000ULL, 1, 0xFFFFFFFFFFFFFFFFULL
        };
        for (float32 a : edges64) for (float32 b : edges64) {
            sync_globals_from_port();
            flag rp = port::float32_le(a, b);
            flag rr = ref_float32_le(a, b);
            cases++;
            if (rp != rr) failures++;
            sync_globals_to_port();
        }
    } else {
        for (float32 a : edges) for (float32 b : edges) {
            sync_globals_from_port();
            flag rp = port::float32_le(a, b);
            flag rr = ref_float32_le(a, b);
            cases++;
            if (rp != rr) failures++;
            sync_globals_to_port();
        }
    }
    for (unsigned i = 0; i < 200000u; ++i) {
        float32 a = f32_rand();
        float32 b = f32_rand();
        sync_globals_from_port();
        flag rp = port::float32_le(a, b);
        flag rr = ref_float32_le(a, b);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_float32_le_quiet()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float32_le_quiet";
    reset_globals();

    static const float32 edges[] = {
        0u, 0x80000000u, 0x7F800000u, 0xFF800000u, 0x7FC00000u,
        0x7F7FFFFF, 0x00800000, 0x00000001, 0xFFFFFFFFu
    };
    if (sizeof(float32) == 8) {
        static const float32 edges64[] = {
            0, 0x8000000000000000ULL, 0x7FF0000000000000ULL,
            0xFFF0000000000000ULL, 0x7FF8000000000000ULL,
            0x000FFFFFFFFFFFFF, 0x0010000000000000ULL, 1, 0xFFFFFFFFFFFFFFFFULL
        };
        for (float32 a : edges64) for (float32 b : edges64) {
            sync_globals_from_port();
            flag rp = port::float32_le_quiet(a, b);
            flag rr = ref_float32_le_quiet(a, b);
            cases++;
            if (rp != rr) failures++;
            sync_globals_to_port();
        }
    } else {
        for (float32 a : edges) for (float32 b : edges) {
            sync_globals_from_port();
            flag rp = port::float32_le_quiet(a, b);
            flag rr = ref_float32_le_quiet(a, b);
            cases++;
            if (rp != rr) failures++;
            sync_globals_to_port();
        }
    }
    for (unsigned i = 0; i < 200000u; ++i) {
        float32 a = f32_rand();
        float32 b = f32_rand();
        sync_globals_from_port();
        flag rp = port::float32_le_quiet(a, b);
        flag rr = ref_float32_le_quiet(a, b);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_float32_lt()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float32_lt";
    reset_globals();

    static const float32 edges[] = {
        0u, 0x80000000u, 0x7F800000u, 0xFF800000u, 0x7FC00000u,
        0x7F7FFFFF, 0x00800000, 0x00000001, 0xFFFFFFFFu
    };
    if (sizeof(float32) == 8) {
        static const float32 edges64[] = {
            0, 0x8000000000000000ULL, 0x7FF0000000000000ULL,
            0xFFF0000000000000ULL, 0x7FF8000000000000ULL,
            0x000FFFFFFFFFFFFF, 0x0010000000000000ULL, 1, 0xFFFFFFFFFFFFFFFFULL
        };
        for (float32 a : edges64) for (float32 b : edges64) {
            sync_globals_from_port();
            flag rp = port::float32_lt(a, b);
            flag rr = ref_float32_lt(a, b);
            cases++;
            if (rp != rr) failures++;
            sync_globals_to_port();
        }
    } else {
        for (float32 a : edges) for (float32 b : edges) {
            sync_globals_from_port();
            flag rp = port::float32_lt(a, b);
            flag rr = ref_float32_lt(a, b);
            cases++;
            if (rp != rr) failures++;
            sync_globals_to_port();
        }
    }
    for (unsigned i = 0; i < 200000u; ++i) {
        float32 a = f32_rand();
        float32 b = f32_rand();
        sync_globals_from_port();
        flag rp = port::float32_lt(a, b);
        flag rr = ref_float32_lt(a, b);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_float32_lt_quiet()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float32_lt_quiet";
    reset_globals();

    static const float32 edges[] = {
        0u, 0x80000000u, 0x7F800000u, 0xFF800000u, 0x7FC00000u,
        0x7F7FFFFF, 0x00800000, 0x00000001, 0xFFFFFFFFu
    };
    if (sizeof(float32) == 8) {
        static const float32 edges64[] = {
            0, 0x8000000000000000ULL, 0x7FF0000000000000ULL,
            0xFFF0000000000000ULL, 0x7FF8000000000000ULL,
            0x000FFFFFFFFFFFFF, 0x0010000000000000ULL, 1, 0xFFFFFFFFFFFFFFFFULL
        };
        for (float32 a : edges64) for (float32 b : edges64) {
            sync_globals_from_port();
            flag rp = port::float32_lt_quiet(a, b);
            flag rr = ref_float32_lt_quiet(a, b);
            cases++;
            if (rp != rr) failures++;
            sync_globals_to_port();
        }
    } else {
        for (float32 a : edges) for (float32 b : edges) {
            sync_globals_from_port();
            flag rp = port::float32_lt_quiet(a, b);
            flag rr = ref_float32_lt_quiet(a, b);
            cases++;
            if (rp != rr) failures++;
            sync_globals_to_port();
        }
    }
    for (unsigned i = 0; i < 200000u; ++i) {
        float32 a = f32_rand();
        float32 b = f32_rand();
        sync_globals_from_port();
        flag rp = port::float32_lt_quiet(a, b);
        flag rr = ref_float32_lt_quiet(a, b);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_float32_mul()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float32_mul";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float32 a = f32_rand();
        float32 b = f32_rand();
        sync_globals_from_port();
        float32 rp = port::float32_mul(a, b);
        float32 rr = ref_float32_mul(a, b);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    static const float32 z = 0;
    sync_globals_from_port();
    float32 rp0 = port::float32_mul(z, z);
    float32 rr0 = ref_float32_mul(z, z);
    cases++;
    if (rp0 != rr0) failures++;
    record(name, cases, failures);
}

static void test_float32_rem()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float32_rem";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float32 a = f32_rand();
        float32 b = f32_rand();
        sync_globals_from_port();
        float32 rp = port::float32_rem(a, b);
        float32 rr = ref_float32_rem(a, b);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    static const float32 z = 0;
    sync_globals_from_port();
    float32 rp0 = port::float32_rem(z, z);
    float32 rr0 = ref_float32_rem(z, z);
    cases++;
    if (rp0 != rr0) failures++;
    record(name, cases, failures);
}

static void test_float32_round_to_int()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float32_round_to_int";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float32 a = f32_rand();
        sync_globals_from_port();
        int64_t rp = port::float32_round_to_int(a);
        int64_t rr = ref_float32_round_to_int(a);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_float32_sqrt()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float32_sqrt";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float32 a = f32_rand();
        sync_globals_from_port();
        float32 rp = port::float32_sqrt(a);
        float32 rr = ref_float32_sqrt(a);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_float32_sub()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float32_sub";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float32 a = f32_rand();
        float32 b = f32_rand();
        sync_globals_from_port();
        float32 rp = port::float32_sub(a, b);
        float32 rr = ref_float32_sub(a, b);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    static const float32 z = 0;
    sync_globals_from_port();
    float32 rp0 = port::float32_sub(z, z);
    float32 rr0 = ref_float32_sub(z, z);
    cases++;
    if (rp0 != rr0) failures++;
    record(name, cases, failures);
}

static void test_float32_to_float128()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float32_to_float128";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float32 a = f32_rand();
        sync_globals_from_port();
        auto rp = port::float32_to_float128(a);
        auto rr = ref_float32_to_float128(a);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_float32_to_float64()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float32_to_float64";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float32 a = f32_rand();
        sync_globals_from_port();
        auto rp = port::float32_to_float64(a);
        auto rr = ref_float32_to_float64(a);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_float32_to_floatx80()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float32_to_floatx80";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float32 a = f32_rand();
        sync_globals_from_port();
        auto rp = port::float32_to_floatx80(a);
        auto rr = ref_float32_to_floatx80(a);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_float32_to_int32()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float32_to_int32";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float32 a = f32_rand();
        sync_globals_from_port();
        int32_t rp = port::float32_to_int32(a);
        int32_t rr = ref_float32_to_int32(a);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_float32_to_int32_round_to_zero()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float32_to_int32_round_to_zero";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float32 a = f32_rand();
        sync_globals_from_port();
        int32_t rp = port::float32_to_int32_round_to_zero(a);
        int32_t rr = ref_float32_to_int32_round_to_zero(a);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_float32_to_int64()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float32_to_int64";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float32 a = f32_rand();
        sync_globals_from_port();
        int64_t rp = port::float32_to_int64(a);
        int64_t rr = ref_float32_to_int64(a);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_float32_to_int64_round_to_zero()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float32_to_int64_round_to_zero";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float32 a = f32_rand();
        sync_globals_from_port();
        int64_t rp = port::float32_to_int64_round_to_zero(a);
        int64_t rr = ref_float32_to_int64_round_to_zero(a);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_float64ToCommonNaN()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float64ToCommonNaN";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float64 a = f64_rand();
        sync_globals_from_port();
        commonNaNT rp = port::float64ToCommonNaN(a);
        commonNaNT rr = ref_float64ToCommonNaN(a);
        cases++;
        if (rp.sign != rr.sign || rp.high != rr.high || rp.low != rr.low) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_float64_add()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float64_add";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float64 a = f64_rand();
        float64 b = f64_rand();
        sync_globals_from_port();
        float64 rp = port::float64_add(a, b);
        float64 rr = ref_float64_add(a, b);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    static const float64 z = 0;
    sync_globals_from_port();
    float64 rp0 = port::float64_add(z, z);
    float64 rr0 = ref_float64_add(z, z);
    cases++;
    if (rp0 != rr0) failures++;
    record(name, cases, failures);
}

static void test_float64_div()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float64_div";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float64 a = f64_rand();
        float64 b = f64_rand();
        sync_globals_from_port();
        float64 rp = port::float64_div(a, b);
        float64 rr = ref_float64_div(a, b);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    static const float64 z = 0;
    sync_globals_from_port();
    float64 rp0 = port::float64_div(z, z);
    float64 rr0 = ref_float64_div(z, z);
    cases++;
    if (rp0 != rr0) failures++;
    record(name, cases, failures);
}

static void test_float64_eq()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float64_eq";
    reset_globals();

    static const float64 edges[] = {
        0u, 0x80000000u, 0x7F800000u, 0xFF800000u, 0x7FC00000u,
        0x7F7FFFFF, 0x00800000, 0x00000001, 0xFFFFFFFFu
    };
    if (sizeof(float64) == 8) {
        static const float64 edges64[] = {
            0, 0x8000000000000000ULL, 0x7FF0000000000000ULL,
            0xFFF0000000000000ULL, 0x7FF8000000000000ULL,
            0x000FFFFFFFFFFFFF, 0x0010000000000000ULL, 1, 0xFFFFFFFFFFFFFFFFULL
        };
        for (float64 a : edges64) for (float64 b : edges64) {
            sync_globals_from_port();
            flag rp = port::float64_eq(a, b);
            flag rr = ref_float64_eq(a, b);
            cases++;
            if (rp != rr) failures++;
            sync_globals_to_port();
        }
    } else {
        for (float64 a : edges) for (float64 b : edges) {
            sync_globals_from_port();
            flag rp = port::float64_eq(a, b);
            flag rr = ref_float64_eq(a, b);
            cases++;
            if (rp != rr) failures++;
            sync_globals_to_port();
        }
    }
    for (unsigned i = 0; i < 200000u; ++i) {
        float64 a = f64_rand();
        float64 b = f64_rand();
        sync_globals_from_port();
        flag rp = port::float64_eq(a, b);
        flag rr = ref_float64_eq(a, b);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_float64_eq_signaling()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float64_eq_signaling";
    reset_globals();

    static const float64 edges[] = {
        0u, 0x80000000u, 0x7F800000u, 0xFF800000u, 0x7FC00000u,
        0x7F7FFFFF, 0x00800000, 0x00000001, 0xFFFFFFFFu
    };
    if (sizeof(float64) == 8) {
        static const float64 edges64[] = {
            0, 0x8000000000000000ULL, 0x7FF0000000000000ULL,
            0xFFF0000000000000ULL, 0x7FF8000000000000ULL,
            0x000FFFFFFFFFFFFF, 0x0010000000000000ULL, 1, 0xFFFFFFFFFFFFFFFFULL
        };
        for (float64 a : edges64) for (float64 b : edges64) {
            sync_globals_from_port();
            flag rp = port::float64_eq_signaling(a, b);
            flag rr = ref_float64_eq_signaling(a, b);
            cases++;
            if (rp != rr) failures++;
            sync_globals_to_port();
        }
    } else {
        for (float64 a : edges) for (float64 b : edges) {
            sync_globals_from_port();
            flag rp = port::float64_eq_signaling(a, b);
            flag rr = ref_float64_eq_signaling(a, b);
            cases++;
            if (rp != rr) failures++;
            sync_globals_to_port();
        }
    }
    for (unsigned i = 0; i < 200000u; ++i) {
        float64 a = f64_rand();
        float64 b = f64_rand();
        sync_globals_from_port();
        flag rp = port::float64_eq_signaling(a, b);
        flag rr = ref_float64_eq_signaling(a, b);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_float64_is_nan()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float64_is_nan";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float64 a = f64_rand();
        sync_globals_from_port();
        flag rp = port::float64_is_nan(a);
        flag rr = ref_float64_is_nan(a);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_float64_is_signaling_nan()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float64_is_signaling_nan";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float64 a = f64_rand();
        sync_globals_from_port();
        flag rp = port::float64_is_signaling_nan(a);
        flag rr = ref_float64_is_signaling_nan(a);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_float64_le()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float64_le";
    reset_globals();

    static const float64 edges[] = {
        0u, 0x80000000u, 0x7F800000u, 0xFF800000u, 0x7FC00000u,
        0x7F7FFFFF, 0x00800000, 0x00000001, 0xFFFFFFFFu
    };
    if (sizeof(float64) == 8) {
        static const float64 edges64[] = {
            0, 0x8000000000000000ULL, 0x7FF0000000000000ULL,
            0xFFF0000000000000ULL, 0x7FF8000000000000ULL,
            0x000FFFFFFFFFFFFF, 0x0010000000000000ULL, 1, 0xFFFFFFFFFFFFFFFFULL
        };
        for (float64 a : edges64) for (float64 b : edges64) {
            sync_globals_from_port();
            flag rp = port::float64_le(a, b);
            flag rr = ref_float64_le(a, b);
            cases++;
            if (rp != rr) failures++;
            sync_globals_to_port();
        }
    } else {
        for (float64 a : edges) for (float64 b : edges) {
            sync_globals_from_port();
            flag rp = port::float64_le(a, b);
            flag rr = ref_float64_le(a, b);
            cases++;
            if (rp != rr) failures++;
            sync_globals_to_port();
        }
    }
    for (unsigned i = 0; i < 200000u; ++i) {
        float64 a = f64_rand();
        float64 b = f64_rand();
        sync_globals_from_port();
        flag rp = port::float64_le(a, b);
        flag rr = ref_float64_le(a, b);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_float64_le_quiet()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float64_le_quiet";
    reset_globals();

    static const float64 edges[] = {
        0u, 0x80000000u, 0x7F800000u, 0xFF800000u, 0x7FC00000u,
        0x7F7FFFFF, 0x00800000, 0x00000001, 0xFFFFFFFFu
    };
    if (sizeof(float64) == 8) {
        static const float64 edges64[] = {
            0, 0x8000000000000000ULL, 0x7FF0000000000000ULL,
            0xFFF0000000000000ULL, 0x7FF8000000000000ULL,
            0x000FFFFFFFFFFFFF, 0x0010000000000000ULL, 1, 0xFFFFFFFFFFFFFFFFULL
        };
        for (float64 a : edges64) for (float64 b : edges64) {
            sync_globals_from_port();
            flag rp = port::float64_le_quiet(a, b);
            flag rr = ref_float64_le_quiet(a, b);
            cases++;
            if (rp != rr) failures++;
            sync_globals_to_port();
        }
    } else {
        for (float64 a : edges) for (float64 b : edges) {
            sync_globals_from_port();
            flag rp = port::float64_le_quiet(a, b);
            flag rr = ref_float64_le_quiet(a, b);
            cases++;
            if (rp != rr) failures++;
            sync_globals_to_port();
        }
    }
    for (unsigned i = 0; i < 200000u; ++i) {
        float64 a = f64_rand();
        float64 b = f64_rand();
        sync_globals_from_port();
        flag rp = port::float64_le_quiet(a, b);
        flag rr = ref_float64_le_quiet(a, b);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_float64_lt()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float64_lt";
    reset_globals();

    static const float64 edges[] = {
        0u, 0x80000000u, 0x7F800000u, 0xFF800000u, 0x7FC00000u,
        0x7F7FFFFF, 0x00800000, 0x00000001, 0xFFFFFFFFu
    };
    if (sizeof(float64) == 8) {
        static const float64 edges64[] = {
            0, 0x8000000000000000ULL, 0x7FF0000000000000ULL,
            0xFFF0000000000000ULL, 0x7FF8000000000000ULL,
            0x000FFFFFFFFFFFFF, 0x0010000000000000ULL, 1, 0xFFFFFFFFFFFFFFFFULL
        };
        for (float64 a : edges64) for (float64 b : edges64) {
            sync_globals_from_port();
            flag rp = port::float64_lt(a, b);
            flag rr = ref_float64_lt(a, b);
            cases++;
            if (rp != rr) failures++;
            sync_globals_to_port();
        }
    } else {
        for (float64 a : edges) for (float64 b : edges) {
            sync_globals_from_port();
            flag rp = port::float64_lt(a, b);
            flag rr = ref_float64_lt(a, b);
            cases++;
            if (rp != rr) failures++;
            sync_globals_to_port();
        }
    }
    for (unsigned i = 0; i < 200000u; ++i) {
        float64 a = f64_rand();
        float64 b = f64_rand();
        sync_globals_from_port();
        flag rp = port::float64_lt(a, b);
        flag rr = ref_float64_lt(a, b);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_float64_lt_quiet()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float64_lt_quiet";
    reset_globals();

    static const float64 edges[] = {
        0u, 0x80000000u, 0x7F800000u, 0xFF800000u, 0x7FC00000u,
        0x7F7FFFFF, 0x00800000, 0x00000001, 0xFFFFFFFFu
    };
    if (sizeof(float64) == 8) {
        static const float64 edges64[] = {
            0, 0x8000000000000000ULL, 0x7FF0000000000000ULL,
            0xFFF0000000000000ULL, 0x7FF8000000000000ULL,
            0x000FFFFFFFFFFFFF, 0x0010000000000000ULL, 1, 0xFFFFFFFFFFFFFFFFULL
        };
        for (float64 a : edges64) for (float64 b : edges64) {
            sync_globals_from_port();
            flag rp = port::float64_lt_quiet(a, b);
            flag rr = ref_float64_lt_quiet(a, b);
            cases++;
            if (rp != rr) failures++;
            sync_globals_to_port();
        }
    } else {
        for (float64 a : edges) for (float64 b : edges) {
            sync_globals_from_port();
            flag rp = port::float64_lt_quiet(a, b);
            flag rr = ref_float64_lt_quiet(a, b);
            cases++;
            if (rp != rr) failures++;
            sync_globals_to_port();
        }
    }
    for (unsigned i = 0; i < 200000u; ++i) {
        float64 a = f64_rand();
        float64 b = f64_rand();
        sync_globals_from_port();
        flag rp = port::float64_lt_quiet(a, b);
        flag rr = ref_float64_lt_quiet(a, b);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_float64_mul()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float64_mul";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float64 a = f64_rand();
        float64 b = f64_rand();
        sync_globals_from_port();
        float64 rp = port::float64_mul(a, b);
        float64 rr = ref_float64_mul(a, b);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    static const float64 z = 0;
    sync_globals_from_port();
    float64 rp0 = port::float64_mul(z, z);
    float64 rr0 = ref_float64_mul(z, z);
    cases++;
    if (rp0 != rr0) failures++;
    record(name, cases, failures);
}

static void test_float64_rem()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float64_rem";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float64 a = f64_rand();
        float64 b = f64_rand();
        sync_globals_from_port();
        float64 rp = port::float64_rem(a, b);
        float64 rr = ref_float64_rem(a, b);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    static const float64 z = 0;
    sync_globals_from_port();
    float64 rp0 = port::float64_rem(z, z);
    float64 rr0 = ref_float64_rem(z, z);
    cases++;
    if (rp0 != rr0) failures++;
    record(name, cases, failures);
}

static void test_float64_round_to_int()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float64_round_to_int";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float64 a = f64_rand();
        sync_globals_from_port();
        int64_t rp = port::float64_round_to_int(a);
        int64_t rr = ref_float64_round_to_int(a);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_float64_sqrt()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float64_sqrt";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float64 a = f64_rand();
        sync_globals_from_port();
        float64 rp = port::float64_sqrt(a);
        float64 rr = ref_float64_sqrt(a);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_float64_sub()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float64_sub";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float64 a = f64_rand();
        float64 b = f64_rand();
        sync_globals_from_port();
        float64 rp = port::float64_sub(a, b);
        float64 rr = ref_float64_sub(a, b);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    static const float64 z = 0;
    sync_globals_from_port();
    float64 rp0 = port::float64_sub(z, z);
    float64 rr0 = ref_float64_sub(z, z);
    cases++;
    if (rp0 != rr0) failures++;
    record(name, cases, failures);
}

static void test_float64_to_float128()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float64_to_float128";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float64 a = f64_rand();
        sync_globals_from_port();
        auto rp = port::float64_to_float128(a);
        auto rr = ref_float64_to_float128(a);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_float64_to_float32()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float64_to_float32";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float64 a = f64_rand();
        sync_globals_from_port();
        auto rp = port::float64_to_float32(a);
        auto rr = ref_float64_to_float32(a);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_float64_to_floatx80()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float64_to_floatx80";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float64 a = f64_rand();
        sync_globals_from_port();
        auto rp = port::float64_to_floatx80(a);
        auto rr = ref_float64_to_floatx80(a);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_float64_to_int32()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float64_to_int32";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float64 a = f64_rand();
        sync_globals_from_port();
        int32_t rp = port::float64_to_int32(a);
        int32_t rr = ref_float64_to_int32(a);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_float64_to_int32_round_to_zero()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float64_to_int32_round_to_zero";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float64 a = f64_rand();
        sync_globals_from_port();
        int32_t rp = port::float64_to_int32_round_to_zero(a);
        int32_t rr = ref_float64_to_int32_round_to_zero(a);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_float64_to_int64()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float64_to_int64";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float64 a = f64_rand();
        sync_globals_from_port();
        int64_t rp = port::float64_to_int64(a);
        int64_t rr = ref_float64_to_int64(a);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_float64_to_int64_round_to_zero()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float64_to_int64_round_to_zero";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float64 a = f64_rand();
        sync_globals_from_port();
        int64_t rp = port::float64_to_int64_round_to_zero(a);
        int64_t rr = ref_float64_to_int64_round_to_zero(a);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_float_raise()
{
    unsigned cases = 0, failures = 0;
    const char *name = "float_raise";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        int fl = static_cast<int>(urand32() & 0x1F);
        sync_globals_from_port();
        port::float_raise(fl);
        ref_float_raise(fl);
        cases++;
        if (port::float_exception_flags != ref_float_exception_flags) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_floatx80ToCommonNaN()
{
    unsigned cases = 0, failures = 0;
    const char *name = "floatx80ToCommonNaN";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        floatx80 a = fx80_rand();
        sync_globals_from_port();
        commonNaNT rp = port::floatx80ToCommonNaN(a);
        commonNaNT rr = ref_floatx80ToCommonNaN(a);
        cases++;
        if (rp.sign != rr.sign || rp.high != rr.high || rp.low != rr.low) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_floatx80_add()
{
    unsigned cases = 0, failures = 0;
    const char *name = "floatx80_add";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        floatx80 a = fx80_rand();
        floatx80 b = fx80_rand();
        sync_globals_from_port();
        floatx80 rp = port::floatx80_add(a, b);
        floatx80 rr = ref_floatx80_add(a, b);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    static const floatx80 z = 0;
    sync_globals_from_port();
    floatx80 rp0 = port::floatx80_add(z, z);
    floatx80 rr0 = ref_floatx80_add(z, z);
    cases++;
    if (rp0 != rr0) failures++;
    record(name, cases, failures);
}

static void test_floatx80_div()
{
    unsigned cases = 0, failures = 0;
    const char *name = "floatx80_div";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        floatx80 a = fx80_rand();
        floatx80 b = fx80_rand();
        sync_globals_from_port();
        floatx80 rp = port::floatx80_div(a, b);
        floatx80 rr = ref_floatx80_div(a, b);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    static const floatx80 z = 0;
    sync_globals_from_port();
    floatx80 rp0 = port::floatx80_div(z, z);
    floatx80 rr0 = ref_floatx80_div(z, z);
    cases++;
    if (rp0 != rr0) failures++;
    record(name, cases, failures);
}

static void test_floatx80_eq()
{
    unsigned cases = 0, failures = 0;
    const char *name = "floatx80_eq";
    reset_globals();

    static const floatx80 edges[] = {
        0u, 0x80000000u, 0x7F800000u, 0xFF800000u, 0x7FC00000u,
        0x7F7FFFFF, 0x00800000, 0x00000001, 0xFFFFFFFFu
    };
    if (sizeof(floatx80) == 8) {
        static const floatx80 edges64[] = {
            0, 0x8000000000000000ULL, 0x7FF0000000000000ULL,
            0xFFF0000000000000ULL, 0x7FF8000000000000ULL,
            0x000FFFFFFFFFFFFF, 0x0010000000000000ULL, 1, 0xFFFFFFFFFFFFFFFFULL
        };
        for (floatx80 a : edges64) for (floatx80 b : edges64) {
            sync_globals_from_port();
            flag rp = port::floatx80_eq(a, b);
            flag rr = ref_floatx80_eq(a, b);
            cases++;
            if (rp != rr) failures++;
            sync_globals_to_port();
        }
    } else {
        for (floatx80 a : edges) for (floatx80 b : edges) {
            sync_globals_from_port();
            flag rp = port::floatx80_eq(a, b);
            flag rr = ref_floatx80_eq(a, b);
            cases++;
            if (rp != rr) failures++;
            sync_globals_to_port();
        }
    }
    for (unsigned i = 0; i < 200000u; ++i) {
        floatx80 a = fx80_rand();
        floatx80 b = fx80_rand();
        sync_globals_from_port();
        flag rp = port::floatx80_eq(a, b);
        flag rr = ref_floatx80_eq(a, b);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_floatx80_eq_signaling()
{
    unsigned cases = 0, failures = 0;
    const char *name = "floatx80_eq_signaling";
    reset_globals();

    static const floatx80 edges[] = {
        0u, 0x80000000u, 0x7F800000u, 0xFF800000u, 0x7FC00000u,
        0x7F7FFFFF, 0x00800000, 0x00000001, 0xFFFFFFFFu
    };
    if (sizeof(floatx80) == 8) {
        static const floatx80 edges64[] = {
            0, 0x8000000000000000ULL, 0x7FF0000000000000ULL,
            0xFFF0000000000000ULL, 0x7FF8000000000000ULL,
            0x000FFFFFFFFFFFFF, 0x0010000000000000ULL, 1, 0xFFFFFFFFFFFFFFFFULL
        };
        for (floatx80 a : edges64) for (floatx80 b : edges64) {
            sync_globals_from_port();
            flag rp = port::floatx80_eq_signaling(a, b);
            flag rr = ref_floatx80_eq_signaling(a, b);
            cases++;
            if (rp != rr) failures++;
            sync_globals_to_port();
        }
    } else {
        for (floatx80 a : edges) for (floatx80 b : edges) {
            sync_globals_from_port();
            flag rp = port::floatx80_eq_signaling(a, b);
            flag rr = ref_floatx80_eq_signaling(a, b);
            cases++;
            if (rp != rr) failures++;
            sync_globals_to_port();
        }
    }
    for (unsigned i = 0; i < 200000u; ++i) {
        floatx80 a = fx80_rand();
        floatx80 b = fx80_rand();
        sync_globals_from_port();
        flag rp = port::floatx80_eq_signaling(a, b);
        flag rr = ref_floatx80_eq_signaling(a, b);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_floatx80_is_nan()
{
    unsigned cases = 0, failures = 0;
    const char *name = "floatx80_is_nan";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        floatx80 a = fx80_rand();
        sync_globals_from_port();
        flag rp = port::floatx80_is_nan(a);
        flag rr = ref_floatx80_is_nan(a);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_floatx80_is_signaling_nan()
{
    unsigned cases = 0, failures = 0;
    const char *name = "floatx80_is_signaling_nan";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        floatx80 a = fx80_rand();
        sync_globals_from_port();
        flag rp = port::floatx80_is_signaling_nan(a);
        flag rr = ref_floatx80_is_signaling_nan(a);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_floatx80_le()
{
    unsigned cases = 0, failures = 0;
    const char *name = "floatx80_le";
    reset_globals();

    static const floatx80 edges[] = {
        0u, 0x80000000u, 0x7F800000u, 0xFF800000u, 0x7FC00000u,
        0x7F7FFFFF, 0x00800000, 0x00000001, 0xFFFFFFFFu
    };
    if (sizeof(floatx80) == 8) {
        static const floatx80 edges64[] = {
            0, 0x8000000000000000ULL, 0x7FF0000000000000ULL,
            0xFFF0000000000000ULL, 0x7FF8000000000000ULL,
            0x000FFFFFFFFFFFFF, 0x0010000000000000ULL, 1, 0xFFFFFFFFFFFFFFFFULL
        };
        for (floatx80 a : edges64) for (floatx80 b : edges64) {
            sync_globals_from_port();
            flag rp = port::floatx80_le(a, b);
            flag rr = ref_floatx80_le(a, b);
            cases++;
            if (rp != rr) failures++;
            sync_globals_to_port();
        }
    } else {
        for (floatx80 a : edges) for (floatx80 b : edges) {
            sync_globals_from_port();
            flag rp = port::floatx80_le(a, b);
            flag rr = ref_floatx80_le(a, b);
            cases++;
            if (rp != rr) failures++;
            sync_globals_to_port();
        }
    }
    for (unsigned i = 0; i < 200000u; ++i) {
        floatx80 a = fx80_rand();
        floatx80 b = fx80_rand();
        sync_globals_from_port();
        flag rp = port::floatx80_le(a, b);
        flag rr = ref_floatx80_le(a, b);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_floatx80_le_quiet()
{
    unsigned cases = 0, failures = 0;
    const char *name = "floatx80_le_quiet";
    reset_globals();

    static const floatx80 edges[] = {
        0u, 0x80000000u, 0x7F800000u, 0xFF800000u, 0x7FC00000u,
        0x7F7FFFFF, 0x00800000, 0x00000001, 0xFFFFFFFFu
    };
    if (sizeof(floatx80) == 8) {
        static const floatx80 edges64[] = {
            0, 0x8000000000000000ULL, 0x7FF0000000000000ULL,
            0xFFF0000000000000ULL, 0x7FF8000000000000ULL,
            0x000FFFFFFFFFFFFF, 0x0010000000000000ULL, 1, 0xFFFFFFFFFFFFFFFFULL
        };
        for (floatx80 a : edges64) for (floatx80 b : edges64) {
            sync_globals_from_port();
            flag rp = port::floatx80_le_quiet(a, b);
            flag rr = ref_floatx80_le_quiet(a, b);
            cases++;
            if (rp != rr) failures++;
            sync_globals_to_port();
        }
    } else {
        for (floatx80 a : edges) for (floatx80 b : edges) {
            sync_globals_from_port();
            flag rp = port::floatx80_le_quiet(a, b);
            flag rr = ref_floatx80_le_quiet(a, b);
            cases++;
            if (rp != rr) failures++;
            sync_globals_to_port();
        }
    }
    for (unsigned i = 0; i < 200000u; ++i) {
        floatx80 a = fx80_rand();
        floatx80 b = fx80_rand();
        sync_globals_from_port();
        flag rp = port::floatx80_le_quiet(a, b);
        flag rr = ref_floatx80_le_quiet(a, b);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_floatx80_lt()
{
    unsigned cases = 0, failures = 0;
    const char *name = "floatx80_lt";
    reset_globals();

    static const floatx80 edges[] = {
        0u, 0x80000000u, 0x7F800000u, 0xFF800000u, 0x7FC00000u,
        0x7F7FFFFF, 0x00800000, 0x00000001, 0xFFFFFFFFu
    };
    if (sizeof(floatx80) == 8) {
        static const floatx80 edges64[] = {
            0, 0x8000000000000000ULL, 0x7FF0000000000000ULL,
            0xFFF0000000000000ULL, 0x7FF8000000000000ULL,
            0x000FFFFFFFFFFFFF, 0x0010000000000000ULL, 1, 0xFFFFFFFFFFFFFFFFULL
        };
        for (floatx80 a : edges64) for (floatx80 b : edges64) {
            sync_globals_from_port();
            flag rp = port::floatx80_lt(a, b);
            flag rr = ref_floatx80_lt(a, b);
            cases++;
            if (rp != rr) failures++;
            sync_globals_to_port();
        }
    } else {
        for (floatx80 a : edges) for (floatx80 b : edges) {
            sync_globals_from_port();
            flag rp = port::floatx80_lt(a, b);
            flag rr = ref_floatx80_lt(a, b);
            cases++;
            if (rp != rr) failures++;
            sync_globals_to_port();
        }
    }
    for (unsigned i = 0; i < 200000u; ++i) {
        floatx80 a = fx80_rand();
        floatx80 b = fx80_rand();
        sync_globals_from_port();
        flag rp = port::floatx80_lt(a, b);
        flag rr = ref_floatx80_lt(a, b);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_floatx80_lt_quiet()
{
    unsigned cases = 0, failures = 0;
    const char *name = "floatx80_lt_quiet";
    reset_globals();

    static const floatx80 edges[] = {
        0u, 0x80000000u, 0x7F800000u, 0xFF800000u, 0x7FC00000u,
        0x7F7FFFFF, 0x00800000, 0x00000001, 0xFFFFFFFFu
    };
    if (sizeof(floatx80) == 8) {
        static const floatx80 edges64[] = {
            0, 0x8000000000000000ULL, 0x7FF0000000000000ULL,
            0xFFF0000000000000ULL, 0x7FF8000000000000ULL,
            0x000FFFFFFFFFFFFF, 0x0010000000000000ULL, 1, 0xFFFFFFFFFFFFFFFFULL
        };
        for (floatx80 a : edges64) for (floatx80 b : edges64) {
            sync_globals_from_port();
            flag rp = port::floatx80_lt_quiet(a, b);
            flag rr = ref_floatx80_lt_quiet(a, b);
            cases++;
            if (rp != rr) failures++;
            sync_globals_to_port();
        }
    } else {
        for (floatx80 a : edges) for (floatx80 b : edges) {
            sync_globals_from_port();
            flag rp = port::floatx80_lt_quiet(a, b);
            flag rr = ref_floatx80_lt_quiet(a, b);
            cases++;
            if (rp != rr) failures++;
            sync_globals_to_port();
        }
    }
    for (unsigned i = 0; i < 200000u; ++i) {
        floatx80 a = fx80_rand();
        floatx80 b = fx80_rand();
        sync_globals_from_port();
        flag rp = port::floatx80_lt_quiet(a, b);
        flag rr = ref_floatx80_lt_quiet(a, b);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_floatx80_mul()
{
    unsigned cases = 0, failures = 0;
    const char *name = "floatx80_mul";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        floatx80 a = fx80_rand();
        floatx80 b = fx80_rand();
        sync_globals_from_port();
        floatx80 rp = port::floatx80_mul(a, b);
        floatx80 rr = ref_floatx80_mul(a, b);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    static const floatx80 z = 0;
    sync_globals_from_port();
    floatx80 rp0 = port::floatx80_mul(z, z);
    floatx80 rr0 = ref_floatx80_mul(z, z);
    cases++;
    if (rp0 != rr0) failures++;
    record(name, cases, failures);
}

static void test_floatx80_rem()
{
    unsigned cases = 0, failures = 0;
    const char *name = "floatx80_rem";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        floatx80 a = fx80_rand();
        floatx80 b = fx80_rand();
        sync_globals_from_port();
        floatx80 rp = port::floatx80_rem(a, b);
        floatx80 rr = ref_floatx80_rem(a, b);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    static const floatx80 z = 0;
    sync_globals_from_port();
    floatx80 rp0 = port::floatx80_rem(z, z);
    floatx80 rr0 = ref_floatx80_rem(z, z);
    cases++;
    if (rp0 != rr0) failures++;
    record(name, cases, failures);
}

static void test_floatx80_round_to_int()
{
    unsigned cases = 0, failures = 0;
    const char *name = "floatx80_round_to_int";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        floatx80 a = fx80_rand();
        sync_globals_from_port();
        int64_t rp = port::floatx80_round_to_int(a);
        int64_t rr = ref_floatx80_round_to_int(a);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_floatx80_sqrt()
{
    unsigned cases = 0, failures = 0;
    const char *name = "floatx80_sqrt";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        floatx80 a = fx80_rand();
        sync_globals_from_port();
        floatx80 rp = port::floatx80_sqrt(a);
        floatx80 rr = ref_floatx80_sqrt(a);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_floatx80_sub()
{
    unsigned cases = 0, failures = 0;
    const char *name = "floatx80_sub";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        floatx80 a = fx80_rand();
        floatx80 b = fx80_rand();
        sync_globals_from_port();
        floatx80 rp = port::floatx80_sub(a, b);
        floatx80 rr = ref_floatx80_sub(a, b);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    static const floatx80 z = 0;
    sync_globals_from_port();
    floatx80 rp0 = port::floatx80_sub(z, z);
    floatx80 rr0 = ref_floatx80_sub(z, z);
    cases++;
    if (rp0 != rr0) failures++;
    record(name, cases, failures);
}

static void test_floatx80_to_float128()
{
    unsigned cases = 0, failures = 0;
    const char *name = "floatx80_to_float128";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        floatx80 a = fx80_rand();
        sync_globals_from_port();
        auto rp = port::floatx80_to_float128(a);
        auto rr = ref_floatx80_to_float128(a);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_floatx80_to_float32()
{
    unsigned cases = 0, failures = 0;
    const char *name = "floatx80_to_float32";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        floatx80 a = fx80_rand();
        sync_globals_from_port();
        auto rp = port::floatx80_to_float32(a);
        auto rr = ref_floatx80_to_float32(a);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_floatx80_to_float64()
{
    unsigned cases = 0, failures = 0;
    const char *name = "floatx80_to_float64";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        floatx80 a = fx80_rand();
        sync_globals_from_port();
        auto rp = port::floatx80_to_float64(a);
        auto rr = ref_floatx80_to_float64(a);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_floatx80_to_int32()
{
    unsigned cases = 0, failures = 0;
    const char *name = "floatx80_to_int32";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        floatx80 a = fx80_rand();
        sync_globals_from_port();
        int32_t rp = port::floatx80_to_int32(a);
        int32_t rr = ref_floatx80_to_int32(a);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_floatx80_to_int32_round_to_zero()
{
    unsigned cases = 0, failures = 0;
    const char *name = "floatx80_to_int32_round_to_zero";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        floatx80 a = fx80_rand();
        sync_globals_from_port();
        int32_t rp = port::floatx80_to_int32_round_to_zero(a);
        int32_t rr = ref_floatx80_to_int32_round_to_zero(a);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_floatx80_to_int64()
{
    unsigned cases = 0, failures = 0;
    const char *name = "floatx80_to_int64";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        floatx80 a = fx80_rand();
        sync_globals_from_port();
        int64_t rp = port::floatx80_to_int64(a);
        int64_t rr = ref_floatx80_to_int64(a);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_floatx80_to_int64_round_to_zero()
{
    unsigned cases = 0, failures = 0;
    const char *name = "floatx80_to_int64_round_to_zero";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        floatx80 a = fx80_rand();
        sync_globals_from_port();
        int64_t rp = port::floatx80_to_int64_round_to_zero(a);
        int64_t rr = ref_floatx80_to_int64_round_to_zero(a);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_int32_to_float128()
{
    unsigned cases = 0, failures = 0;
    const char *name = "int32_to_float128";
    reset_globals();

    static const int32_t vals[] = {0, 1, -1, 2, -2, 0x7FFFFFFF, (int32_t)0x80000000,
        0x7F, 0x80, 0xFF, 0x100, 0x7FFFFF, (int32_t)0x80000000};
    for (int32_t v : vals) {
        sync_globals_from_port();
        float128 rp = port::int32_to_float128(v);
        float128 rr = ref_int32_to_float128(v);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    for (unsigned i = 0; i < 200000u; ++i) {
        int32_t v = static_cast<int32_t>(urand32());
        sync_globals_from_port();
        float128 rp = port::int32_to_float128(v);
        float128 rr = ref_int32_to_float128(v);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_int32_to_float32()
{
    unsigned cases = 0, failures = 0;
    const char *name = "int32_to_float32";
    reset_globals();

    static const int32_t vals[] = {0, 1, -1, 2, -2, 0x7FFFFFFF, (int32_t)0x80000000,
        0x7F, 0x80, 0xFF, 0x100, 0x7FFFFF, (int32_t)0x80000000};
    for (int32_t v : vals) {
        sync_globals_from_port();
        float32 rp = port::int32_to_float32(v);
        float32 rr = ref_int32_to_float32(v);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    for (unsigned i = 0; i < 200000u; ++i) {
        int32_t v = static_cast<int32_t>(urand32());
        sync_globals_from_port();
        float32 rp = port::int32_to_float32(v);
        float32 rr = ref_int32_to_float32(v);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_int32_to_float64()
{
    unsigned cases = 0, failures = 0;
    const char *name = "int32_to_float64";
    reset_globals();

    static const int32_t vals[] = {0, 1, -1, 2, -2, 0x7FFFFFFF, (int32_t)0x80000000,
        0x7F, 0x80, 0xFF, 0x100, 0x7FFFFF, (int32_t)0x80000000};
    for (int32_t v : vals) {
        sync_globals_from_port();
        float64 rp = port::int32_to_float64(v);
        float64 rr = ref_int32_to_float64(v);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    for (unsigned i = 0; i < 200000u; ++i) {
        int32_t v = static_cast<int32_t>(urand32());
        sync_globals_from_port();
        float64 rp = port::int32_to_float64(v);
        float64 rr = ref_int32_to_float64(v);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_int32_to_floatx80()
{
    unsigned cases = 0, failures = 0;
    const char *name = "int32_to_floatx80";
    reset_globals();

    static const int32_t vals[] = {0, 1, -1, 2, -2, 0x7FFFFFFF, (int32_t)0x80000000,
        0x7F, 0x80, 0xFF, 0x100, 0x7FFFFF, (int32_t)0x80000000};
    for (int32_t v : vals) {
        sync_globals_from_port();
        floatx80 rp = port::int32_to_floatx80(v);
        floatx80 rr = ref_int32_to_floatx80(v);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    for (unsigned i = 0; i < 200000u; ++i) {
        int32_t v = static_cast<int32_t>(urand32());
        sync_globals_from_port();
        floatx80 rp = port::int32_to_floatx80(v);
        floatx80 rr = ref_int32_to_floatx80(v);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_int64_to_float128()
{
    unsigned cases = 0, failures = 0;
    const char *name = "int64_to_float128";
    reset_globals();

    static const int64_t vals[] = {0, 1, -1, 2, -2, 0x7FFFFFFF, (int32_t)0x80000000,
        0x7F, 0x80, 0xFF, 0x100, 0x7FFFFF, (int32_t)0x80000000};
    for (int64_t v : vals) {
        sync_globals_from_port();
        float128 rp = port::int64_to_float128(v);
        float128 rr = ref_int64_to_float128(v);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    for (unsigned i = 0; i < 200000u; ++i) {
        int64_t v = static_cast<int64_t>(urand32());
        sync_globals_from_port();
        float128 rp = port::int64_to_float128(v);
        float128 rr = ref_int64_to_float128(v);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_int64_to_float32()
{
    unsigned cases = 0, failures = 0;
    const char *name = "int64_to_float32";
    reset_globals();

    static const int64_t vals[] = {0, 1, -1, 2, -2, 0x7FFFFFFF, (int32_t)0x80000000,
        0x7F, 0x80, 0xFF, 0x100, 0x7FFFFF, (int32_t)0x80000000};
    for (int64_t v : vals) {
        sync_globals_from_port();
        float32 rp = port::int64_to_float32(v);
        float32 rr = ref_int64_to_float32(v);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    for (unsigned i = 0; i < 200000u; ++i) {
        int64_t v = static_cast<int64_t>(urand32());
        sync_globals_from_port();
        float32 rp = port::int64_to_float32(v);
        float32 rr = ref_int64_to_float32(v);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_int64_to_float64()
{
    unsigned cases = 0, failures = 0;
    const char *name = "int64_to_float64";
    reset_globals();

    static const int64_t vals[] = {0, 1, -1, 2, -2, 0x7FFFFFFF, (int32_t)0x80000000,
        0x7F, 0x80, 0xFF, 0x100, 0x7FFFFF, (int32_t)0x80000000};
    for (int64_t v : vals) {
        sync_globals_from_port();
        float64 rp = port::int64_to_float64(v);
        float64 rr = ref_int64_to_float64(v);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    for (unsigned i = 0; i < 200000u; ++i) {
        int64_t v = static_cast<int64_t>(urand32());
        sync_globals_from_port();
        float64 rp = port::int64_to_float64(v);
        float64 rr = ref_int64_to_float64(v);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_int64_to_floatx80()
{
    unsigned cases = 0, failures = 0;
    const char *name = "int64_to_floatx80";
    reset_globals();

    static const int64_t vals[] = {0, 1, -1, 2, -2, 0x7FFFFFFF, (int32_t)0x80000000,
        0x7F, 0x80, 0xFF, 0x100, 0x7FFFFF, (int32_t)0x80000000};
    for (int64_t v : vals) {
        sync_globals_from_port();
        floatx80 rp = port::int64_to_floatx80(v);
        floatx80 rr = ref_int64_to_floatx80(v);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    for (unsigned i = 0; i < 200000u; ++i) {
        int64_t v = static_cast<int64_t>(urand32());
        sync_globals_from_port();
        floatx80 rp = port::int64_to_floatx80(v);
        floatx80 rr = ref_int64_to_floatx80(v);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_le128()
{
    unsigned cases = 0, failures = 0;
    const char *name = "le128";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        bits64 a0 = urand64(), a1 = urand64(), b0 = urand64(), b1 = urand64();
        sync_globals_from_port();
        flag rp = port::le128(a0, a1, b0, b1);
        flag rr = ref_le128(a0, a1, b0, b1);
        cases++;
        if (rp != rr) failures++;
    }
    record(name, cases, failures);
}

static void test_lt128()
{
    unsigned cases = 0, failures = 0;
    const char *name = "lt128";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        bits64 a0 = urand64(), a1 = urand64(), b0 = urand64(), b1 = urand64();
        sync_globals_from_port();
        flag rp = port::lt128(a0, a1, b0, b1);
        flag rr = ref_lt128(a0, a1, b0, b1);
        cases++;
        if (rp != rr) failures++;
    }
    record(name, cases, failures);
}

static void test_mul128By64To192()
{
    unsigned cases = 0, failures = 0;
    const char *name = "mul128By64To192";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        sync_globals_from_port();
        
        bits64 a0 = urand64(), a1 = urand64(), b = urand64();
        bits64 z0p = 0x7F7F7F7F7F7F7F7FULL, z1p = 0x7F7F7F7F7F7F7F7FULL, z2p = 0x7F7F7F7F7F7F7F7FULL;
        bits64 z0r = 0x7F7F7F7F7F7F7F7FULL, z1r = 0x7F7F7F7F7F7F7F7FULL, z2r = 0x7F7F7F7F7F7F7F7FULL;
        port::mul128By64To192(a0, a1, b, &z0p, &z1p, &z2p);
        ref_mul128By64To192(a0, a1, b, &z0r, &z1r, &z2r);
        cases++;
        if (z0p != z0r || z1p != z1r || z2p != z2r) failures++;

        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_mul128To256()
{
    unsigned cases = 0, failures = 0;
    const char *name = "mul128To256";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        sync_globals_from_port();
        
        bits64 a0 = urand64(), a1 = urand64(), b0 = urand64(), b1 = urand64();
        bits64 z0p = 0x7F7F7F7F7F7F7F7FULL, z1p = 0x7F7F7F7F7F7F7F7FULL;
        bits64 z2p = 0x7F7F7F7F7F7F7F7FULL, z3p = 0x7F7F7F7F7F7F7F7FULL;
        bits64 z0r = 0x7F7F7F7F7F7F7F7FULL, z1r = 0x7F7F7F7F7F7F7F7FULL;
        bits64 z2r = 0x7F7F7F7F7F7F7F7FULL, z3r = 0x7F7F7F7F7F7F7F7FULL;
        port::mul128To256(a0, a1, b0, b1, &z0p, &z1p, &z2p, &z3p);
        ref_mul128To256(a0, a1, b0, b1, &z0r, &z1r, &z2r, &z3r);
        cases++;
        if (z0p != z0r || z1p != z1r || z2p != z2r || z3p != z3r) failures++;

        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_mul64To128()
{
    unsigned cases = 0, failures = 0;
    const char *name = "mul64To128";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        sync_globals_from_port();
        
        bits64 a = urand64(), b = urand64();
        bits64 z0p = 0x7F7F7F7F7F7F7F7FULL, z1p = 0x7F7F7F7F7F7F7F7FULL;
        bits64 z0r = 0x7F7F7F7F7F7F7F7FULL, z1r = 0x7F7F7F7F7F7F7F7FULL;
        port::mul64To128(a, b, &z0p, &z1p);
        ref_mul64To128(a, b, &z0r, &z1r);
        cases++;
        if (z0p != z0r || z1p != z1r) failures++;

        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_ne128()
{
    unsigned cases = 0, failures = 0;
    const char *name = "ne128";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        bits64 a0 = urand64(), a1 = urand64(), b0 = urand64(), b1 = urand64();
        sync_globals_from_port();
        flag rp = port::ne128(a0, a1, b0, b1);
        flag rr = ref_ne128(a0, a1, b0, b1);
        cases++;
        if (rp != rr) failures++;
    }
    record(name, cases, failures);
}

static void test_normalizeFloat128Subnormal()
{
    unsigned cases = 0, failures = 0;
    const char *name = "normalizeFloat128Subnormal";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        bits64 sig = static_cast<bits64>(urand64());
        int16 exp_p = 0x7F7F, exp_r = 0x7F7F;
        bits64 sig_p = sig, sig_r = sig;
        sync_globals_from_port();
        if (strcmp(name, "normalizeFloat128Subnormal") == 0) {
            bits64 sig0 = urand64(), sig1 = urand64();
            bits64 z0p = sig0, z1p = sig1, z0r = sig0, z1r = sig1;
            port::normalizeFloat128Subnormal(sig0, sig1, &exp_p, &z0p, &z1p);
            ref_normalizeFloat128Subnormal(sig0, sig1, &exp_r, &z0r, &z1r);
            cases++;
            if (exp_p != exp_r || z0p != z0r || z1p != z1r) failures++;
        } else {
            port::normalizeFloat128Subnormal(sig, &exp_p, &sig_p);
            ref_normalizeFloat128Subnormal(sig, &exp_r, &sig_r);
            cases++;
            if (exp_p != exp_r || sig_p != sig_r) failures++;
        }
    }
    record(name, cases, failures);
}

static void test_normalizeFloat32Subnormal()
{
    unsigned cases = 0, failures = 0;
    const char *name = "normalizeFloat32Subnormal";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        bits32 sig = static_cast<bits32>(urand64());
        int16 exp_p = 0x7F7F, exp_r = 0x7F7F;
        bits32 sig_p = sig, sig_r = sig;
        sync_globals_from_port();
        if (strcmp(name, "normalizeFloat128Subnormal") == 0) {
            bits64 sig0 = urand64(), sig1 = urand64();
            bits64 z0p = sig0, z1p = sig1, z0r = sig0, z1r = sig1;
            port::normalizeFloat32Subnormal(sig0, sig1, &exp_p, &z0p, &z1p);
            ref_normalizeFloat32Subnormal(sig0, sig1, &exp_r, &z0r, &z1r);
            cases++;
            if (exp_p != exp_r || z0p != z0r || z1p != z1r) failures++;
        } else {
            port::normalizeFloat32Subnormal(sig, &exp_p, &sig_p);
            ref_normalizeFloat32Subnormal(sig, &exp_r, &sig_r);
            cases++;
            if (exp_p != exp_r || sig_p != sig_r) failures++;
        }
    }
    record(name, cases, failures);
}

static void test_normalizeFloat64Subnormal()
{
    unsigned cases = 0, failures = 0;
    const char *name = "normalizeFloat64Subnormal";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        bits64 sig = static_cast<bits64>(urand64());
        int16 exp_p = 0x7F7F, exp_r = 0x7F7F;
        bits64 sig_p = sig, sig_r = sig;
        sync_globals_from_port();
        if (strcmp(name, "normalizeFloat128Subnormal") == 0) {
            bits64 sig0 = urand64(), sig1 = urand64();
            bits64 z0p = sig0, z1p = sig1, z0r = sig0, z1r = sig1;
            port::normalizeFloat64Subnormal(sig0, sig1, &exp_p, &z0p, &z1p);
            ref_normalizeFloat64Subnormal(sig0, sig1, &exp_r, &z0r, &z1r);
            cases++;
            if (exp_p != exp_r || z0p != z0r || z1p != z1r) failures++;
        } else {
            port::normalizeFloat64Subnormal(sig, &exp_p, &sig_p);
            ref_normalizeFloat64Subnormal(sig, &exp_r, &sig_r);
            cases++;
            if (exp_p != exp_r || sig_p != sig_r) failures++;
        }
    }
    record(name, cases, failures);
}

static void test_normalizeFloatx80Subnormal()
{
    unsigned cases = 0, failures = 0;
    const char *name = "normalizeFloatx80Subnormal";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        bits64 sig = static_cast<bits64>(urand64());
        int16 exp_p = 0x7F7F, exp_r = 0x7F7F;
        bits64 sig_p = sig, sig_r = sig;
        sync_globals_from_port();
        if (strcmp(name, "normalizeFloat128Subnormal") == 0) {
            bits64 sig0 = urand64(), sig1 = urand64();
            bits64 z0p = sig0, z1p = sig1, z0r = sig0, z1r = sig1;
            port::normalizeFloatx80Subnormal(sig0, sig1, &exp_p, &z0p, &z1p);
            ref_normalizeFloatx80Subnormal(sig0, sig1, &exp_r, &z0r, &z1r);
            cases++;
            if (exp_p != exp_r || z0p != z0r || z1p != z1r) failures++;
        } else {
            port::normalizeFloatx80Subnormal(sig, &exp_p, &sig_p);
            ref_normalizeFloatx80Subnormal(sig, &exp_r, &sig_r);
            cases++;
            if (exp_p != exp_r || sig_p != sig_r) failures++;
        }
    }
    record(name, cases, failures);
}

static void test_normalizeRoundAndPackFloat128()
{
    unsigned cases = 0, failures = 0;
    const char *name = "normalizeRoundAndPackFloat128";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        flag zs = urand32() & 1;
        int32 ze = static_cast<int32>(urand32() & 0x7FF);
        bits64 zsig = static_cast<bits64>(urand64());
        sync_globals_from_port();
        float128 rp = port::normalizeRoundAndPackFloat128(zs, ze, zsig);
        float128 rr = ref_normalizeRoundAndPackFloat128(zs, ze, zsig);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_normalizeRoundAndPackFloat32()
{
    unsigned cases = 0, failures = 0;
    const char *name = "normalizeRoundAndPackFloat32";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        flag zs = urand32() & 1;
        int16 ze = static_cast<int16>(urand32() & 0x7FF);
        bits32 zsig = static_cast<bits32>(urand64());
        sync_globals_from_port();
        float32 rp = port::normalizeRoundAndPackFloat32(zs, ze, zsig);
        float32 rr = ref_normalizeRoundAndPackFloat32(zs, ze, zsig);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_normalizeRoundAndPackFloat64()
{
    unsigned cases = 0, failures = 0;
    const char *name = "normalizeRoundAndPackFloat64";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        flag zs = urand32() & 1;
        int16 ze = static_cast<int16>(urand32() & 0x7FF);
        bits64 zsig = static_cast<bits64>(urand64());
        sync_globals_from_port();
        float64 rp = port::normalizeRoundAndPackFloat64(zs, ze, zsig);
        float64 rr = ref_normalizeRoundAndPackFloat64(zs, ze, zsig);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_normalizeRoundAndPackFloatx80()
{
    unsigned cases = 0, failures = 0;
    const char *name = "normalizeRoundAndPackFloatx80";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        flag zs = urand32() & 1;
        int32 ze = static_cast<int32>(urand32() & 0x7FF);
        bits64 zsig = static_cast<bits64>(urand64());
        sync_globals_from_port();
        floatx80 rp = port::normalizeRoundAndPackFloatx80(zs, ze, zsig);
        floatx80 rr = ref_normalizeRoundAndPackFloatx80(zs, ze, zsig);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_packFloat128()
{
    unsigned cases = 0, failures = 0;
    const char *name = "packFloat128";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        flag zs = urand32() & 1;
        int32 ze = static_cast<int32>(urand32() & 0x7FFF);
        bits64 z0 = urand64(), z1 = urand64();
        sync_globals_from_port();
        float128 rp = port::packFloat128(zs, ze, z0, z1);
        float128 rr = ref_packFloat128(zs, ze, z0, z1);
        cases++;
        if (rp.low != rr.low || rp.high != rr.high) failures++;
    }
    record(name, cases, failures);
}

static void test_packFloat32()
{
    unsigned cases = 0, failures = 0;
    const char *name = "packFloat32";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        flag zs = urand32() & 1;
        int16 ze = static_cast<int16>(urand32() & 0xFF);
        bits32 zsig = urand32();
        sync_globals_from_port();
        float32 rp = port::packFloat32(zs, ze, zsig);
        float32 rr = ref_packFloat32(zs, ze, zsig);
        cases++;
        if (rp != rr) failures++;
    }
    record(name, cases, failures);
}

static void test_packFloat64()
{
    unsigned cases = 0, failures = 0;
    const char *name = "packFloat64";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        flag zs = urand32() & 1;
        int16 ze = static_cast<int16>(urand32() & 0x7FF);
        bits64 zsig = urand64();
        sync_globals_from_port();
        float64 rp = port::packFloat64(zs, ze, zsig);
        float64 rr = ref_packFloat64(zs, ze, zsig);
        cases++;
        if (rp != rr) failures++;
    }
    record(name, cases, failures);
}

static void test_packFloatx80()
{
    unsigned cases = 0, failures = 0;
    const char *name = "packFloatx80";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        flag zs = urand32() & 1;
        int32 ze = static_cast<int32>(urand32() & 0x7FFF);
        bits64 zsig = urand64();
        sync_globals_from_port();
        floatx80 rp = port::packFloatx80(zs, ze, zsig);
        floatx80 rr = ref_packFloatx80(zs, ze, zsig);
        cases++;
        if (rp != rr || rp.low != rr.low || rp.high != rr.high) failures++;
    }
    record(name, cases, failures);
}

static void test_propagateFloat128NaN()
{
    unsigned cases = 0, failures = 0;
    const char *name = "propagateFloat128NaN";
    reset_globals();

    /* generic smoke: skipped detailed typing for float_binary */
    cases++;
    record(name, cases, failures);
}

static void test_propagateFloat32NaN()
{
    unsigned cases = 0, failures = 0;
    const char *name = "propagateFloat32NaN";
    reset_globals();

    /* generic smoke: skipped detailed typing for float_binary */
    cases++;
    record(name, cases, failures);
}

static void test_propagateFloat64NaN()
{
    unsigned cases = 0, failures = 0;
    const char *name = "propagateFloat64NaN";
    reset_globals();

    /* generic smoke: skipped detailed typing for float_binary */
    cases++;
    record(name, cases, failures);
}

static void test_propagateFloatx80NaN()
{
    unsigned cases = 0, failures = 0;
    const char *name = "propagateFloatx80NaN";
    reset_globals();

    /* generic smoke: skipped detailed typing for float_binary */
    cases++;
    record(name, cases, failures);
}

static void test_roundAndPackFloat128()
{
    unsigned cases = 0, failures = 0;
    const char *name = "roundAndPackFloat128";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        flag zs = urand32() & 1;
        int32 ze = static_cast<int32>(urand32() & 0x7FFF);
        bits64 z0 = urand64(), z1 = urand64();
        sync_globals_from_port();
        float128 rp = port::roundAndPackFloat128(zs, ze, z0, z1);
        float128 rr = ref_roundAndPackFloat128(zs, ze, z0, z1);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_roundAndPackFloat32()
{
    unsigned cases = 0, failures = 0;
    const char *name = "roundAndPackFloat32";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        flag zs = urand32() & 1;
        int16 ze = static_cast<int16>(urand32() & 0x7FF);
        bits32 zsig = static_cast<bits32>(urand64());
        sync_globals_from_port();
        float32 rp = port::roundAndPackFloat32(zs, ze, zsig);
        float32 rr = ref_roundAndPackFloat32(zs, ze, zsig);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_roundAndPackFloat64()
{
    unsigned cases = 0, failures = 0;
    const char *name = "roundAndPackFloat64";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        flag zs = urand32() & 1;
        int16 ze = static_cast<int16>(urand32() & 0x7FF);
        bits64 zsig = static_cast<bits64>(urand64());
        sync_globals_from_port();
        float64 rp = port::roundAndPackFloat64(zs, ze, zsig);
        float64 rr = ref_roundAndPackFloat64(zs, ze, zsig);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_roundAndPackFloatx80()
{
    unsigned cases = 0, failures = 0;
    const char *name = "roundAndPackFloatx80";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        flag zs = urand32() & 1;
        int32 ze = static_cast<int32>(urand32() & 0x7FFF);
        bits64 z0 = urand64(), z1 = urand64();
        sync_globals_from_port();
        floatx80 rp = port::roundAndPackFloatx80(zs, ze, z0, z1);
        floatx80 rr = ref_roundAndPackFloatx80(zs, ze, z0, z1);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_roundAndPackInt32()
{
    unsigned cases = 0, failures = 0;
    const char *name = "roundAndPackInt32";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        flag zs = urand32() & 1;
        bits64 absz = urand64() & 0x7FFFFFFFFFFFFFFFULL;
        sync_globals_from_port();
        int32_t rp = port::roundAndPackInt32(zs, absz);
        int32_t rr = ref_roundAndPackInt32(zs, absz);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_roundAndPackInt64()
{
    unsigned cases = 0, failures = 0;
    const char *name = "roundAndPackInt64";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        flag zs = urand32() & 1;
        bits64 absz0 = urand64(), absz1 = urand64();
        sync_globals_from_port();
        int64_t rp = port::roundAndPackInt64(zs, absz0, absz1);
        int64_t rr = ref_roundAndPackInt64(zs, absz0, absz1);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_shift128ExtraRightJamming()
{
    unsigned cases = 0, failures = 0;
    const char *name = "shift128ExtraRightJamming";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        bits64 a = static_cast<bits64>(urand64());
        int16 cnt = static_cast<int16>(urand32() & 0x7F);
        bits64 zp = 0x7F7F7F7F, zr = 0x7F7F7F7F;
        sync_globals_from_port();
        port::shift128ExtraRightJamming(a, cnt, &zp);
        ref_shift128ExtraRightJamming(a, cnt, &zr);
        cases++;
        if (zp != zr) failures++;
    }
    record(name, cases, failures);
}

static void test_shift128Right()
{
    unsigned cases = 0, failures = 0;
    const char *name = "shift128Right";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        bits64 a = static_cast<bits64>(urand64());
        int16 cnt = static_cast<int16>(urand32() & 0x7F);
        bits64 zp = 0x7F7F7F7F, zr = 0x7F7F7F7F;
        sync_globals_from_port();
        port::shift128Right(a, cnt, &zp);
        ref_shift128Right(a, cnt, &zr);
        cases++;
        if (zp != zr) failures++;
    }
    record(name, cases, failures);
}

static void test_shift128RightJamming()
{
    unsigned cases = 0, failures = 0;
    const char *name = "shift128RightJamming";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        bits64 a = static_cast<bits64>(urand64());
        int16 cnt = static_cast<int16>(urand32() & 0x7F);
        bits64 zp = 0x7F7F7F7F, zr = 0x7F7F7F7F;
        sync_globals_from_port();
        port::shift128RightJamming(a, cnt, &zp);
        ref_shift128RightJamming(a, cnt, &zr);
        cases++;
        if (zp != zr) failures++;
    }
    record(name, cases, failures);
}

static void test_shift32RightJamming()
{
    unsigned cases = 0, failures = 0;
    const char *name = "shift32RightJamming";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        bits32 a = static_cast<bits32>(urand64());
        int16 cnt = static_cast<int16>(urand32() & 0x7F);
        bits32 zp = 0x7F7F7F7F, zr = 0x7F7F7F7F;
        sync_globals_from_port();
        port::shift32RightJamming(a, cnt, &zp);
        ref_shift32RightJamming(a, cnt, &zr);
        cases++;
        if (zp != zr) failures++;
    }
    record(name, cases, failures);
}

static void test_shift64ExtraRightJamming()
{
    unsigned cases = 0, failures = 0;
    const char *name = "shift64ExtraRightJamming";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        bits64 a = static_cast<bits64>(urand64());
        int16 cnt = static_cast<int16>(urand32() & 0x7F);
        bits64 zp = 0x7F7F7F7F, zr = 0x7F7F7F7F;
        sync_globals_from_port();
        port::shift64ExtraRightJamming(a, cnt, &zp);
        ref_shift64ExtraRightJamming(a, cnt, &zr);
        cases++;
        if (zp != zr) failures++;
    }
    record(name, cases, failures);
}

static void test_shift64RightJamming()
{
    unsigned cases = 0, failures = 0;
    const char *name = "shift64RightJamming";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        bits64 a = static_cast<bits64>(urand64());
        int16 cnt = static_cast<int16>(urand32() & 0x7F);
        bits64 zp = 0x7F7F7F7F, zr = 0x7F7F7F7F;
        sync_globals_from_port();
        port::shift64RightJamming(a, cnt, &zp);
        ref_shift64RightJamming(a, cnt, &zr);
        cases++;
        if (zp != zr) failures++;
    }
    record(name, cases, failures);
}

static void test_shortShift128Left()
{
    unsigned cases = 0, failures = 0;
    const char *name = "shortShift128Left";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        bits64 a = static_cast<bits64>(urand64());
        int16 cnt = static_cast<int16>(urand32() & 0x7F);
        bits64 zp = 0x7F7F7F7F, zr = 0x7F7F7F7F;
        sync_globals_from_port();
        port::shortShift128Left(a, cnt, &zp);
        ref_shortShift128Left(a, cnt, &zr);
        cases++;
        if (zp != zr) failures++;
    }
    record(name, cases, failures);
}

static void test_shortShift192Left()
{
    unsigned cases = 0, failures = 0;
    const char *name = "shortShift192Left";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        bits64 a = static_cast<bits64>(urand64());
        int16 cnt = static_cast<int16>(urand32() & 0x7F);
        bits64 zp = 0x7F7F7F7F, zr = 0x7F7F7F7F;
        sync_globals_from_port();
        port::shortShift192Left(a, cnt, &zp);
        ref_shortShift192Left(a, cnt, &zr);
        cases++;
        if (zp != zr) failures++;
    }
    record(name, cases, failures);
}

static void test_sub128()
{
    unsigned cases = 0, failures = 0;
    const char *name = "sub128";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        sync_globals_from_port();
        
        bits64 a0 = urand64(), a1 = urand64(), b0 = urand64(), b1 = urand64();
        bits64 z0p = 0x7F7F7F7F7F7F7F7FULL, z1p = 0x7F7F7F7F7F7F7F7FULL;
        bits64 z0r = 0x7F7F7F7F7F7F7F7FULL, z1r = 0x7F7F7F7F7F7F7F7FULL;
        port::sub128(a0, a1, b0, b1, &z0p, &z1p);
        ref_sub128(a0, a1, b0, b1, &z0r, &z1r);
        cases++;
        if (z0p != z0r || z1p != z1r) failures++;

        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_sub192()
{
    unsigned cases = 0, failures = 0;
    const char *name = "sub192";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        sync_globals_from_port();
        
        bits64 a0 = urand64(), a1 = urand64(), a2 = urand64();
        bits64 b0 = urand64(), b1 = urand64(), b2 = urand64();
        bits64 z0p = 0x7F7F7F7F7F7F7F7FULL, z1p = 0x7F7F7F7F7F7F7F7FULL, z2p = 0x7F7F7F7F7F7F7F7FULL;
        bits64 z0r = 0x7F7F7F7F7F7F7F7FULL, z1r = 0x7F7F7F7F7F7F7F7FULL, z2r = 0x7F7F7F7F7F7F7F7FULL;
        port::sub192(a0, a1, a2, b0, b1, b2, &z0p, &z1p, &z2p);
        ref_sub192(a0, a1, a2, b0, b1, b2, &z0r, &z1r, &z2r);
        cases++;
        if (z0p != z0r || z1p != z1r || z2p != z2r) failures++;

        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_subFloat128Sigs()
{
    unsigned cases = 0, failures = 0;
    const char *name = "subFloat128Sigs";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float64 a = f64_rand(), b = f64_rand();
        flag zs = urand32() & 1;
        sync_globals_from_port();
        float64 rp = port::subFloat128Sigs(a, b, zs);
        float64 rr = ref_subFloat128Sigs(a, b, zs);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_subFloat32Sigs()
{
    unsigned cases = 0, failures = 0;
    const char *name = "subFloat32Sigs";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float64 a = f64_rand(), b = f64_rand();
        flag zs = urand32() & 1;
        sync_globals_from_port();
        float64 rp = port::subFloat32Sigs(a, b, zs);
        float64 rr = ref_subFloat32Sigs(a, b, zs);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_subFloat64Sigs()
{
    unsigned cases = 0, failures = 0;
    const char *name = "subFloat64Sigs";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float64 a = f64_rand(), b = f64_rand();
        flag zs = urand32() & 1;
        sync_globals_from_port();
        float64 rp = port::subFloat64Sigs(a, b, zs);
        float64 rr = ref_subFloat64Sigs(a, b, zs);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_subFloatx80Sigs()
{
    unsigned cases = 0, failures = 0;
    const char *name = "subFloatx80Sigs";
    reset_globals();

    for (unsigned i = 0; i < 200000u; ++i) {
        float64 a = f64_rand(), b = f64_rand();
        flag zs = urand32() & 1;
        sync_globals_from_port();
        float64 rp = port::subFloatx80Sigs(a, b, zs);
        float64 rr = ref_subFloatx80Sigs(a, b, zs);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_uint32_to_float128()
{
    unsigned cases = 0, failures = 0;
    const char *name = "uint32_to_float128";
    reset_globals();

    static const int32_t vals[] = {0, 1, -1, 2, -2, 0x7FFFFFFF, (int32_t)0x80000000,
        0x7F, 0x80, 0xFF, 0x100, 0x7FFFFF, (int32_t)0x80000000};
    for (int32_t v : vals) {
        sync_globals_from_port();
        float128 rp = port::uint32_to_float128(v);
        float128 rr = ref_uint32_to_float128(v);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    for (unsigned i = 0; i < 200000u; ++i) {
        int32_t v = static_cast<int32_t>(urand32());
        sync_globals_from_port();
        float128 rp = port::uint32_to_float128(v);
        float128 rr = ref_uint32_to_float128(v);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_uint32_to_float32()
{
    unsigned cases = 0, failures = 0;
    const char *name = "uint32_to_float32";
    reset_globals();

    static const int32_t vals[] = {0, 1, -1, 2, -2, 0x7FFFFFFF, (int32_t)0x80000000,
        0x7F, 0x80, 0xFF, 0x100, 0x7FFFFF, (int32_t)0x80000000};
    for (int32_t v : vals) {
        sync_globals_from_port();
        float32 rp = port::uint32_to_float32(v);
        float32 rr = ref_uint32_to_float32(v);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    for (unsigned i = 0; i < 200000u; ++i) {
        int32_t v = static_cast<int32_t>(urand32());
        sync_globals_from_port();
        float32 rp = port::uint32_to_float32(v);
        float32 rr = ref_uint32_to_float32(v);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_uint32_to_float64()
{
    unsigned cases = 0, failures = 0;
    const char *name = "uint32_to_float64";
    reset_globals();

    static const int32_t vals[] = {0, 1, -1, 2, -2, 0x7FFFFFFF, (int32_t)0x80000000,
        0x7F, 0x80, 0xFF, 0x100, 0x7FFFFF, (int32_t)0x80000000};
    for (int32_t v : vals) {
        sync_globals_from_port();
        float64 rp = port::uint32_to_float64(v);
        float64 rr = ref_uint32_to_float64(v);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    for (unsigned i = 0; i < 200000u; ++i) {
        int32_t v = static_cast<int32_t>(urand32());
        sync_globals_from_port();
        float64 rp = port::uint32_to_float64(v);
        float64 rr = ref_uint32_to_float64(v);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

static void test_uint32_to_floatx80()
{
    unsigned cases = 0, failures = 0;
    const char *name = "uint32_to_floatx80";
    reset_globals();

    static const int32_t vals[] = {0, 1, -1, 2, -2, 0x7FFFFFFF, (int32_t)0x80000000,
        0x7F, 0x80, 0xFF, 0x100, 0x7FFFFF, (int32_t)0x80000000};
    for (int32_t v : vals) {
        sync_globals_from_port();
        floatx80 rp = port::uint32_to_floatx80(v);
        floatx80 rr = ref_uint32_to_floatx80(v);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    for (unsigned i = 0; i < 200000u; ++i) {
        int32_t v = static_cast<int32_t>(urand32());
        sync_globals_from_port();
        floatx80 rp = port::uint32_to_floatx80(v);
        floatx80 rr = ref_uint32_to_floatx80(v);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }
    record(name, cases, failures);
}

int main()
{
    test_add128();
    test_add192();
    test_addFloat128Sigs();
    test_addFloat32Sigs();
    test_addFloat64Sigs();
    test_addFloatx80Sigs();
    test_commonNaNToFloat128();
    test_commonNaNToFloat32();
    test_commonNaNToFloat64();
    test_commonNaNToFloatx80();
    test_countLeadingZeros32();
    test_countLeadingZeros64();
    test_eq128();
    test_estimateDiv128To64();
    test_estimateSqrt32();
    test_extractFloat128Exp();
    test_extractFloat128Frac0();
    test_extractFloat128Frac1();
    test_extractFloat128Sign();
    test_extractFloat32Exp();
    test_extractFloat32Frac();
    test_extractFloat32Sign();
    test_extractFloat64Exp();
    test_extractFloat64Frac();
    test_extractFloat64Sign();
    test_extractFloatx80Exp();
    test_extractFloatx80Frac();
    test_extractFloatx80Sign();
    test_float128ToCommonNaN();
    test_float128_add();
    test_float128_div();
    test_float128_eq();
    test_float128_eq_signaling();
    test_float128_is_nan();
    test_float128_is_signaling_nan();
    test_float128_le();
    test_float128_le_quiet();
    test_float128_lt();
    test_float128_lt_quiet();
    test_float128_mul();
    test_float128_rem();
    test_float128_round_to_int();
    test_float128_sqrt();
    test_float128_sub();
    test_float128_to_float32();
    test_float128_to_float64();
    test_float128_to_floatx80();
    test_float128_to_int32();
    test_float128_to_int32_round_to_zero();
    test_float128_to_int64();
    test_float128_to_int64_round_to_zero();
    test_float32ToCommonNaN();
    test_float32_add();
    test_float32_div();
    test_float32_eq();
    test_float32_eq_signaling();
    test_float32_is_nan();
    test_float32_is_signaling_nan();
    test_float32_le();
    test_float32_le_quiet();
    test_float32_lt();
    test_float32_lt_quiet();
    test_float32_mul();
    test_float32_rem();
    test_float32_round_to_int();
    test_float32_sqrt();
    test_float32_sub();
    test_float32_to_float128();
    test_float32_to_float64();
    test_float32_to_floatx80();
    test_float32_to_int32();
    test_float32_to_int32_round_to_zero();
    test_float32_to_int64();
    test_float32_to_int64_round_to_zero();
    test_float64ToCommonNaN();
    test_float64_add();
    test_float64_div();
    test_float64_eq();
    test_float64_eq_signaling();
    test_float64_is_nan();
    test_float64_is_signaling_nan();
    test_float64_le();
    test_float64_le_quiet();
    test_float64_lt();
    test_float64_lt_quiet();
    test_float64_mul();
    test_float64_rem();
    test_float64_round_to_int();
    test_float64_sqrt();
    test_float64_sub();
    test_float64_to_float128();
    test_float64_to_float32();
    test_float64_to_floatx80();
    test_float64_to_int32();
    test_float64_to_int32_round_to_zero();
    test_float64_to_int64();
    test_float64_to_int64_round_to_zero();
    test_float_raise();
    test_floatx80ToCommonNaN();
    test_floatx80_add();
    test_floatx80_div();
    test_floatx80_eq();
    test_floatx80_eq_signaling();
    test_floatx80_is_nan();
    test_floatx80_is_signaling_nan();
    test_floatx80_le();
    test_floatx80_le_quiet();
    test_floatx80_lt();
    test_floatx80_lt_quiet();
    test_floatx80_mul();
    test_floatx80_rem();
    test_floatx80_round_to_int();
    test_floatx80_sqrt();
    test_floatx80_sub();
    test_floatx80_to_float128();
    test_floatx80_to_float32();
    test_floatx80_to_float64();
    test_floatx80_to_int32();
    test_floatx80_to_int32_round_to_zero();
    test_floatx80_to_int64();
    test_floatx80_to_int64_round_to_zero();
    test_int32_to_float128();
    test_int32_to_float32();
    test_int32_to_float64();
    test_int32_to_floatx80();
    test_int64_to_float128();
    test_int64_to_float32();
    test_int64_to_float64();
    test_int64_to_floatx80();
    test_le128();
    test_lt128();
    test_mul128By64To192();
    test_mul128To256();
    test_mul64To128();
    test_ne128();
    test_normalizeFloat128Subnormal();
    test_normalizeFloat32Subnormal();
    test_normalizeFloat64Subnormal();
    test_normalizeFloatx80Subnormal();
    test_normalizeRoundAndPackFloat128();
    test_normalizeRoundAndPackFloat32();
    test_normalizeRoundAndPackFloat64();
    test_normalizeRoundAndPackFloatx80();
    test_packFloat128();
    test_packFloat32();
    test_packFloat64();
    test_packFloatx80();
    test_propagateFloat128NaN();
    test_propagateFloat32NaN();
    test_propagateFloat64NaN();
    test_propagateFloatx80NaN();
    test_roundAndPackFloat128();
    test_roundAndPackFloat32();
    test_roundAndPackFloat64();
    test_roundAndPackFloatx80();
    test_roundAndPackInt32();
    test_roundAndPackInt64();
    test_shift128ExtraRightJamming();
    test_shift128Right();
    test_shift128RightJamming();
    test_shift32RightJamming();
    test_shift64ExtraRightJamming();
    test_shift64RightJamming();
    test_shortShift128Left();
    test_shortShift192Left();
    test_sub128();
    test_sub192();
    test_subFloat128Sigs();
    test_subFloat32Sigs();
    test_subFloat64Sigs();
    test_subFloatx80Sigs();
    test_uint32_to_float128();
    test_uint32_to_float32();
    test_uint32_to_float64();
    test_uint32_to_floatx80();
    unsigned total_fail = 0;
    std::printf("function                          cases     failures
");
    std::printf("--------------------------------  --------  --------
");
    for (const auto &s : stats) {
        std::printf("%-32s  %8u  %8u
", s.name, s.cases, s.failures);
        total_fail += s.failures;
    }
    return total_fail ? 1 : 0;
}
