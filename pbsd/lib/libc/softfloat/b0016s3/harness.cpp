/*
 * harness.cpp -- differential test for PBSD batch b0016s3.
 *
 * Every case is run through both the C++23 port and the ref_ oracle built
 * from the original C, and the following are compared:
 *
 *   - the return value (exactly, as int; __getf2 returns -1/0 so a
 *     truncating or unsigned port shows up immediately);
 *   - the SoftFloat exception flags raised by the call (the NaN path of
 *     float128_le is only distinguishable from "false" by the flag);
 *   - the entire argument buffers.  No function in this batch writes
 *     through a pointer, so instead the two float128 operands are planted
 *     inside two 64-byte buffers pre-filled with the guard byte 0x7f; after
 *     the call both buffers must still be byte-identical to each other and
 *     to the pristine image, padding bytes included.
 *
 * Inputs are the full cross product of a hand-written edge table (zeroes of
 * both signs, denormals, minimum and maximum normals, both infinities,
 * quiet and signalling NaNs, values differing only in the last significand
 * bit, and equal values) followed by a fixed-seed randomised sweep.
 */

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

import pbsd.lib.libc.softfloat.b0016s3;

namespace P = pbsd::lib_libc_softfloat::b0016s3;

/*
 * The oracle's float128 (bits64 high; bits64 low).  Declared here with the
 * identical layout rather than sharing a header so that the two sides stay
 * genuinely independent.
 */
struct ref_float128 {
    unsigned long long high;
    unsigned long long low;
};

extern "C" {
int ref___getf2(ref_float128 a, ref_float128 b);
int ref_float128_le(ref_float128 a, ref_float128 b);
extern int ref_float_exception_flags;
extern int ref_float_exception_mask;
}

static_assert(sizeof(ref_float128) == sizeof(P::float128),
              "port and oracle float128 must have the same size");
static_assert(sizeof(ref_float128) == 16, "unexpected float128 layout");
static_assert(offsetof(ref_float128, high) == 0, "unexpected float128 layout");
static_assert(offsetof(ref_float128, low) == 8, "unexpected float128 layout");
static_assert(offsetof(P::float128, high) == 0, "unexpected float128 layout");
static_assert(offsetof(P::float128, low) == 8, "unexpected float128 layout");

/* ------------------------------------------------------------------ */

struct Val {
    std::uint64_t high;
    std::uint64_t low;
};

struct Stat {
    const char *name;
    long long cases;
    long long failures;
};

static Stat stats[] = {
    { "float128_le", 0, 0 },
    { "__getf2", 0, 0 },
};

enum { FN_LE = 0, FN_GE = 1 };

static int reported;

static void
report(int fn, const Val &a, const Val &b, const char *what,
       long long rr, long long rp)
{
    stats[fn].failures++;
    if (reported < 25) {
        reported++;
        std::printf("FAIL %-12s a={%016llx,%016llx} b={%016llx,%016llx} %s: "
                    "oracle=%lld port=%lld\n",
                    stats[fn].name,
                    (unsigned long long)a.high, (unsigned long long)a.low,
                    (unsigned long long)b.high, (unsigned long long)b.low,
                    what, rr, rp);
    }
}

/* ------------------------------------------------------------------ */
/* Guarded buffers.  The operands live at OFF_A / OFF_B inside them.   */

enum : std::size_t {
    BUFSZ = 64,
    OFF_A = 16,
    OFF_B = 32
};

static const unsigned char GUARD = 0x7f;

static void
plant(unsigned char *buf, std::size_t off, const Val &v)
{
    std::uint64_t h = v.high;
    std::uint64_t l = v.low;
    std::memcpy(buf + off + 0, &h, sizeof h);
    std::memcpy(buf + off + 8, &l, sizeof l);
}

/*
 * Run one (a, b) pair through both implementations of both functions.
 */
static void
check(const Val &a, const Val &b)
{
    unsigned char bufRef[BUFSZ], bufPort[BUFSZ], bufPristine[BUFSZ];

    std::memset(bufRef, GUARD, BUFSZ);
    std::memset(bufPort, GUARD, BUFSZ);
    plant(bufRef, OFF_A, a);
    plant(bufRef, OFF_B, b);
    plant(bufPort, OFF_A, a);
    plant(bufPort, OFF_B, b);
    std::memcpy(bufPristine, bufRef, BUFSZ);

    ref_float128 ra, rb;
    P::float128 pa, pb;
    std::memcpy(&ra, bufRef + OFF_A, sizeof ra);
    std::memcpy(&rb, bufRef + OFF_B, sizeof rb);
    std::memcpy(&pa, bufPort + OFF_A, sizeof pa);
    std::memcpy(&pb, bufPort + OFF_B, sizeof pb);

    for (int fn = FN_LE; fn <= FN_GE; fn++) {
        stats[fn].cases++;

        ref_float_exception_flags = 0;
        int rret = (fn == FN_LE) ? ref_float128_le(ra, rb)
                                 : ref___getf2(ra, rb);
        int rflags = ref_float_exception_flags;

        P::float_exception_flags = 0;
        int pret = (fn == FN_LE) ? P::float128_le(pa, pb)
                                 : P::getf2(pa, pb);
        int pflags = P::float_exception_flags;

        if (rret != pret)
            report(fn, a, b, "return", rret, pret);
        else if (rflags != pflags)
            report(fn, a, b, "exception flags", rflags, pflags);
        else if (std::memcmp(bufRef, bufPort, BUFSZ) != 0)
            report(fn, a, b, "buffers differ", 0, 0);
        else if (std::memcmp(bufRef, bufPristine, BUFSZ) != 0)
            report(fn, a, b, "oracle clobbered its buffer", 0, 0);
        else if (std::memcmp(bufPort, bufPristine, BUFSZ) != 0)
            report(fn, a, b, "port clobbered its buffer", 0, 0);
        else if (ref_float_exception_mask != 0 ||
                 P::float_exception_mask != 0)
            report(fn, a, b, "exception mask changed",
                   ref_float_exception_mask, P::float_exception_mask);
    }
}

/* ------------------------------------------------------------------ */
/* Hand-written edge values.                                          */

static const Val edge[] = {
    /* zeroes */
    { 0x0000000000000000ULL, 0x0000000000000000ULL },   /* +0 */
    { 0x8000000000000000ULL, 0x0000000000000000ULL },   /* -0 */
    /* denormals */
    { 0x0000000000000000ULL, 0x0000000000000001ULL },
    { 0x8000000000000000ULL, 0x0000000000000001ULL },
    { 0x0000000000000000ULL, 0x8000000000000000ULL },
    { 0x0000FFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL },
    { 0x8000FFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL },
    /* smallest normals */
    { 0x0001000000000000ULL, 0x0000000000000000ULL },
    { 0x8001000000000000ULL, 0x0000000000000000ULL },
    /* ordinary values */
    { 0x3FFE000000000000ULL, 0x0000000000000000ULL },   /* +0.5 */
    { 0xBFFE000000000000ULL, 0x0000000000000000ULL },   /* -0.5 */
    { 0x3FFF000000000000ULL, 0x0000000000000000ULL },   /* +1.0 */
    { 0xBFFF000000000000ULL, 0x0000000000000000ULL },   /* -1.0 */
    { 0x3FFF000000000000ULL, 0x0000000000000001ULL },
    { 0xBFFF000000000000ULL, 0x0000000000000001ULL },
    { 0x4000000000000000ULL, 0x0000000000000000ULL },   /* +2.0 */
    { 0xC000000000000000ULL, 0x0000000000000000ULL },   /* -2.0 */
    { 0x4001000000000000ULL, 0x0000000000000000ULL },
    { 0xC001000000000000ULL, 0x0000000000000000ULL },
    /* largest finite */
    { 0x7FFEFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL },
    { 0xFFFEFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL },
    /* infinities */
    { 0x7FFF000000000000ULL, 0x0000000000000000ULL },
    { 0xFFFF000000000000ULL, 0x0000000000000000ULL },
    /* NaNs */
    { 0x7FFF800000000000ULL, 0x0000000000000000ULL },
    { 0xFFFF800000000000ULL, 0x0000000000000000ULL },
    { 0x7FFF000000000000ULL, 0x0000000000000001ULL },
    { 0x7FFF000000000001ULL, 0x0000000000000000ULL },
    { 0xFFFF000000000000ULL, 0x0000000000000001ULL },
    { 0x7FFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL },
    { 0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL },
    /* guard / high-bit patterns */
    { 0x7F7F7F7F7F7F7F7FULL, 0x7F7F7F7F7F7F7F7FULL },
    { 0x8080808080808080ULL, 0x8080808080808080ULL },
    { 0x0000000000000000ULL, 0xFFFFFFFFFFFFFFFFULL },
    { 0xFFFFFFFFFFFFFFFFULL, 0x0000000000000000ULL },
    { 0x00000000000000FFULL, 0xFF00000000000000ULL },
    { 0x0001020304050607ULL, 0x08090A0B0C0D0E0FULL },
    { 0xF0E1D2C3B4A59687ULL, 0x78695A4B3C2D1E0FULL },
};

enum { NEDGE = (int)(sizeof edge / sizeof edge[0]) };

/* ------------------------------------------------------------------ */
/* Fixed-seed randomised sweep.                                       */

static std::uint64_t rng_state = 0x9E3779B97F4A7C15ULL;

static std::uint64_t
next_u64(void)
{
    std::uint64_t z = (rng_state += 0x9E3779B97F4A7C15ULL);
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    return z ^ (z >> 31);
}

static std::uint32_t
next_u32(void)
{
    return (std::uint32_t)(next_u64() >> 32);
}

static Val
random_val(void)
{
    Val v;
    std::uint64_t r = next_u64();
    unsigned shape = (unsigned)(next_u32() % 10u);

    switch (shape) {
    case 0:
        v = edge[next_u32() % (unsigned)NEDGE];
        break;
    case 1: /* exponent 0x7FFF: infinity or NaN */
        v.high = 0x7FFF000000000000ULL |
                 ((std::uint64_t)(next_u32() & 1u) << 63);
        v.low = ((next_u32() & 1u) ? (r & 0x8000000000000000ULL) : r);
        break;
    case 2: /* exponent 0: zero or denormal */
        v.high = (std::uint64_t)(next_u32() & 1u) << 63;
        v.low = ((next_u32() % 3u) == 0u) ? 0ULL : r;
        break;
    case 3: /* small exponents */
        v.high = ((std::uint64_t)(next_u32() & 1u) << 63) |
                 ((std::uint64_t)(next_u32() % 4u) << 48);
        v.low = r;
        break;
    case 4: /* largest finite exponents */
        v.high = ((std::uint64_t)(next_u32() & 1u) << 63) |
                 ((0x7FFEULL - (next_u32() % 4u)) << 48);
        v.low = r;
        break;
    case 5: /* normalised values */
        v.high = ((std::uint64_t)(next_u32() & 1u) << 63) |
                 ((std::uint64_t)(1u + next_u32() % 0x7FFEu) << 48) |
                 0x0001000000000000ULL;
        v.low = r;
        break;
    case 6: /* significands clustered around 0 and around the top */
        v.high = ((std::uint64_t)(next_u32() & 1u) << 63) |
                 (next_u32() % 0x0000FFFFFFFFFFFFULL);
        v.low = (std::uint64_t)(next_u32() % 4u) - 1ULL;
        break;
    default: /* wholly random */
        v.high = next_u64();
        v.low = r;
        break;
    }
    return v;
}

static Val
derive_val(const Val &a)
{
    Val v = a;
    switch (next_u32() % 8u) {
    case 0:
        break;
    case 1:
        v.high ^= 0x8000000000000000ULL;
        break;
    case 2:
        v.low = v.low + 1ULL;
        break;
    case 3:
        v.low = v.low - 1ULL;
        break;
    case 4:
        v.high = v.high + (1ULL << 48);
        break;
    case 5:
        v.high = v.high - (1ULL << 48);
        break;
    case 6:
        v.low = v.low ^ (1ULL << (next_u32() % 64u));
        break;
    default:
        v.high = v.high ^ (1ULL << (next_u32() % 64u));
        break;
    }
    return v;
}

/* ------------------------------------------------------------------ */

int
main(void)
{
    long long pairs = 0;

    for (int i = 0; i < NEDGE; i++) {
        for (int j = 0; j < NEDGE; j++) {
            check(edge[i], edge[j]);
            pairs++;
        }
    }
    long long edge_pairs = pairs;

    const long long ITERS = 250000;
    for (long long k = 0; k < ITERS; k++) {
        Val a = random_val();
        Val b;
        if ((next_u32() % 3u) == 0u)
            b = derive_val(a);
        else
            b = random_val();
        check(a, b);
        pairs++;
        check(b, a);
        pairs++;
    }

    std::printf("\n");
    std::printf("edge pairs: %lld   random pairs: %lld   total pairs: %lld\n",
                edge_pairs, pairs - edge_pairs, pairs);
    std::printf("%-16s %12s %12s\n", "function", "cases", "failures");
    std::printf("%-16s %12s %12s\n", "----------------", "------------",
                "------------");

    long long total_failures = 0;
    for (unsigned i = 0; i < sizeof stats / sizeof stats[0]; i++) {
        std::printf("%-16s %12lld %12lld\n", stats[i].name, stats[i].cases,
                    stats[i].failures);
        total_failures += stats[i].failures;
    }
    std::printf("%-16s %12s %12lld\n", "TOTAL", "", total_failures);

    if (total_failures != 0) {
        std::printf("\nRESULT: FAIL (%lld mismatches)\n", total_failures);
        return 1;
    }
    std::printf("\nRESULT: PASS\n");
    return 0;
}
