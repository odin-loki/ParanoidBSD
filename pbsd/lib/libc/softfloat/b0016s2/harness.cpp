/*
 * harness.cpp -- differential test for PBSD batch b0016s2.
 *
 * Every case is run through both the C++23 port and the ref_ oracle built
 * from the original C, and the following are compared:
 *
 *   - the return value (exactly, as int; __gexf2 returns -1/0 so a
 *     truncating or unsigned port shows up immediately);
 *   - the SoftFloat exception flags raised by the call (the NaN path of
 *     floatx80_le is only distinguishable from "false" by the flag);
 *   - the entire argument buffers.  No function in this batch writes
 *     through a pointer, so instead the two floatx80 operands are planted
 *     inside two 64-byte buffers pre-filled with the guard byte 0x7f; after
 *     the call both buffers must still be byte-identical to each other and
 *     to the pristine image, padding bytes included.
 *
 * Inputs are the full cross product of a hand-written edge table (zeroes of
 * both signs, denormals, pseudo-denormals, unnormals, both infinities,
 * pseudo-infinities, quiet and signalling NaNs, minimum and maximum
 * exponents, values differing only in the last significand bit, and equal
 * values) followed by a fixed-seed randomised sweep.
 */

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

import pbsd.lib.libc.softfloat.b0016s2;

namespace P = pbsd::lib_libc_softfloat::b0016s2;

/*
 * The oracle's floatx80 (bits16 high; bits64 low).  Declared here with the
 * identical layout rather than sharing a header so that the two sides stay
 * genuinely independent.
 */
struct ref_floatx80 {
    unsigned short high;
    unsigned long long low;
};

extern "C" {
int ref___gexf2(ref_floatx80 a, ref_floatx80 b);
int ref_floatx80_le(ref_floatx80 a, ref_floatx80 b);
extern int ref_float_exception_flags;
extern int ref_float_exception_mask;
}

static_assert(sizeof(ref_floatx80) == sizeof(P::floatx80),
              "port and oracle floatx80 must have the same size");
static_assert(sizeof(ref_floatx80) == 16, "unexpected floatx80 layout");
static_assert(offsetof(ref_floatx80, high) == 0, "unexpected floatx80 layout");
static_assert(offsetof(ref_floatx80, low) == 8, "unexpected floatx80 layout");
static_assert(offsetof(P::floatx80, high) == 0, "unexpected floatx80 layout");
static_assert(offsetof(P::floatx80, low) == 8, "unexpected floatx80 layout");

/* ------------------------------------------------------------------ */

struct Val {
    std::uint16_t high;
    std::uint64_t low;
};

struct Stat {
    const char *name;
    long long cases;
    long long failures;
};

static Stat stats[] = {
    { "floatx80_le", 0, 0 },
    { "__gexf2", 0, 0 },
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
        std::printf("FAIL %-12s a={%04x,%016llx} b={%04x,%016llx} %s: "
                    "oracle=%lld port=%lld\n",
                    stats[fn].name,
                    (unsigned)a.high, (unsigned long long)a.low,
                    (unsigned)b.high, (unsigned long long)b.low,
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
    std::uint16_t h = v.high;
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

    ref_floatx80 ra, rb;
    P::floatx80 pa, pb;
    std::memcpy(&ra, bufRef + OFF_A, sizeof ra);
    std::memcpy(&rb, bufRef + OFF_B, sizeof rb);
    std::memcpy(&pa, bufPort + OFF_A, sizeof pa);
    std::memcpy(&pb, bufPort + OFF_B, sizeof pb);

    for (int fn = FN_LE; fn <= FN_GE; fn++) {
        stats[fn].cases++;

        ref_float_exception_flags = 0;
        int rret = (fn == FN_LE) ? ref_floatx80_le(ra, rb)
                                 : ref___gexf2(ra, rb);
        int rflags = ref_float_exception_flags;

        P::float_exception_flags = 0;
        int pret = (fn == FN_LE) ? P::floatx80_le(pa, pb)
                                 : P::gexf2(pa, pb);
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
    { 0x0000, 0x0000000000000000ULL },   /* +0                       */
    { 0x8000, 0x0000000000000000ULL },   /* -0                       */
    /* denormals: exponent 0, significand MSB clear                   */
    { 0x0000, 0x0000000000000001ULL },
    { 0x8000, 0x0000000000000001ULL },
    { 0x0000, 0x7FFFFFFFFFFFFFFFULL },
    { 0x8000, 0x7FFFFFFFFFFFFFFFULL },
    /* pseudo-denormals: exponent 0, significand MSB set              */
    { 0x0000, 0x8000000000000000ULL },
    { 0x8000, 0x8000000000000000ULL },
    { 0x0000, 0xFFFFFFFFFFFFFFFFULL },
    { 0x8000, 0xFFFFFFFFFFFFFFFFULL },
    /* smallest non-zero exponents                                    */
    { 0x0001, 0x0000000000000000ULL },
    { 0x8001, 0x0000000000000000ULL },
    { 0x0001, 0x8000000000000000ULL },
    { 0x8001, 0x8000000000000000ULL },
    { 0x0002, 0x0000000000000000ULL },
    { 0x8002, 0x0000000000000000ULL },
    { 0x0003, 0xFFFFFFFFFFFFFFFFULL },
    { 0x8003, 0xFFFFFFFFFFFFFFFFULL },
    /* ordinary values                                                */
    { 0x3FFE, 0x8000000000000000ULL },   /* +0.5                     */
    { 0xBFFE, 0x8000000000000000ULL },   /* -0.5                     */
    { 0x3FFF, 0x8000000000000000ULL },   /* +1.0                     */
    { 0xBFFF, 0x8000000000000000ULL },   /* -1.0                     */
    { 0x3FFF, 0x8000000000000001ULL },   /* +1.0 + 1ulp              */
    { 0xBFFF, 0x8000000000000001ULL },   /* -1.0 - 1ulp              */
    { 0x3FFF, 0x7FFFFFFFFFFFFFFFULL },   /* unnormal                 */
    { 0xBFFF, 0x7FFFFFFFFFFFFFFFULL },
    { 0x4000, 0x0000000000000000ULL },
    { 0xC000, 0x0000000000000000ULL },
    { 0x4000, 0x8000000000000000ULL },   /* +2.0                     */
    { 0xC000, 0x8000000000000000ULL },   /* -2.0                     */
    /* largest finite exponent                                        */
    { 0x7FFE, 0x0000000000000000ULL },
    { 0xFFFE, 0x0000000000000000ULL },
    { 0x7FFE, 0xFFFFFFFFFFFFFFFFULL },
    { 0xFFFE, 0xFFFFFFFFFFFFFFFFULL },
    /* exponent 0x7FFF with significand<<1 == 0: infinities and       */
    /* pseudo-infinities -- NOT NaNs                                  */
    { 0x7FFF, 0x8000000000000000ULL },   /* +inf                     */
    { 0xFFFF, 0x8000000000000000ULL },   /* -inf                     */
    { 0x7FFF, 0x0000000000000000ULL },   /* +pseudo-inf              */
    { 0xFFFF, 0x0000000000000000ULL },   /* -pseudo-inf              */
    /* exponent 0x7FFF with significand<<1 != 0: NaNs                 */
    { 0x7FFF, 0x0000000000000001ULL },
    { 0xFFFF, 0x0000000000000001ULL },
    { 0x7FFF, 0x4000000000000000ULL },
    { 0x7FFF, 0xA000000000000000ULL },   /* signalling NaN           */
    { 0xFFFF, 0xA000000000000000ULL },
    { 0x7FFF, 0xC000000000000000ULL },   /* quiet NaN                */
    { 0xFFFF, 0xC000000000000000ULL },
    { 0x7FFF, 0x7FFFFFFFFFFFFFFFULL },
    { 0x7FFF, 0xFFFFFFFFFFFFFFFFULL },   /* default NaN              */
    { 0xFFFF, 0xFFFFFFFFFFFFFFFFULL },
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

/*
 * Generate a value from a mix of shapes so that every branch of the port is
 * reached often: NaN/infinity exponents, zero exponents, ordinary
 * exponents, all-random bit patterns and edge-table entries.
 */
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
    case 1: /* exponent 0x7FFF: infinity, pseudo-infinity or NaN */
        v.high = (std::uint16_t)(0x7FFFu | ((next_u32() & 1u) << 15));
        v.low = ((next_u32() & 1u) ? (r & 0x8000000000000000ULL) : r);
        break;
    case 2: /* exponent 0: zero, denormal or pseudo-denormal */
        v.high = (std::uint16_t)((next_u32() & 1u) << 15);
        v.low = ((next_u32() % 3u) == 0u) ? 0ULL : r;
        break;
    case 3: /* small exponents */
        v.high = (std::uint16_t)(((next_u32() & 1u) << 15) |
                                 (next_u32() % 4u));
        v.low = r;
        break;
    case 4: /* largest finite exponents */
        v.high = (std::uint16_t)(((next_u32() & 1u) << 15) |
                                 (0x7FFEu - (next_u32() % 4u)));
        v.low = r;
        break;
    case 5: /* normalised values */
        v.high = (std::uint16_t)(((next_u32() & 1u) << 15) |
                                 (1u + next_u32() % 0x7FFEu));
        v.low = r | 0x8000000000000000ULL;
        break;
    case 6: /* significands clustered around 0 and around the top */
        v.high = (std::uint16_t)(((next_u32() & 1u) << 15) |
                                 (next_u32() % 0x8000u));
        v.low = (std::uint64_t)(next_u32() % 4u) - 1ULL;
        break;
    default: /* wholly random */
        v.high = (std::uint16_t)next_u32();
        v.low = r;
        break;
    }
    return v;
}

/*
 * Derive `b' from `a' so that the equality-sensitive comparisons inside
 * le128 (a0 == b0, a1 <= b1) get hit from both sides, not just by chance.
 */
static Val
derive_val(const Val &a)
{
    Val v = a;
    switch (next_u32() % 8u) {
    case 0:
        break;                                          /* identical      */
    case 1:
        v.high = (std::uint16_t)(v.high ^ 0x8000u);     /* opposite sign  */
        break;
    case 2:
        v.low = v.low + 1ULL;                           /* one ulp up     */
        break;
    case 3:
        v.low = v.low - 1ULL;                           /* one ulp down   */
        break;
    case 4:
        v.high = (std::uint16_t)(v.high + 1u);
        break;
    case 5:
        v.high = (std::uint16_t)(v.high - 1u);
        break;
    case 6:
        v.low = v.low ^ (1ULL << (next_u32() % 64u));
        break;
    default:
        v.high = (std::uint16_t)(v.high ^ (1u << (next_u32() % 16u)));
        break;
    }
    return v;
}

/* ------------------------------------------------------------------ */

int
main(void)
{
    long long pairs = 0;

    /* Hand-written edge cases: the full cross product, both orders and
       including a == b. */
    for (int i = 0; i < NEDGE; i++) {
        for (int j = 0; j < NEDGE; j++) {
            check(edge[i], edge[j]);
            pairs++;
        }
    }
    long long edge_pairs = pairs;

    /* Randomised sweep. */
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
        check(b, a);            /* the mirrored pair too */
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
