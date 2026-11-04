// SPDX-License-Identifier: AGPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
//
// Tests for pbsd.fp. These are the claims the module makes, checked
// rather than asserted in a comment.
//
// The exhaustive ones matter more than the spot checks: bfloat16 has 65,536
// values and binary32 has 2^32, so "every bfloat16 round-trips" and "every
// binary32 narrows the same way the reference algorithm does" are both
// provable by enumeration in under a second, and there is no reason to test
// a floating-point conversion by sampling when the whole domain fits.

import pbsd.fp;

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cmath>

using namespace pbsd::fp;

static int failures = 0;

static void check(bool cond, const char *what)
{
    if (!cond) {
        std::printf("FAIL %s\n", what);
        ++failures;
    }
}

// The reference bfloat16 rounding, written the way everyone writes it, to
// compare the generic one against.
static std::uint16_t reference_f32_to_bf16(float f)
{
    std::uint32_t x;
    std::memcpy(&x, &f, sizeof x);
    if ((x & 0x7f800000u) == 0x7f800000u && (x & 0x007fffffu) != 0)
        return static_cast<std::uint16_t>((x >> 16) | 0x0040u);   // quiet NaN
    const std::uint32_t lsb = (x >> 16) & 1u;
    x += 0x7fffu + lsb;
    return static_cast<std::uint16_t>(x >> 16);
}

static void test_shapes()
{
    check(bfloat16::width == 16, "bfloat16 is 16 bits");
    check(bfloat16::exponent_bits == 8, "bfloat16 has 8 exponent bits");
    check(bfloat16::mantissa_bits == 7, "bfloat16 has 7 mantissa bits");
    check(bfloat32::width == 32, "bfloat32 is 32 bits");
    check(bfloat32::exponent_bits == 11, "bfloat32 has 11 exponent bits");
    check(bfloat32::mantissa_bits == 20, "bfloat32 has 20 mantissa bits");

    // The range claim: a bfloat has the exponent range of a format twice its
    // size, so its largest finite value has the same exponent as that
    // format's.
    check(bfloat16::max().raw_exponent() == binary32::max().raw_exponent(),
          "bfloat16 reaches binary32's top exponent");
    check(bfloat32::max().raw_exponent() == binary64::max().raw_exponent(),
          "bfloat32 reaches binary64's top exponent");
}

static void test_bfloat16_exhaustive_roundtrip()
{
    // Widening is exact, so narrow(widen(x)) == x for every x, NaN included
    // (widening keeps the payload, narrowing forces quiet - so quiet NaNs
    // round-trip and signalling ones become quiet, which is checked apart).
    for (std::uint32_t i = 0; i < 0x10000u; ++i) {
        const auto b = bfloat16::from_bits(static_cast<std::uint16_t>(i));
        const auto back = narrow<bfloat16>(widen<binary32>(b));
        const bool quiet_nan = b.is_nan() && (b.to_bits() & bfloat16::quiet_mask);
        const bool exact = back.to_bits() == b.to_bits();
        if (b.is_nan() && !quiet_nan) {
            check(back.is_nan(), "signalling NaN stays NaN through a round trip");
        } else if (!exact) {
            std::printf("FAIL bfloat16 0x%04x round-tripped to 0x%04x\n",
                        i, back.to_bits());
            ++failures;
            return;
        }
    }
}

static void test_bfloat16_narrowing_matches_reference()
{
    // Every binary32 bit pattern, against the hand-written algorithm.
    for (std::uint64_t i = 0; i <= 0xffffffffull; ++i) {
        const auto x = static_cast<std::uint32_t>(i);
        float f;
        std::memcpy(&f, &x, sizeof f);
        const auto got = narrow<bfloat16>(binary32::from_bits(x)).to_bits();
        const auto want = reference_f32_to_bf16(f);
        if (got != want) {
            std::printf("FAIL binary32 0x%08x narrowed to 0x%04x, reference 0x%04x\n",
                        x, got, want);
            ++failures;
            return;
        }
    }
}

static void test_arithmetic()
{
    // 1 + 1 == 2, in a format where 1 and 2 are both exact.
    const auto one = from_native<bfloat16>(1.0f);
    const auto two = from_native<bfloat16>(2.0f);
    check(add(one, one).to_bits() == two.to_bits(), "bfloat16 1 + 1 == 2");
    check(mul(two, two).to_bits() == from_native<bfloat16>(4.0f).to_bits(),
          "bfloat16 2 * 2 == 4");
    check(sub(two, one).to_bits() == one.to_bits(), "bfloat16 2 - 1 == 1");
    check(div(two, two).to_bits() == one.to_bits(), "bfloat16 2 / 2 == 1");

    // Rounding: 1 + epsilon/2 ties to even, so it stays 1.
    const auto half_eps = from_native<bfloat16>(
        to_native(bfloat16::epsilon()) / 2.0f);
    check(add(one, half_eps).to_bits() == one.to_bits(),
          "bfloat16 1 + eps/2 ties to even");
    // 1 + epsilon is the next value up.
    const auto next = add(one, from_native<bfloat16>(to_native(bfloat16::epsilon())));
    check(next.to_bits() == static_cast<std::uint16_t>(one.to_bits() + 1),
          "bfloat16 1 + eps is the next representable value");

    // The range claim, arithmetically: bfloat16 holds a number binary16
    // cannot. 3.4e38 is near binary32's maximum and far above binary16's.
    const auto big = from_native<bfloat16>(3.0e38f);
    check(big.is_finite(), "bfloat16 represents 3e38 as a finite number");
    check(to_native(big) > 2.0e38f, "and it is the right magnitude");

    // Overflow goes to infinity rather than wrapping.
    const auto huge = mul(from_native<bfloat16>(3.0e38f),
                          from_native<bfloat16>(3.0e38f));
    check(huge.is_infinite(), "bfloat16 overflow reaches infinity");

    // NaN propagates.
    check(add(bfloat16::quiet_nan(), one).is_nan(), "NaN + 1 is NaN");
    check(!equal(bfloat16::quiet_nan(), bfloat16::quiet_nan()),
          "NaN is not equal to itself");

    // Signed zero.
    const auto zp = bfloat16::zero(false);
    const auto zn = bfloat16::zero(true);
    check(zp.to_bits() != zn.to_bits(), "+0 and -0 differ in bits");
    check(equal(zp, zn), "+0 == -0 numerically");
}

static void test_bfloat32()
{
    const auto one = from_native<bfloat32>(1.0);
    check(add(one, one).to_bits() == from_native<bfloat32>(2.0).to_bits(),
          "bfloat32 1 + 1 == 2");
    // bfloat32 has binary64's range: 1e300 is finite in it and not in binary32.
    const auto big = from_native<bfloat32>(1.0e300);
    check(big.is_finite(), "bfloat32 represents 1e300 as a finite number");
    check(to_native(big) > 1.0e299, "and it is the right magnitude");
    check(static_cast<float>(1.0e300) == static_cast<float>(1.0e300) &&
          std::isinf(static_cast<float>(1.0e300)),
          "the same value overflows binary32, which is the point");
}

static void test_conversions()
{
    // Between formats, through the hosts.
    const auto b16 = from_native<bfloat16>(1.5f);
    const auto b32 = convert<bfloat32>(b16);
    check(to_native(b32) == 1.5, "bfloat16 1.5 converts to bfloat32 1.5");
    check(convert<bfloat16>(b32).to_bits() == b16.to_bits(),
          "and back again");

    const auto f32 = convert<binary32>(b16);
    check(to_native(f32) == 1.5f, "bfloat16 1.5 converts to binary32 1.5");
}

int main()
{
    test_shapes();
    test_bfloat16_exhaustive_roundtrip();
    test_bfloat16_narrowing_matches_reference();
    test_arithmetic();
    test_bfloat32();
    test_conversions();

    if (failures != 0) {
        std::printf("\n%d check(s) failed.\n", failures);
        return 1;
    }
    std::printf("pbsd.fp OK\n");
    std::printf("  bfloat16  1+%u+%u  all %u values round-trip through binary32\n",
                bfloat16::exponent_bits, bfloat16::mantissa_bits, 1u << 16);
    std::printf("  bfloat16  all 2^32 binary32 values narrow as the reference does\n");
    std::printf("  bfloat32  1+%u+%u\n",
                bfloat32::exponent_bits, bfloat32::mantissa_bits);
    return 0;
}
