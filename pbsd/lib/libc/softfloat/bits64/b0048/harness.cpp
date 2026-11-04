// PBSD batch b0048 -- differential test: port.cppm vs oracle.c.
//
// Every function of the translation unit (175 of them, including the internal
// helpers, which oracle.c and port.cppm both expose) is called twice with
// identical arguments: once in the C++ module and once in the C oracle.  A
// case passes only if the return value AND the resulting float_exception_flags
// agree; for the void helpers that write through pointers, two 128-byte
// buffers pre-filled with the guard byte 0x7f are compared in their entirety,
// so a write outside the nominal output window fails just as loudly as a wrong
// value.
//
// SoftFloat takes no buffers and returns no pointers, so the "compare offsets
// from the buffer base" and "drive the stateful iterator to exhaustion" rules
// have no subject here; the analogue of the NUL-heavy / high-bit-byte edge
// cases is the table of hand-picked bit patterns below: zero, the smallest and
// largest subnormal, the smallest and largest normal, both sides of every
// exponent boundary the code tests (0x7E/0x7F, 0xFD/0xFE/0xFF, 0x3FE/0x3FF,
// 0x7FD/0x7FE/0x7FF, 0x403D/0x403E, 0x7FFD/0x7FFE/0x7FFF, ...), infinities,
// quiet and signalling NaNs, the two integer-conversion magic constants
// (0xCF000000, 0xC3E0000000000000), and the all-ones default NaNs -- each with
// both signs, and each crossed against every other value of its width.  Every
// such cross product is replayed under all four rounding modes, both tininess
// modes and all four floatx80 rounding precisions.

#include <cstdint>
#include <cstdio>
#include <cstring>

import pbsd.lib.libc.softfloat.bits64.b0048;

namespace P = pbsd::lib_libc_softfloat_bits64::b0048;

// The SoftFloat type names, so the oracle prototypes below can be spelled
// exactly as they are in oracle.c.
using flag = P::flag;
using int8 = P::int8;
using int16 = P::int16;
using int32 = P::int32;
using int64 = P::int64;
using uint32 = P::uint32;
using uint64 = P::uint64;
using bits32 = P::bits32;
using bits64 = P::bits64;
using float32 = P::float32;
using float64 = P::float64;
using floatx80 = P::floatx80;
using float128 = P::float128;
using commonNaNT = P::commonNaNT;

extern "C" {

extern int float_rounding_mode;
extern int float_exception_flags;
extern int float_detect_tininess;
extern int floatx80_rounding_precision;

void ref_shift32RightJamming( bits32 a, int16 count, bits32 *zPtr );
void ref_shift64RightJamming( bits64 a, int16 count, bits64 *zPtr );
void ref_shift64ExtraRightJamming( bits64 a0, bits64 a1, int16 count, bits64 *z0Ptr, bits64 *z1Ptr );
void ref_shift128Right( bits64 a0, bits64 a1, int16 count, bits64 *z0Ptr, bits64 *z1Ptr );
void ref_shift128RightJamming( bits64 a0, bits64 a1, int16 count, bits64 *z0Ptr, bits64 *z1Ptr );
void ref_shift128ExtraRightJamming( bits64 a0, bits64 a1, bits64 a2, int16 count, bits64 *z0Ptr, bits64 *z1Ptr, bits64 *z2Ptr );
void ref_shortShift128Left( bits64 a0, bits64 a1, int16 count, bits64 *z0Ptr, bits64 *z1Ptr );
void ref_shortShift192Left( bits64 a0, bits64 a1, bits64 a2, int16 count, bits64 *z0Ptr, bits64 *z1Ptr, bits64 *z2Ptr );
void ref_add128( bits64 a0, bits64 a1, bits64 b0, bits64 b1, bits64 *z0Ptr, bits64 *z1Ptr );
void ref_add192( bits64 a0, bits64 a1, bits64 a2, bits64 b0, bits64 b1, bits64 b2, bits64 *z0Ptr, bits64 *z1Ptr, bits64 *z2Ptr );
void ref_sub128( bits64 a0, bits64 a1, bits64 b0, bits64 b1, bits64 *z0Ptr, bits64 *z1Ptr );
void ref_sub192( bits64 a0, bits64 a1, bits64 a2, bits64 b0, bits64 b1, bits64 b2, bits64 *z0Ptr, bits64 *z1Ptr, bits64 *z2Ptr );
void ref_mul64To128( bits64 a, bits64 b, bits64 *z0Ptr, bits64 *z1Ptr );
void ref_mul128By64To192( bits64 a0, bits64 a1, bits64 b, bits64 *z0Ptr, bits64 *z1Ptr, bits64 *z2Ptr );
void ref_mul128To256( bits64 a0, bits64 a1, bits64 b0, bits64 b1, bits64 *z0Ptr, bits64 *z1Ptr, bits64 *z2Ptr, bits64 *z3Ptr );
bits64 ref_estimateDiv128To64( bits64 a0, bits64 a1, bits64 b );
bits32 ref_estimateSqrt32( int16 aExp, bits32 a );
int8 ref_countLeadingZeros32( bits32 a );
int8 ref_countLeadingZeros64( bits64 a );
flag ref_eq128( bits64 a0, bits64 a1, bits64 b0, bits64 b1 );
flag ref_le128( bits64 a0, bits64 a1, bits64 b0, bits64 b1 );
flag ref_lt128( bits64 a0, bits64 a1, bits64 b0, bits64 b1 );
flag ref_ne128( bits64 a0, bits64 a1, bits64 b0, bits64 b1 );
void ref_float_raise( int flags );
flag ref_float32_is_nan( float32 a );
flag ref_float32_is_signaling_nan( float32 a );
commonNaNT ref_float32ToCommonNaN( float32 a );
float32 ref_commonNaNToFloat32( commonNaNT a );
float32 ref_propagateFloat32NaN( float32 a, float32 b );
flag ref_float64_is_nan( float64 a );
flag ref_float64_is_signaling_nan( float64 a );
commonNaNT ref_float64ToCommonNaN( float64 a );
float64 ref_commonNaNToFloat64( commonNaNT a );
float64 ref_propagateFloat64NaN( float64 a, float64 b );
flag ref_floatx80_is_nan( floatx80 a );
flag ref_floatx80_is_signaling_nan( floatx80 a );
commonNaNT ref_floatx80ToCommonNaN( floatx80 a );
floatx80 ref_commonNaNToFloatx80( commonNaNT a );
floatx80 ref_propagateFloatx80NaN( floatx80 a, floatx80 b );
flag ref_float128_is_nan( float128 a );
flag ref_float128_is_signaling_nan( float128 a );
commonNaNT ref_float128ToCommonNaN( float128 a );
float128 ref_commonNaNToFloat128( commonNaNT a );
float128 ref_propagateFloat128NaN( float128 a, float128 b );
int32 ref_roundAndPackInt32( flag zSign, bits64 absZ );
int64 ref_roundAndPackInt64( flag zSign, bits64 absZ0, bits64 absZ1 );
bits32 ref_extractFloat32Frac( float32 a );
int16 ref_extractFloat32Exp( float32 a );
flag ref_extractFloat32Sign( float32 a );
void ref_normalizeFloat32Subnormal( bits32 aSig, int16 *zExpPtr, bits32 *zSigPtr );
float32 ref_packFloat32( flag zSign, int16 zExp, bits32 zSig );
float32 ref_roundAndPackFloat32( flag zSign, int16 zExp, bits32 zSig );
float32 ref_normalizeRoundAndPackFloat32( flag zSign, int16 zExp, bits32 zSig );
bits64 ref_extractFloat64Frac( float64 a );
int16 ref_extractFloat64Exp( float64 a );
flag ref_extractFloat64Sign( float64 a );
void ref_normalizeFloat64Subnormal( bits64 aSig, int16 *zExpPtr, bits64 *zSigPtr );
float64 ref_packFloat64( flag zSign, int16 zExp, bits64 zSig );
float64 ref_roundAndPackFloat64( flag zSign, int16 zExp, bits64 zSig );
float64 ref_normalizeRoundAndPackFloat64( flag zSign, int16 zExp, bits64 zSig );
bits64 ref_extractFloatx80Frac( floatx80 a );
int32 ref_extractFloatx80Exp( floatx80 a );
flag ref_extractFloatx80Sign( floatx80 a );
void ref_normalizeFloatx80Subnormal( bits64 aSig, int32 *zExpPtr, bits64 *zSigPtr );
floatx80 ref_packFloatx80( flag zSign, int32 zExp, bits64 zSig );
floatx80 ref_roundAndPackFloatx80( int8 roundingPrecision, flag zSign, int32 zExp, bits64 zSig0, bits64 zSig1 );
floatx80 ref_normalizeRoundAndPackFloatx80( int8 roundingPrecision, flag zSign, int32 zExp, bits64 zSig0, bits64 zSig1 );
bits64 ref_extractFloat128Frac1( float128 a );
bits64 ref_extractFloat128Frac0( float128 a );
int32 ref_extractFloat128Exp( float128 a );
flag ref_extractFloat128Sign( float128 a );
void ref_normalizeFloat128Subnormal( bits64 aSig0, bits64 aSig1, int32 *zExpPtr, bits64 *zSig0Ptr, bits64 *zSig1Ptr );
float128 ref_packFloat128( flag zSign, int32 zExp, bits64 zSig0, bits64 zSig1 );
float128 ref_roundAndPackFloat128( flag zSign, int32 zExp, bits64 zSig0, bits64 zSig1, bits64 zSig2 );
float128 ref_normalizeRoundAndPackFloat128( flag zSign, int32 zExp, bits64 zSig0, bits64 zSig1 );
float32 ref_int32_to_float32( int32 a );
float32 ref_uint32_to_float32( uint32 a );
float64 ref_int32_to_float64( int32 a );
float64 ref_uint32_to_float64( uint32 a );
floatx80 ref_int32_to_floatx80( int32 a );
floatx80 ref_uint32_to_floatx80( uint32 a );
float128 ref_int32_to_float128( int32 a );
float128 ref_uint32_to_float128( uint32 a );
float32 ref_int64_to_float32( int64 a );
float64 ref_int64_to_float64( int64 a );
floatx80 ref_int64_to_floatx80( int64 a );
float128 ref_int64_to_float128( int64 a );
int32 ref_float32_to_int32( float32 a );
int32 ref_float32_to_int32_round_to_zero( float32 a );
int64 ref_float32_to_int64( float32 a );
int64 ref_float32_to_int64_round_to_zero( float32 a );
float64 ref_float32_to_float64( float32 a );
floatx80 ref_float32_to_floatx80( float32 a );
float128 ref_float32_to_float128( float32 a );
float32 ref_float32_round_to_int( float32 a );
float32 ref_addFloat32Sigs( float32 a, float32 b, flag zSign );
float32 ref_subFloat32Sigs( float32 a, float32 b, flag zSign );
float32 ref_float32_add( float32 a, float32 b );
float32 ref_float32_sub( float32 a, float32 b );
float32 ref_float32_mul( float32 a, float32 b );
float32 ref_float32_div( float32 a, float32 b );
float32 ref_float32_rem( float32 a, float32 b );
float32 ref_float32_sqrt( float32 a );
flag ref_float32_eq( float32 a, float32 b );
flag ref_float32_le( float32 a, float32 b );
flag ref_float32_lt( float32 a, float32 b );
flag ref_float32_eq_signaling( float32 a, float32 b );
flag ref_float32_le_quiet( float32 a, float32 b );
flag ref_float32_lt_quiet( float32 a, float32 b );
int32 ref_float64_to_int32( float64 a );
int32 ref_float64_to_int32_round_to_zero( float64 a );
int64 ref_float64_to_int64( float64 a );
int64 ref_float64_to_int64_round_to_zero( float64 a );
float32 ref_float64_to_float32( float64 a );
floatx80 ref_float64_to_floatx80( float64 a );
float128 ref_float64_to_float128( float64 a );
float64 ref_float64_round_to_int( float64 a );
float64 ref_addFloat64Sigs( float64 a, float64 b, flag zSign );
float64 ref_subFloat64Sigs( float64 a, float64 b, flag zSign );
float64 ref_float64_add( float64 a, float64 b );
float64 ref_float64_sub( float64 a, float64 b );
float64 ref_float64_mul( float64 a, float64 b );
float64 ref_float64_div( float64 a, float64 b );
float64 ref_float64_rem( float64 a, float64 b );
float64 ref_float64_sqrt( float64 a );
flag ref_float64_eq( float64 a, float64 b );
flag ref_float64_le( float64 a, float64 b );
flag ref_float64_lt( float64 a, float64 b );
flag ref_float64_eq_signaling( float64 a, float64 b );
flag ref_float64_le_quiet( float64 a, float64 b );
flag ref_float64_lt_quiet( float64 a, float64 b );
int32 ref_floatx80_to_int32( floatx80 a );
int32 ref_floatx80_to_int32_round_to_zero( floatx80 a );
int64 ref_floatx80_to_int64( floatx80 a );
int64 ref_floatx80_to_int64_round_to_zero( floatx80 a );
float32 ref_floatx80_to_float32( floatx80 a );
float64 ref_floatx80_to_float64( floatx80 a );
float128 ref_floatx80_to_float128( floatx80 a );
floatx80 ref_floatx80_round_to_int( floatx80 a );
floatx80 ref_addFloatx80Sigs( floatx80 a, floatx80 b, flag zSign );
floatx80 ref_subFloatx80Sigs( floatx80 a, floatx80 b, flag zSign );
floatx80 ref_floatx80_add( floatx80 a, floatx80 b );
floatx80 ref_floatx80_sub( floatx80 a, floatx80 b );
floatx80 ref_floatx80_mul( floatx80 a, floatx80 b );
floatx80 ref_floatx80_div( floatx80 a, floatx80 b );
floatx80 ref_floatx80_rem( floatx80 a, floatx80 b );
floatx80 ref_floatx80_sqrt( floatx80 a );
flag ref_floatx80_eq( floatx80 a, floatx80 b );
flag ref_floatx80_le( floatx80 a, floatx80 b );
flag ref_floatx80_lt( floatx80 a, floatx80 b );
flag ref_floatx80_eq_signaling( floatx80 a, floatx80 b );
flag ref_floatx80_le_quiet( floatx80 a, floatx80 b );
flag ref_floatx80_lt_quiet( floatx80 a, floatx80 b );
int32 ref_float128_to_int32( float128 a );
int32 ref_float128_to_int32_round_to_zero( float128 a );
int64 ref_float128_to_int64( float128 a );
int64 ref_float128_to_int64_round_to_zero( float128 a );
float32 ref_float128_to_float32( float128 a );
float64 ref_float128_to_float64( float128 a );
floatx80 ref_float128_to_floatx80( float128 a );
float128 ref_float128_round_to_int( float128 a );
float128 ref_addFloat128Sigs( float128 a, float128 b, flag zSign );
float128 ref_subFloat128Sigs( float128 a, float128 b, flag zSign );
float128 ref_float128_add( float128 a, float128 b );
float128 ref_float128_sub( float128 a, float128 b );
float128 ref_float128_mul( float128 a, float128 b );
float128 ref_float128_div( float128 a, float128 b );
float128 ref_float128_rem( float128 a, float128 b );
float128 ref_float128_sqrt( float128 a );
flag ref_float128_eq( float128 a, float128 b );
flag ref_float128_le( float128 a, float128 b );
flag ref_float128_lt( float128 a, float128 b );
flag ref_float128_eq_signaling( float128 a, float128 b );
flag ref_float128_le_quiet( float128 a, float128 b );
flag ref_float128_lt_quiet( float128 a, float128 b );

} // extern "C"

// ---------------------------------------------------------------------------
// Bookkeeping
// ---------------------------------------------------------------------------

#define FN_LIST(X) \
    X(shift32RightJamming) X(shift64RightJamming) X(shift64ExtraRightJamming) \
    X(shift128Right) X(shift128RightJamming) X(shift128ExtraRightJamming) \
    X(shortShift128Left) X(shortShift192Left) \
    X(add128) X(add192) X(sub128) X(sub192) \
    X(mul64To128) X(mul128By64To192) X(mul128To256) \
    X(estimateDiv128To64) X(estimateSqrt32) \
    X(countLeadingZeros32) X(countLeadingZeros64) \
    X(eq128) X(le128) X(lt128) X(ne128) \
    X(float_raise) \
    X(float32_is_nan) X(float32_is_signaling_nan) X(float32ToCommonNaN) \
    X(commonNaNToFloat32) X(propagateFloat32NaN) \
    X(float64_is_nan) X(float64_is_signaling_nan) X(float64ToCommonNaN) \
    X(commonNaNToFloat64) X(propagateFloat64NaN) \
    X(floatx80_is_nan) X(floatx80_is_signaling_nan) X(floatx80ToCommonNaN) \
    X(commonNaNToFloatx80) X(propagateFloatx80NaN) \
    X(float128_is_nan) X(float128_is_signaling_nan) X(float128ToCommonNaN) \
    X(commonNaNToFloat128) X(propagateFloat128NaN) \
    X(roundAndPackInt32) X(roundAndPackInt64) \
    X(extractFloat32Frac) X(extractFloat32Exp) X(extractFloat32Sign) \
    X(normalizeFloat32Subnormal) X(packFloat32) X(roundAndPackFloat32) \
    X(normalizeRoundAndPackFloat32) \
    X(extractFloat64Frac) X(extractFloat64Exp) X(extractFloat64Sign) \
    X(normalizeFloat64Subnormal) X(packFloat64) X(roundAndPackFloat64) \
    X(normalizeRoundAndPackFloat64) \
    X(extractFloatx80Frac) X(extractFloatx80Exp) X(extractFloatx80Sign) \
    X(normalizeFloatx80Subnormal) X(packFloatx80) X(roundAndPackFloatx80) \
    X(normalizeRoundAndPackFloatx80) \
    X(extractFloat128Frac1) X(extractFloat128Frac0) X(extractFloat128Exp) \
    X(extractFloat128Sign) X(normalizeFloat128Subnormal) X(packFloat128) \
    X(roundAndPackFloat128) X(normalizeRoundAndPackFloat128) \
    X(int32_to_float32) X(uint32_to_float32) X(int32_to_float64) \
    X(uint32_to_float64) X(int32_to_floatx80) X(uint32_to_floatx80) \
    X(int32_to_float128) X(uint32_to_float128) \
    X(int64_to_float32) X(int64_to_float64) X(int64_to_floatx80) \
    X(int64_to_float128) \
    X(float32_to_int32) X(float32_to_int32_round_to_zero) \
    X(float32_to_int64) X(float32_to_int64_round_to_zero) \
    X(float32_to_float64) X(float32_to_floatx80) X(float32_to_float128) \
    X(float32_round_to_int) X(addFloat32Sigs) X(subFloat32Sigs) \
    X(float32_add) X(float32_sub) X(float32_mul) X(float32_div) \
    X(float32_rem) X(float32_sqrt) \
    X(float32_eq) X(float32_le) X(float32_lt) \
    X(float32_eq_signaling) X(float32_le_quiet) X(float32_lt_quiet) \
    X(float64_to_int32) X(float64_to_int32_round_to_zero) \
    X(float64_to_int64) X(float64_to_int64_round_to_zero) \
    X(float64_to_float32) X(float64_to_floatx80) X(float64_to_float128) \
    X(float64_round_to_int) X(addFloat64Sigs) X(subFloat64Sigs) \
    X(float64_add) X(float64_sub) X(float64_mul) X(float64_div) \
    X(float64_rem) X(float64_sqrt) \
    X(float64_eq) X(float64_le) X(float64_lt) \
    X(float64_eq_signaling) X(float64_le_quiet) X(float64_lt_quiet) \
    X(floatx80_to_int32) X(floatx80_to_int32_round_to_zero) \
    X(floatx80_to_int64) X(floatx80_to_int64_round_to_zero) \
    X(floatx80_to_float32) X(floatx80_to_float64) X(floatx80_to_float128) \
    X(floatx80_round_to_int) X(addFloatx80Sigs) X(subFloatx80Sigs) \
    X(floatx80_add) X(floatx80_sub) X(floatx80_mul) X(floatx80_div) \
    X(floatx80_rem) X(floatx80_sqrt) \
    X(floatx80_eq) X(floatx80_le) X(floatx80_lt) \
    X(floatx80_eq_signaling) X(floatx80_le_quiet) X(floatx80_lt_quiet) \
    X(float128_to_int32) X(float128_to_int32_round_to_zero) \
    X(float128_to_int64) X(float128_to_int64_round_to_zero) \
    X(float128_to_float32) X(float128_to_float64) X(float128_to_floatx80) \
    X(float128_round_to_int) X(addFloat128Sigs) X(subFloat128Sigs) \
    X(float128_add) X(float128_sub) X(float128_mul) X(float128_div) \
    X(float128_rem) X(float128_sqrt) \
    X(float128_eq) X(float128_le) X(float128_lt) \
    X(float128_eq_signaling) X(float128_le_quiet) X(float128_lt_quiet)

enum {
#define X(n) F_##n,
    FN_LIST(X)
#undef X
    F_COUNT
};

static const char *const fnNames[F_COUNT] = {
#define X(n) #n,
    FN_LIST(X)
#undef X
};

static long long g_cases[F_COUNT];
static long long g_fails[F_COUNT];

static inline void record(int id, bool ok)
{
    ++g_cases[id];
    if (!ok) {
        if (g_fails[id] == 0)
            std::fprintf(stderr, "first divergence in %s\n", fnNames[id]);
        ++g_fails[id];
    }
}

static inline void clearFlags()
{
    P::float_exception_flags = 0;
    ::float_exception_flags = 0;
}

static inline bool flagsOk()
{
    return P::float_exception_flags == ::float_exception_flags;
}

static void setState(int rm, int tininess, int prec)
{
    P::float_rounding_mode = rm;
    ::float_rounding_mode = rm;
    P::float_detect_tininess = tininess;
    ::float_detect_tininess = tininess;
    P::floatx80_rounding_precision = prec;
    ::floatx80_rounding_precision = prec;
}

static const int RM[4] = { 0, 1, 2, 3 };            // nearest_even, zero, down, up
static const int TIN[2] = { 0, 1 };                 // after / before rounding
static const int PREC[4] = { 80, 64, 32, 7 };       // 7 == "anything else" => 80

static void setStateIdx(unsigned k)
{
    setState(RM[k % 4], TIN[(k / 4) % 2], PREC[(k / 8) % 4]);
}

static const unsigned NSTATE = 32;

// Structural comparison of results.  floatx80 has padding, so its bytes are
// not compared -- only the fields the type actually has.
static inline bool same(const floatx80 &a, const floatx80 &b)
{
    return a.high == b.high && a.low == b.low;
}

static inline bool same(const float128 &a, const float128 &b)
{
    return a.high == b.high && a.low == b.low;
}

static inline bool same(const commonNaNT &a, const commonNaNT &b)
{
    return a.sign == b.sign && a.high == b.high && a.low == b.low;
}

template <class T> static inline bool same(T a, T b) { return a == b; }

// A guard-filled scratch buffer for the pointer-writing helpers.  Outputs go
// at offsets 16..55; everything else must still read 0x7f afterwards.
struct alignas(16) Buf {
    unsigned char raw[128];
    Buf() { std::memset(raw, 0x7f, sizeof raw); }
    template <class T> T *p(int off) { return reinterpret_cast<T *>(raw + off); }
    bool eq(const Buf &o) const { return std::memcmp(raw, o.raw, sizeof raw) == 0; }
};

#define CHK(fn, ...)                                                        \
    do {                                                                    \
        clearFlags();                                                       \
        auto pv_ = P::fn(__VA_ARGS__);                                      \
        auto rv_ = ref_##fn(__VA_ARGS__);                                   \
        record(F_##fn, same(pv_, rv_) && flagsOk());                        \
    } while (0)

#define BUF_BEGIN Buf bufA, bufB; clearFlags()
#define BUF_END(fn) record(F_##fn, bufA.eq(bufB) && flagsOk())

// ---------------------------------------------------------------------------
// Random number generation (fixed seed, splitmix64)
// ---------------------------------------------------------------------------

static std::uint64_t g_rng = 0x0123456789ABCDEFull;

static inline std::uint64_t rnd()
{
    std::uint64_t z = (g_rng += 0x9E3779B97F4A7C15ull);
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
    return z ^ (z >> 31);
}

static inline unsigned pick(unsigned n) { return (unsigned)(rnd() % n); }

// ---------------------------------------------------------------------------
// Hand-picked value tables
// ---------------------------------------------------------------------------

static const bits32 F32E[] = {
    0x00000000u, 0x80000000u,             // +-0
    0x00000001u, 0x80000001u,             // +-min subnormal
    0x00000002u, 0x00400000u,
    0x007FFFFFu, 0x807FFFFFu,             // +-max subnormal
    0x00800000u, 0x80800000u,             // +-min normal
    0x00800001u, 0x00FFFFFFu, 0x01000000u,
    0x33000000u,                          // 2^-89
    0x3EFFFFFFu,                          // just under 0.5
    0x3F000000u, 0xBF000000u,             // +-0.5
    0x3F7FFFFFu, 0xBF7FFFFFu,             // just under +-1
    0x3F800000u, 0xBF800000u,             // +-1
    0x3F800001u,
    0x4B000000u, 0x4B7FFFFFu, 0x4B800000u,// 2^23 boundary
    0x4C000000u,
    0x4E6E6B28u,                          // ~1e9
    0x4F000000u, 0xCF000000u,             // +-2^31 (0xCF000000 is special-cased)
    0x4EFFFFFFu, 0xCEFFFFFFu,
    0x5F000000u, 0xDF000000u,             // +-2^63 (0xDF000000 is special-cased)
    0x5EFFFFFFu, 0xDEFFFFFFu,
    0x7E800000u, 0x7F000000u,
    0x7F7FFFFEu, 0x7F7FFFFFu, 0xFF7FFFFFu,// +-max finite
    0x7F800000u, 0xFF800000u,             // +-inf
    0x7F800001u, 0xFF800001u,             // +-min signalling NaN
    0x7FBFFFFFu, 0xFFBFFFFFu,             // +-max signalling NaN
    0x7FC00000u, 0xFFC00000u,             // +-min quiet NaN
    0x7FFFFFFFu, 0xFFFFFFFFu              // default NaN pattern
};
static const unsigned NF32E = sizeof F32E / sizeof F32E[0];

static const bits64 F64E[] = {
    0x0000000000000000ull, 0x8000000000000000ull,
    0x0000000000000001ull, 0x8000000000000001ull,
    0x0000000000000002ull, 0x0004000000000000ull, 0x0008000000000000ull,
    0x000FFFFFFFFFFFFFull, 0x800FFFFFFFFFFFFFull,
    0x0010000000000000ull, 0x8010000000000000ull,
    0x0010000000000001ull, 0x001FFFFFFFFFFFFFull, 0x0020000000000000ull,
    0x3CA0000000000000ull,
    0x3FDFFFFFFFFFFFFFull,
    0x3FE0000000000000ull, 0xBFE0000000000000ull,
    0x3FEFFFFFFFFFFFFFull, 0xBFEFFFFFFFFFFFFFull,
    0x3FF0000000000000ull, 0xBFF0000000000000ull,
    0x3FF0000000000001ull,
    0x41E0000000000000ull, 0xC1E0000000000000ull,
    0x41DFFFFFFFFFFFFFull,
    0x4330000000000000ull, 0x433FFFFFFFFFFFFFull, 0x4340000000000000ull,
    0x43E0000000000000ull, 0xC3E0000000000000ull,
    0x43DFFFFFFFFFFFFFull, 0xC3DFFFFFFFFFFFFFull,
    0x7FE0000000000000ull,
    0x7FEFFFFFFFFFFFFFull, 0xFFEFFFFFFFFFFFFFull,
    0x7FF0000000000000ull, 0xFFF0000000000000ull,
    0x7FF0000000000001ull, 0xFFF0000000000001ull,
    0x7FF7FFFFFFFFFFFFull, 0xFFF7FFFFFFFFFFFFull,
    0x7FF8000000000000ull, 0xFFF8000000000000ull,
    0xFFFFFFFFFFFFFFFFull
};
static const unsigned NF64E = sizeof F64E / sizeof F64E[0];

struct X80Lit { unsigned high; bits64 low; };
static const X80Lit X80E[] = {
    { 0x0000, 0x0000000000000000ull }, { 0x8000, 0x0000000000000000ull },
    { 0x0000, 0x0000000000000001ull }, { 0x8000, 0x0000000000000001ull },
    { 0x0000, 0x4000000000000000ull }, { 0x0000, 0x8000000000000000ull },
    { 0x0001, 0x8000000000000000ull }, { 0x0001, 0x0000000000000001ull },
    { 0x3FFE, 0x8000000000000000ull }, { 0xBFFE, 0x8000000000000000ull },
    { 0x3FFE, 0xFFFFFFFFFFFFFFFFull },
    { 0x3FFF, 0x8000000000000000ull }, { 0xBFFF, 0x8000000000000000ull },
    { 0x3FFF, 0x8000000000000001ull }, { 0x3FFF, 0xFFFFFFFFFFFFFFFFull },
    { 0x4000, 0x8000000000000000ull },
    { 0x401E, 0x8000000000000000ull }, { 0xC01E, 0x8000000000000000ull },
    { 0x403D, 0xFFFFFFFFFFFFFFFFull },
    { 0x403E, 0x8000000000000000ull }, { 0xC03E, 0x8000000000000000ull },
    { 0x403F, 0x8000000000000000ull },
    { 0x7FFD, 0xFFFFFFFFFFFFFFFFull },
    { 0x7FFE, 0xFFFFFFFFFFFFFFFFull }, { 0xFFFE, 0xFFFFFFFFFFFFFFFFull },
    { 0x7FFF, 0x8000000000000000ull }, { 0xFFFF, 0x8000000000000000ull },
    { 0x7FFF, 0x8000000000000001ull }, { 0x7FFF, 0xA000000000000000ull },
    { 0x7FFF, 0xC000000000000000ull }, { 0xFFFF, 0xC000000000000000ull },
    { 0x7FFF, 0xFFFFFFFFFFFFFFFFull }
};
static const unsigned NX80E = sizeof X80E / sizeof X80E[0];

struct Q128Lit { bits64 high, low; };
static const Q128Lit Q128E[] = {
    { 0x0000000000000000ull, 0x0000000000000000ull },
    { 0x8000000000000000ull, 0x0000000000000000ull },
    { 0x0000000000000000ull, 0x0000000000000001ull },
    { 0x8000000000000000ull, 0x0000000000000001ull },
    { 0x0000000000000000ull, 0x8000000000000000ull },
    { 0x0000800000000000ull, 0x0000000000000000ull },
    { 0x0000FFFFFFFFFFFFull, 0xFFFFFFFFFFFFFFFFull },
    { 0x0001000000000000ull, 0x0000000000000000ull },
    { 0x0001000000000000ull, 0x0000000000000001ull },
    { 0x3FFE000000000000ull, 0x0000000000000000ull },
    { 0x3FFEFFFFFFFFFFFFull, 0xFFFFFFFFFFFFFFFFull },
    { 0x3FFF000000000000ull, 0x0000000000000000ull },
    { 0xBFFF000000000000ull, 0x0000000000000000ull },
    { 0x3FFF000000000000ull, 0x0000000000000001ull },
    { 0x4000000000000000ull, 0x0000000000000000ull },
    { 0x401E000000000000ull, 0x0000000000000000ull },
    { 0x402F000000000000ull, 0x0000000000000000ull },
    { 0x403E000000000000ull, 0x0000000000000000ull },
    { 0xC03E000000000000ull, 0x0000000000000000ull },
    { 0x406E000000000000ull, 0x0000000000000000ull },
    { 0x406F000000000000ull, 0x0000000000000000ull },
    { 0x7FFEFFFFFFFFFFFFull, 0xFFFFFFFFFFFFFFFFull },
    { 0xFFFEFFFFFFFFFFFFull, 0xFFFFFFFFFFFFFFFFull },
    { 0x7FFF000000000000ull, 0x0000000000000000ull },
    { 0xFFFF000000000000ull, 0x0000000000000000ull },
    { 0x7FFF000000000000ull, 0x0000000000000001ull },
    { 0x7FFF400000000000ull, 0x0000000000000000ull },
    { 0x7FFF800000000000ull, 0x0000000000000000ull },
    { 0xFFFF800000000000ull, 0x0000000000000000ull },
    { 0xFFFFFFFFFFFFFFFFull, 0xFFFFFFFFFFFFFFFFull }
};
static const unsigned NQ128E = sizeof Q128E / sizeof Q128E[0];

static const int16 CNTE[] = {
    0, 1, 2, 3, 4, 11, 12, 13, 14, 15, 16, 17, 30, 31, 32, 33, 34,
    49, 50, 52, 60, 61, 62, 63, 64, 65, 66, 79, 100, 126, 127, 128, 129, 130, 200
};
static const unsigned NCNTE = sizeof CNTE / sizeof CNTE[0];

static const bits64 U64E[] = {
    0x0000000000000000ull, 0x0000000000000001ull, 0x0000000000000002ull,
    0x0000000000000003ull, 0x0000000000000010ull,
    0x00000000FFFFFFFFull, 0x0000000100000000ull,
    0x0001000000000000ull, 0x0010000000000000ull,
    0x4000000000000000ull, 0x5555555555555555ull,
    0x7FFFFFFFFFFFFFFFull, 0x8000000000000000ull, 0x8000000000000001ull,
    0xAAAAAAAAAAAAAAAAull, 0xC000000000000000ull,
    0xFFFFFFFF00000000ull, 0xFFFFFFFFFFFFFFFEull, 0xFFFFFFFFFFFFFFFFull,
    0x123456789ABCDEF0ull
};
static const unsigned NU64E = sizeof U64E / sizeof U64E[0];

static const bits32 U32E[] = {
    0x00000000u, 0x00000001u, 0x00000002u, 0x00000003u,
    0x0000FFFFu, 0x00010000u, 0x007FFFFFu, 0x00800000u,
    0x40000000u, 0x55555555u, 0x7FFFFFFFu, 0x80000000u,
    0x80000001u, 0xAAAAAAAAu, 0xC0000000u, 0xFFFFFFFFu
};
static const unsigned NU32E = sizeof U32E / sizeof U32E[0];

static const int32 I32E[] = {
    0, 1, -1, 2, -2, 3, -3, 0x7FFFFFFF, (int32)0x80000000,
    0x40000000, -0x40000000, 1000000000, -1000000000, 0xFFFF, 0x800000,
    0x1000000, (int32)0x80000001
};
static const unsigned NI32E = sizeof I32E / sizeof I32E[0];

static const int64 I64E[] = {
    0, 1, -1, 2, -2, 0x7FFFFFFFFFFFFFFFll, (int64)0x8000000000000000ull,
    (int64)0x8000000000000001ull, 0x4000000000000000ll, -0x4000000000000000ll,
    1ll << 52, 1ll << 53, (1ll << 53) + 1, -((1ll << 53) + 1),
    0x00FFFFFFFFFFFFFFll, -0x00FFFFFFFFFFFFFFll, 0x7FFFFFFF, -0x80000000ll
};
static const unsigned NI64E = sizeof I64E / sizeof I64E[0];

static const int16 E16E[] = {
    -0x10000, -0x8000, -0x100, -2, -1, 0, 1, 2, 0x7E, 0x7F, 0x80,
    0xFC, 0xFD, 0xFE, 0xFF, 0x100, 0x3FE, 0x3FF, 0x7FC, 0x7FD, 0x7FE,
    0x7FF, 0x800, 0xFFFF, 0x10000, 0x100FD, 0x1FFFF
};
static const unsigned NE16E = sizeof E16E / sizeof E16E[0];

static const int32 E32E[] = {
    -0x10000, -0x100, -2, -1, 0, 1, 2, 0x3FFE, 0x3FFF, 0x4000,
    0x7FFC, 0x7FFD, 0x7FFE, 0x7FFF, 0x8000, 0x10000
};
static const unsigned NE32E = sizeof E32E / sizeof E32E[0];

static const int RAISEE[] = { 0, 1, 2, 3, 4, 8, 16, 31, 5, 12, 24, 7, 15, 32 };
static const unsigned NRAISEE = sizeof RAISEE / sizeof RAISEE[0];

static inline floatx80 mkx80(unsigned high, bits64 low)
{
    floatx80 z;
    z.high = (unsigned short)high;
    z.low = low;
    return z;
}

static inline float128 mkq128(bits64 high, bits64 low)
{
    float128 z;
    z.high = high;
    z.low = low;
    return z;
}

// floatx80_div and floatx80_sqrt require an explicit integer bit when the
// exponent is non-zero (that is what the format guarantees); given an
// "unnormal" instead, the original code spins in estimateDiv128To64's
// correction loop for 2^32 iterations.  Both sides get the same canonical
// value, so no comparison is weakened -- only the domain is respected.
static inline floatx80 canonx80(floatx80 a)
{
    if ((a.high & 0x7FFF) != 0)
        a.low |= 0x8000000000000000ull;
    return a;
}

// ---------------------------------------------------------------------------
// Per-group exercisers.  Between them these cover all 175 functions.
// ---------------------------------------------------------------------------

static void ex_prim(bits32 w, bits64 a0, bits64 a1, bits64 a2,
                    bits64 b0, bits64 b1, bits64 b2,
                    int16 cnt, int16 cnt64, int16 sqExp)
{
    {
        BUF_BEGIN;
        P::shift32RightJamming(w, cnt, bufA.p<bits32>(16));
        ref_shift32RightJamming(w, cnt, bufB.p<bits32>(16));
        BUF_END(shift32RightJamming);
    }
    {
        BUF_BEGIN;
        P::shift64RightJamming(a0, cnt, bufA.p<bits64>(16));
        ref_shift64RightJamming(a0, cnt, bufB.p<bits64>(16));
        BUF_END(shift64RightJamming);
    }
    {
        BUF_BEGIN;
        P::shift64ExtraRightJamming(a0, a1, cnt, bufA.p<bits64>(16), bufA.p<bits64>(24));
        ref_shift64ExtraRightJamming(a0, a1, cnt, bufB.p<bits64>(16), bufB.p<bits64>(24));
        BUF_END(shift64ExtraRightJamming);
    }
    {
        BUF_BEGIN;
        P::shift128Right(a0, a1, cnt, bufA.p<bits64>(16), bufA.p<bits64>(24));
        ref_shift128Right(a0, a1, cnt, bufB.p<bits64>(16), bufB.p<bits64>(24));
        BUF_END(shift128Right);
    }
    {
        BUF_BEGIN;
        P::shift128RightJamming(a0, a1, cnt, bufA.p<bits64>(16), bufA.p<bits64>(24));
        ref_shift128RightJamming(a0, a1, cnt, bufB.p<bits64>(16), bufB.p<bits64>(24));
        BUF_END(shift128RightJamming);
    }
    {
        BUF_BEGIN;
        P::shift128ExtraRightJamming(a0, a1, a2, cnt, bufA.p<bits64>(16),
                                     bufA.p<bits64>(24), bufA.p<bits64>(32));
        ref_shift128ExtraRightJamming(a0, a1, a2, cnt, bufB.p<bits64>(16),
                                      bufB.p<bits64>(24), bufB.p<bits64>(32));
        BUF_END(shift128ExtraRightJamming);
    }
    {
        BUF_BEGIN;
        P::shortShift128Left(a0, a1, cnt64, bufA.p<bits64>(16), bufA.p<bits64>(24));
        ref_shortShift128Left(a0, a1, cnt64, bufB.p<bits64>(16), bufB.p<bits64>(24));
        BUF_END(shortShift128Left);
    }
    {
        BUF_BEGIN;
        P::shortShift192Left(a0, a1, a2, cnt64, bufA.p<bits64>(16),
                             bufA.p<bits64>(24), bufA.p<bits64>(32));
        ref_shortShift192Left(a0, a1, a2, cnt64, bufB.p<bits64>(16),
                              bufB.p<bits64>(24), bufB.p<bits64>(32));
        BUF_END(shortShift192Left);
    }
    {
        BUF_BEGIN;
        P::add128(a0, a1, b0, b1, bufA.p<bits64>(16), bufA.p<bits64>(24));
        ref_add128(a0, a1, b0, b1, bufB.p<bits64>(16), bufB.p<bits64>(24));
        BUF_END(add128);
    }
    {
        BUF_BEGIN;
        P::add192(a0, a1, a2, b0, b1, b2, bufA.p<bits64>(16),
                  bufA.p<bits64>(24), bufA.p<bits64>(32));
        ref_add192(a0, a1, a2, b0, b1, b2, bufB.p<bits64>(16),
                   bufB.p<bits64>(24), bufB.p<bits64>(32));
        BUF_END(add192);
    }
    {
        BUF_BEGIN;
        P::sub128(a0, a1, b0, b1, bufA.p<bits64>(16), bufA.p<bits64>(24));
        ref_sub128(a0, a1, b0, b1, bufB.p<bits64>(16), bufB.p<bits64>(24));
        BUF_END(sub128);
    }
    {
        BUF_BEGIN;
        P::sub192(a0, a1, a2, b0, b1, b2, bufA.p<bits64>(16),
                  bufA.p<bits64>(24), bufA.p<bits64>(32));
        ref_sub192(a0, a1, a2, b0, b1, b2, bufB.p<bits64>(16),
                   bufB.p<bits64>(24), bufB.p<bits64>(32));
        BUF_END(sub192);
    }
    {
        BUF_BEGIN;
        P::mul64To128(a0, b0, bufA.p<bits64>(16), bufA.p<bits64>(24));
        ref_mul64To128(a0, b0, bufB.p<bits64>(16), bufB.p<bits64>(24));
        BUF_END(mul64To128);
    }
    {
        BUF_BEGIN;
        P::mul128By64To192(a0, a1, b0, bufA.p<bits64>(16),
                           bufA.p<bits64>(24), bufA.p<bits64>(32));
        ref_mul128By64To192(a0, a1, b0, bufB.p<bits64>(16),
                            bufB.p<bits64>(24), bufB.p<bits64>(32));
        BUF_END(mul128By64To192);
    }
    {
        BUF_BEGIN;
        P::mul128To256(a0, a1, b0, b1, bufA.p<bits64>(16), bufA.p<bits64>(24),
                       bufA.p<bits64>(32), bufA.p<bits64>(40));
        ref_mul128To256(a0, a1, b0, b1, bufB.p<bits64>(16), bufB.p<bits64>(24),
                        bufB.p<bits64>(32), bufB.p<bits64>(40));
        BUF_END(mul128To256);
    }

    // estimateDiv128To64 needs b >= 2^63.  The first call lets a0 land on
    // either side of b (the `b <= a0' early exit), the second forces the
    // long path through the correction loop.
    const bits64 bdiv = b0 | 0x8000000000000000ull;
    CHK(estimateDiv128To64, a0, a1, bdiv);
    CHK(estimateDiv128To64, a0 >> 1, a1, bdiv);

    // estimateSqrt32 needs a >= 2^31; both exponent parities matter.
    const bits32 asq = w | 0x80000000u;
    CHK(estimateSqrt32, sqExp, asq);
    CHK(estimateSqrt32, (int16)(sqExp ^ 1), asq);

    CHK(countLeadingZeros32, w);
    CHK(countLeadingZeros32, w >> 16);
    CHK(countLeadingZeros32, w >> 24);
    CHK(countLeadingZeros64, a0);
    CHK(countLeadingZeros64, a0 >> 32);

    CHK(eq128, a0, a1, b0, b1);
    CHK(eq128, a0, a1, a0, b1);
    CHK(le128, a0, a1, b0, b1);
    CHK(le128, a0, a1, a0, b1);
    CHK(le128, a0, a1, a0, a1);
    CHK(lt128, a0, a1, b0, b1);
    CHK(lt128, a0, a1, a0, b1);
    CHK(lt128, a0, a1, a0, a1);
    CHK(ne128, a0, a1, b0, b1);
    CHK(ne128, a0, a1, a0, b1);
}

static void ex_raise(int flags)
{
    CHK(float_raise, flags);
}

static void ex_common(flag sgn, bits64 high, bits64 low)
{
    commonNaNT n;
    n.sign = sgn;
    n.high = high;
    n.low = low;
    CHK(commonNaNToFloat32, n);
    CHK(commonNaNToFloat64, n);
    CHK(commonNaNToFloatx80, n);
    CHK(commonNaNToFloat128, n);
}

static void ex_f32(float32 a, float32 b, flag sgn)
{
    CHK(extractFloat32Frac, a);
    CHK(extractFloat32Exp, a);
    CHK(extractFloat32Sign, a);
    CHK(float32_is_nan, a);
    CHK(float32_is_signaling_nan, a);
    CHK(float32ToCommonNaN, a);
    CHK(propagateFloat32NaN, a, b);
    CHK(float32_to_int32, a);
    CHK(float32_to_int32_round_to_zero, a);
    CHK(float32_to_int64, a);
    CHK(float32_to_int64_round_to_zero, a);
    CHK(float32_to_float64, a);
    CHK(float32_to_floatx80, a);
    CHK(float32_to_float128, a);
    CHK(float32_round_to_int, a);
    CHK(addFloat32Sigs, a, b, sgn);
    CHK(subFloat32Sigs, a, b, sgn);
    CHK(float32_add, a, b);
    CHK(float32_sub, a, b);
    CHK(float32_mul, a, b);
    CHK(float32_div, a, b);
    CHK(float32_rem, a, b);
    CHK(float32_sqrt, a);
    CHK(float32_eq, a, b);
    CHK(float32_le, a, b);
    CHK(float32_lt, a, b);
    CHK(float32_eq_signaling, a, b);
    CHK(float32_le_quiet, a, b);
    CHK(float32_lt_quiet, a, b);
}

static void ex_f64(float64 a, float64 b, flag sgn)
{
    CHK(extractFloat64Frac, a);
    CHK(extractFloat64Exp, a);
    CHK(extractFloat64Sign, a);
    CHK(float64_is_nan, a);
    CHK(float64_is_signaling_nan, a);
    CHK(float64ToCommonNaN, a);
    CHK(propagateFloat64NaN, a, b);
    CHK(float64_to_int32, a);
    CHK(float64_to_int32_round_to_zero, a);
    CHK(float64_to_int64, a);
    CHK(float64_to_int64_round_to_zero, a);
    CHK(float64_to_float32, a);
    CHK(float64_to_floatx80, a);
    CHK(float64_to_float128, a);
    CHK(float64_round_to_int, a);
    CHK(addFloat64Sigs, a, b, sgn);
    CHK(subFloat64Sigs, a, b, sgn);
    CHK(float64_add, a, b);
    CHK(float64_sub, a, b);
    CHK(float64_mul, a, b);
    CHK(float64_div, a, b);
    CHK(float64_rem, a, b);
    CHK(float64_sqrt, a);
    CHK(float64_eq, a, b);
    CHK(float64_le, a, b);
    CHK(float64_lt, a, b);
    CHK(float64_eq_signaling, a, b);
    CHK(float64_le_quiet, a, b);
    CHK(float64_lt_quiet, a, b);
}

static void ex_x80(floatx80 a, floatx80 b, flag sgn)
{
    CHK(extractFloatx80Frac, a);
    CHK(extractFloatx80Exp, a);
    CHK(extractFloatx80Sign, a);
    CHK(floatx80_is_nan, a);
    CHK(floatx80_is_signaling_nan, a);
    CHK(floatx80ToCommonNaN, a);
    CHK(propagateFloatx80NaN, a, b);
    CHK(floatx80_to_int32, a);
    CHK(floatx80_to_int32_round_to_zero, a);
    CHK(floatx80_to_int64, a);
    CHK(floatx80_to_int64_round_to_zero, a);
    CHK(floatx80_to_float32, a);
    CHK(floatx80_to_float64, a);
    CHK(floatx80_to_float128, a);
    CHK(floatx80_round_to_int, a);
    CHK(addFloatx80Sigs, a, b, sgn);
    CHK(subFloatx80Sigs, a, b, sgn);
    CHK(floatx80_add, a, b);
    CHK(floatx80_sub, a, b);
    CHK(floatx80_mul, a, b);
    CHK(floatx80_rem, a, b);
    const floatx80 ca = canonx80(a), cb = canonx80(b);
    CHK(floatx80_div, ca, cb);
    CHK(floatx80_sqrt, ca);
    CHK(floatx80_eq, a, b);
    CHK(floatx80_le, a, b);
    CHK(floatx80_lt, a, b);
    CHK(floatx80_eq_signaling, a, b);
    CHK(floatx80_le_quiet, a, b);
    CHK(floatx80_lt_quiet, a, b);
}

static void ex_q128(float128 a, float128 b, flag sgn)
{
    CHK(extractFloat128Frac1, a);
    CHK(extractFloat128Frac0, a);
    CHK(extractFloat128Exp, a);
    CHK(extractFloat128Sign, a);
    CHK(float128_is_nan, a);
    CHK(float128_is_signaling_nan, a);
    CHK(float128ToCommonNaN, a);
    CHK(propagateFloat128NaN, a, b);
    CHK(float128_to_int32, a);
    CHK(float128_to_int32_round_to_zero, a);
    CHK(float128_to_int64, a);
    CHK(float128_to_int64_round_to_zero, a);
    CHK(float128_to_float32, a);
    CHK(float128_to_float64, a);
    CHK(float128_to_floatx80, a);
    CHK(float128_round_to_int, a);
    CHK(addFloat128Sigs, a, b, sgn);
    CHK(subFloat128Sigs, a, b, sgn);
    CHK(float128_add, a, b);
    CHK(float128_sub, a, b);
    CHK(float128_mul, a, b);
    CHK(float128_div, a, b);
    CHK(float128_rem, a, b);
    CHK(float128_sqrt, a);
    CHK(float128_eq, a, b);
    CHK(float128_le, a, b);
    CHK(float128_lt, a, b);
    CHK(float128_eq_signaling, a, b);
    CHK(float128_le_quiet, a, b);
    CHK(float128_lt_quiet, a, b);
}

static void ex_int(int32 i32, uint32 u32, int64 i64)
{
    CHK(int32_to_float32, i32);
    CHK(uint32_to_float32, u32);
    CHK(int32_to_float64, i32);
    CHK(uint32_to_float64, u32);
    CHK(int32_to_floatx80, i32);
    CHK(uint32_to_floatx80, u32);
    CHK(int32_to_float128, i32);
    CHK(uint32_to_float128, u32);
    CHK(int64_to_float32, i64);
    CHK(int64_to_float64, i64);
    CHK(int64_to_floatx80, i64);
    CHK(int64_to_float128, i64);
}

static void ex_pack(flag sgn, int16 e16, int32 e32, bits32 sig32,
                    bits64 s0, bits64 s1, bits64 s2, int8 prec)
{
    CHK(roundAndPackInt32, sgn, s0);
    CHK(roundAndPackInt32, sgn, s0 >> 1);
    CHK(roundAndPackInt64, sgn, s0, s1);

    {
        BUF_BEGIN;
        P::normalizeFloat32Subnormal(sig32 & 0x007FFFFFu, bufA.p<int16>(16),
                                     bufA.p<bits32>(24));
        ref_normalizeFloat32Subnormal(sig32 & 0x007FFFFFu, bufB.p<int16>(16),
                                      bufB.p<bits32>(24));
        BUF_END(normalizeFloat32Subnormal);
    }
    CHK(packFloat32, sgn, e16, sig32);
    CHK(roundAndPackFloat32, sgn, e16, sig32);
    // normalizeRoundAndPackFloat32 requires bit 31 of zSig to be clear.
    CHK(normalizeRoundAndPackFloat32, sgn, e16, sig32 & 0x7FFFFFFFu);

    {
        BUF_BEGIN;
        P::normalizeFloat64Subnormal(s0 & 0x000FFFFFFFFFFFFFull, bufA.p<int16>(16),
                                     bufA.p<bits64>(24));
        ref_normalizeFloat64Subnormal(s0 & 0x000FFFFFFFFFFFFFull, bufB.p<int16>(16),
                                      bufB.p<bits64>(24));
        BUF_END(normalizeFloat64Subnormal);
    }
    CHK(packFloat64, sgn, e16, s0);
    CHK(roundAndPackFloat64, sgn, e16, s0);
    // normalizeRoundAndPackFloat64 requires bit 63 of zSig to be clear.
    CHK(normalizeRoundAndPackFloat64, sgn, e16, s0 & 0x7FFFFFFFFFFFFFFFull);

    {
        BUF_BEGIN;
        const bits64 nz = s0 ? s0 : 1;  // the caller only ever passes non-zero
        P::normalizeFloatx80Subnormal(nz, bufA.p<int32>(16), bufA.p<bits64>(24));
        ref_normalizeFloatx80Subnormal(nz, bufB.p<int32>(16), bufB.p<bits64>(24));
        BUF_END(normalizeFloatx80Subnormal);
    }
    CHK(packFloatx80, sgn, e32, s0);
    CHK(roundAndPackFloatx80, prec, sgn, e32, s0, s1);
    // normalizeRoundAndPackFloatx80 requires a non-zero significand.
    CHK(normalizeRoundAndPackFloatx80, prec, sgn, e32, s0,
        (s0 | s1) ? s1 : (bits64)1);

    {
        BUF_BEGIN;
        // normalizeFloat128Subnormal is only reached with aSig0 < 2^48.
        const bits64 q0 = s0 & 0x0000FFFFFFFFFFFFull;
        P::normalizeFloat128Subnormal(q0, s1, bufA.p<int32>(16),
                                      bufA.p<bits64>(24), bufA.p<bits64>(32));
        ref_normalizeFloat128Subnormal(q0, s1, bufB.p<int32>(16),
                                       bufB.p<bits64>(24), bufB.p<bits64>(32));
        BUF_END(normalizeFloat128Subnormal);
    }
    CHK(packFloat128, sgn, e32, s0, s1);
    CHK(roundAndPackFloat128, sgn, e32, s0, s1, s2);
    CHK(normalizeRoundAndPackFloat128, sgn, e32, s0, s1);
}

// ---------------------------------------------------------------------------
// Phase 1: hand-written edge cases
// ---------------------------------------------------------------------------

static void edge_prim()
{
    unsigned k = 0;
    for (unsigned c = 0; c < NCNTE; ++c) {
        for (unsigned i = 0; i < NU64E; ++i) {
            for (unsigned j = 0; j < NU64E; ++j) {
                setStateIdx(k++);
                ex_prim((bits32)U64E[i], U64E[i], U64E[j], U64E[(i + j) % NU64E],
                        U64E[j], U64E[i], U64E[(i + 1) % NU64E],
                        CNTE[c], (int16)(CNTE[c] & 63),
                        (int16)(CNTE[c] + (int16)j));
            }
        }
    }
    // Explicitly walk every count 0..130 as well, so each boundary in the
    // shifters is hit from both sides with a value that has bits everywhere.
    for (int n = 0; n <= 130; ++n) {
        setStateIdx(k++);
        ex_prim(0xDEADBEEFu, 0x0123456789ABCDEFull, 0xFEDCBA9876543210ull,
                0x8000000000000001ull, 0x5555555555555555ull,
                0xAAAAAAAAAAAAAAAAull, 0x0000000000000001ull,
                (int16)n, (int16)(n & 63), (int16)n);
    }
}

static void edge_states()
{
    for (unsigned st = 0; st < NSTATE; ++st) {
        setStateIdx(st);

        for (unsigned i = 0; i < NF32E; ++i)
            for (unsigned j = 0; j < NF32E; ++j)
                ex_f32(F32E[i], F32E[j], (flag)((i + j) & 1));

        for (unsigned i = 0; i < NF64E; ++i)
            for (unsigned j = 0; j < NF64E; ++j)
                ex_f64(F64E[i], F64E[j], (flag)((i + j) & 1));

        for (unsigned i = 0; i < NX80E; ++i)
            for (unsigned j = 0; j < NX80E; ++j)
                ex_x80(mkx80(X80E[i].high, X80E[i].low),
                       mkx80(X80E[j].high, X80E[j].low), (flag)((i + j) & 1));

        for (unsigned i = 0; i < NQ128E; ++i)
            for (unsigned j = 0; j < NQ128E; ++j)
                ex_q128(mkq128(Q128E[i].high, Q128E[i].low),
                        mkq128(Q128E[j].high, Q128E[j].low), (flag)((i + j) & 1));

        for (unsigned e = 0; e < NE16E; ++e)
            for (unsigned i = 0; i < NU64E; ++i)
                for (unsigned s = 0; s < NU32E; ++s)
                    ex_pack((flag)((e + i + s) & 1), E16E[e],
                            E32E[(e + i) % NE32E], U32E[s], U64E[i],
                            U64E[(i + s) % NU64E], U64E[(i + e) % NU64E],
                            (int8)PREC[(e + s) % 4]);

        for (unsigned i = 0; i < NI32E; ++i)
            for (unsigned j = 0; j < NI64E; ++j)
                ex_int(I32E[i], U32E[(i + j) % NU32E], I64E[j]);

        for (unsigned i = 0; i < NRAISEE; ++i)
            ex_raise(RAISEE[i]);

        for (unsigned i = 0; i < NU64E; ++i)
            for (unsigned j = 0; j < NU64E; ++j)
                ex_common((flag)((i + j) & 1), U64E[i], U64E[j]);
    }
}

// ---------------------------------------------------------------------------
// Phase 2: fixed-seed randomised sweep
// ---------------------------------------------------------------------------

static const bits32 E32B[] = { 0, 1, 2, 0x7E, 0x7F, 0x80, 0x81, 0x95, 0x96,
                               0x9C, 0x9D, 0x9E, 0xBE, 0xBF, 0xFC, 0xFD,
                               0xFE, 0xFF };
static const bits32 S32B[] = { 0, 1, 2, 0x200000u, 0x3FFFFFu, 0x400000u,
                               0x400001u, 0x7FFFFEu, 0x7FFFFFu };
static const bits64 E64B[] = { 0, 1, 2, 0x3FE, 0x3FF, 0x400, 0x41E, 0x432,
                               0x433, 0x43C, 0x43D, 0x43E, 0x7FC, 0x7FD,
                               0x7FE, 0x7FF };
static const bits64 S64B[] = { 0, 1, 2, 0x4000000000000ull, 0x7FFFFFFFFFFFFull,
                               0x8000000000000ull, 0x8000000000001ull,
                               0xFFFFFFFFFFFFEull, 0xFFFFFFFFFFFFFull };
static const unsigned NE32B = sizeof E32B / sizeof E32B[0];
static const unsigned NS32B = sizeof S32B / sizeof S32B[0];
static const unsigned NE64B = sizeof E64B / sizeof E64B[0];
static const unsigned NS64B = sizeof S64B / sizeof S64B[0];

static bits64 rndU64()
{
    std::uint64_t r = rnd();
    switch (r & 7) {
    case 0: return U64E[pick(NU64E)];
    case 1: return rnd() & 0xFFFFull;
    case 2: return rnd() | 0x8000000000000000ull;
    case 3: return rnd() >> pick(64);
    case 4: return (bits64)0 - (rnd() & 0xFFull);
    case 5: return U64E[pick(NU64E)] ^ ((bits64)1 << pick(64));
    default: return rnd();
    }
}

static bits32 rndU32() { return (bits32)rndU64(); }

static float32 rndF32()
{
    std::uint64_t r = rnd();
    bits32 s = (bits32)((r >> 8) & 1) << 31;
    switch (r & 7) {
    case 0:
    case 1: return F32E[pick(NF32E)];
    case 2: return s | (E32B[pick(NE32B)] << 23) | (bits32)(rnd() & 0x7FFFFFu);
    case 3: return s | (((bits32)rnd() & 0xFFu) << 23) | S32B[pick(NS32B)];
    case 4: return s | (E32B[pick(NE32B)] << 23) | S32B[pick(NS32B)];
    case 5: return F32E[pick(NF32E)] ^ ((bits32)1 << pick(32));
    case 6: return (bits32)rnd() & 0x00FFFFFFu;
    default: return (bits32)rnd();
    }
}

static float64 rndF64()
{
    std::uint64_t r = rnd();
    bits64 s = ((r >> 8) & 1ull) << 63;
    switch (r & 7) {
    case 0:
    case 1: return F64E[pick(NF64E)];
    case 2: return s | (E64B[pick(NE64B)] << 52) | (rnd() & 0x000FFFFFFFFFFFFFull);
    case 3: return s | ((rnd() & 0x7FFull) << 52) | S64B[pick(NS64B)];
    case 4: return s | (E64B[pick(NE64B)] << 52) | S64B[pick(NS64B)];
    case 5: return F64E[pick(NF64E)] ^ (1ull << pick(64));
    case 6: return rnd() & 0x001FFFFFFFFFFFFFull;
    default: return rnd();
    }
}

static const unsigned EX80B[] = { 0, 1, 2, 0x3FFD, 0x3FFE, 0x3FFF, 0x4000,
                                  0x401E, 0x401F, 0x403D, 0x403E, 0x403F,
                                  0x7FFC, 0x7FFD, 0x7FFE, 0x7FFF };
static const bits64 LX80B[] = { 0, 1, 2, 0x4000000000000000ull,
                                0x7FFFFFFFFFFFFFFFull, 0x8000000000000000ull,
                                0x8000000000000001ull, 0xA000000000000000ull,
                                0xC000000000000000ull, 0xFFFFFFFFFFFFFFFEull,
                                0xFFFFFFFFFFFFFFFFull };
static const unsigned NEX80B = sizeof EX80B / sizeof EX80B[0];
static const unsigned NLX80B = sizeof LX80B / sizeof LX80B[0];

static floatx80 rndX80()
{
    std::uint64_t r = rnd();
    unsigned sign = (unsigned)((r >> 8) & 1) << 15;
    switch (r & 7) {
    case 0:
    case 1: return mkx80(X80E[pick(NX80E)].high, X80E[pick(NX80E)].low);
    case 2: return mkx80(sign | EX80B[pick(NEX80B)], rnd() | 0x8000000000000000ull);
    case 3: return mkx80(sign | EX80B[pick(NEX80B)], LX80B[pick(NLX80B)]);
    case 4: return mkx80(sign | (unsigned)(rnd() & 0x7FFF), LX80B[pick(NLX80B)]);
    case 5: {
        X80Lit v = X80E[pick(NX80E)];
        return mkx80(v.high ^ (unsigned)(rnd() & 3), v.low ^ (1ull << pick(64)));
    }
    case 6: return mkx80((unsigned)(rnd() & 0xFFFF), rnd());
    default: return mkx80(sign | EX80B[pick(NEX80B)], rnd());
    }
}

static const bits64 EQ128B[] = { 0, 1, 2, 0x3FFD, 0x3FFE, 0x3FFF, 0x4000,
                                 0x401E, 0x402F, 0x4030, 0x403E, 0x403F,
                                 0x406E, 0x406F, 0x7FFC, 0x7FFD, 0x7FFE,
                                 0x7FFF };
static const bits64 FQ128B[] = { 0, 1, 2, 0x0000800000000000ull,
                                 0x0000FFFFFFFFFFFEull, 0x0000FFFFFFFFFFFFull,
                                 0x0000400000000000ull };
static const unsigned NEQ128B = sizeof EQ128B / sizeof EQ128B[0];
static const unsigned NFQ128B = sizeof FQ128B / sizeof FQ128B[0];

static float128 rndQ128()
{
    std::uint64_t r = rnd();
    bits64 s = ((r >> 8) & 1ull) << 63;
    switch (r & 7) {
    case 0:
    case 1: return mkq128(Q128E[pick(NQ128E)].high, Q128E[pick(NQ128E)].low);
    case 2: return mkq128(s | (EQ128B[pick(NEQ128B)] << 48) |
                              (rnd() & 0x0000FFFFFFFFFFFFull), rnd());
    case 3: return mkq128(s | (EQ128B[pick(NEQ128B)] << 48) |
                              FQ128B[pick(NFQ128B)], rndU64());
    case 4: return mkq128(s | ((rnd() & 0x7FFFull) << 48) |
                              FQ128B[pick(NFQ128B)], rndU64());
    case 5: {
        Q128Lit v = Q128E[pick(NQ128E)];
        return mkq128(v.high ^ (1ull << pick(64)), v.low ^ (1ull << pick(64)));
    }
    case 6: return mkq128(rnd() & 0x0001FFFFFFFFFFFFull, rnd());
    default: return mkq128(rnd(), rnd());
    }
}

static const long RANDOM_ITERATIONS = 200000;

static void random_sweep()
{
    for (long it = 0; it < RANDOM_ITERATIONS; ++it) {
        setStateIdx((unsigned)rnd());

        ex_prim(rndU32(), rndU64(), rndU64(), rndU64(), rndU64(), rndU64(),
                rndU64(), (int16)(rnd() % 200), (int16)(rnd() % 64),
                (int16)(rnd() % 0x10000));

        ex_f32(rndF32(), rndF32(), (flag)(rnd() & 1));
        ex_f64(rndF64(), rndF64(), (flag)(rnd() & 1));
        ex_x80(rndX80(), rndX80(), (flag)(rnd() & 1));
        ex_q128(rndQ128(), rndQ128(), (flag)(rnd() & 1));

        ex_pack((flag)(rnd() & 1), E16E[pick(NE16E)], E32E[pick(NE32E)],
                rndU32(), rndU64(), rndU64(), rndU64(),
                (int8)PREC[pick(4)]);

        ex_int(I32E[pick(NI32E)], rndU32(), I64E[pick(NI64E)]);
        ex_raise(RAISEE[pick(NRAISEE)]);
        ex_common((flag)(rnd() & 1), rndU64(), rndU64());
    }
}

// ---------------------------------------------------------------------------

int main()
{
    edge_prim();
    edge_states();
    random_sweep();

    long long totalCases = 0, totalFails = 0;
    int untested = 0;

    std::printf("%-40s %14s %12s\n", "function", "cases", "failures");
    std::printf("--------------------------------------------------"
                "--------------------------------\n");
    for (int i = 0; i < F_COUNT; ++i) {
        std::printf("%-40s %14lld %12lld%s\n", fnNames[i], g_cases[i],
                    g_fails[i], g_fails[i] ? "   <== FAIL" : "");
        totalCases += g_cases[i];
        totalFails += g_fails[i];
        if (g_cases[i] == 0)
            ++untested;
    }
    std::printf("--------------------------------------------------"
                "--------------------------------\n");
    std::printf("%-40s %14lld %12lld\n", "TOTAL (175 functions)", totalCases,
                totalFails);
    std::printf("random iterations per function: %ld\n", RANDOM_ITERATIONS);

    if (untested) {
        std::printf("%d function(s) were never exercised\n", untested);
        return 1;
    }
    return totalFails ? 1 : 0;
}
