// SPDX-License-Identifier: AGPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
//
// One generic binary floating-point format, and bfloat as an instance of it.
//
// The tree carries five copies of NetBSD's softfloat.h (two distinct texts)
// and a sixth template, all describing the same thing: sign, exponent,
// mantissa, and rules for moving between widths. basic_float<E, M> is that
// description written once, with the widths as parameters.
//
// The design rule, and the reason this is worth having:
//
//   THIS MODULE NEVER IMPLEMENTS ARITHMETIC.
//
// Hand-written soft float is where floating-point bugs live. Every format
// here is exactly representable in a wider format the target can already
// compute in, so arithmetic is done there and rounded once. Where that is
// not provably equivalent to computing in the narrow format directly, it is
// a compile error rather than a rounding difference nobody notices.
//
// bfloat is not a special case in this scheme, it is the general one. For
// any width W, take the IEEE format of width 2W and keep the top W bits:
// the sign, the whole exponent field, and as much mantissa as fits.
//
//   bfloat16 = top 16 bits of binary32   1 + 8 + 7
//   bfloat32 = top 32 bits of binary64   1 + 11 + 20
//   bfloat64 = top 64 bits of binary128  1 + 15 + 48
//
// That is what gives it the range of a format twice its size - the exponent
// field is copied whole - and what makes it hardware-compatible for free.
// Widening is a shift by W with no rounding at all; narrowing is an add and
// a shift, because the fields are contiguous so a mantissa carry lands in
// the exponent and an exponent carry lands in infinity, both correctly.
module;

#include <bit>
#include <cstddef>
#include <cstdint>
#include <type_traits>

export module pbsd.fp;

export namespace pbsd::fp {

// ---------------------------------------------------------------------------
// Storage
// ---------------------------------------------------------------------------

template<unsigned Bits>
struct uint_for {
    static_assert(Bits <= 64, "no unsigned integer wide enough for this format");
    using type =
        std::conditional_t<(Bits <= 8),  std::uint8_t,
        std::conditional_t<(Bits <= 16), std::uint16_t,
        std::conditional_t<(Bits <= 32), std::uint32_t, std::uint64_t>>>;
};

template<unsigned Bits>
using uint_for_t = typename uint_for<Bits>::type;

// ---------------------------------------------------------------------------
// The IEEE 754 interchange formats, by width
// ---------------------------------------------------------------------------
//
// IEEE 754-2008 gives k - round(4*log2(k)) + 13 significand bits for
// k >= 128. The four widths below are the ones the standard names outright
// and the only ones anything here uses, so they are written out rather than
// computed - a table is checkable by eye and log2 in a constant expression
// is not.

constexpr unsigned ieee_exponent_bits(unsigned width) noexcept
{
    return width == 16  ? 5u
         : width == 32  ? 8u
         : width == 64  ? 11u
         : width == 128 ? 15u
         : 0u;  // not an interchange width
}

// ---------------------------------------------------------------------------
// basic_float
// ---------------------------------------------------------------------------

template<unsigned E, unsigned M>
struct basic_float {
    static constexpr unsigned exponent_bits = E;
    static constexpr unsigned mantissa_bits = M;
    static constexpr unsigned width = 1u + E + M;

    // IEEE calls this p: the mantissa plus the implicit leading bit.
    static constexpr unsigned precision = M + 1u;
    static constexpr int bias = (1 << (E - 1)) - 1;

    using bits_type = uint_for_t<width>;

    static constexpr unsigned storage_bits =
        static_cast<unsigned>(sizeof(bits_type) * 8u);
    static_assert(storage_bits == width,
        "basic_float only describes formats that fill their storage exactly");

    static constexpr bits_type sign_mask =
        static_cast<bits_type>(bits_type{1} << (width - 1));
    static constexpr bits_type mantissa_mask =
        static_cast<bits_type>((bits_type{1} << M) - 1);
    static constexpr bits_type exponent_mask =
        static_cast<bits_type>(~(sign_mask | mantissa_mask));
    // The quiet bit is the top mantissa bit, in every IEEE binary format.
    static constexpr bits_type quiet_mask =
        static_cast<bits_type>(bits_type{1} << (M - 1));

    bits_type bits{};

    // -- construction from a bit pattern, which is the only primitive --------

    static constexpr basic_float from_bits(bits_type b) noexcept
    {
        return basic_float{b};
    }

    [[nodiscard]] constexpr bits_type to_bits() const noexcept { return bits; }

    // -- classification ------------------------------------------------------

    [[nodiscard]] constexpr bool sign() const noexcept
    {
        return (bits & sign_mask) != 0;
    }

    [[nodiscard]] constexpr bits_type raw_exponent() const noexcept
    {
        return static_cast<bits_type>((bits & exponent_mask) >> M);
    }

    [[nodiscard]] constexpr bits_type raw_mantissa() const noexcept
    {
        return static_cast<bits_type>(bits & mantissa_mask);
    }

    [[nodiscard]] constexpr bool is_zero() const noexcept
    {
        return (bits & ~sign_mask) == 0;
    }

    [[nodiscard]] constexpr bool is_subnormal() const noexcept
    {
        return raw_exponent() == 0 && raw_mantissa() != 0;
    }

    [[nodiscard]] constexpr bool is_infinite() const noexcept
    {
        return raw_exponent() == (exponent_mask >> M) && raw_mantissa() == 0;
    }

    [[nodiscard]] constexpr bool is_nan() const noexcept
    {
        return raw_exponent() == (exponent_mask >> M) && raw_mantissa() != 0;
    }

    [[nodiscard]] constexpr bool is_finite() const noexcept
    {
        return raw_exponent() != (exponent_mask >> M);
    }

    // -- the constants that a format has to be able to name -------------------

    static constexpr basic_float zero(bool negative = false) noexcept
    {
        return from_bits(negative ? sign_mask : bits_type{0});
    }

    static constexpr basic_float infinity(bool negative = false) noexcept
    {
        return from_bits(static_cast<bits_type>(
            exponent_mask | (negative ? sign_mask : bits_type{0})));
    }

    static constexpr basic_float quiet_nan() noexcept
    {
        return from_bits(static_cast<bits_type>(exponent_mask | quiet_mask));
    }

    // Largest finite: exponent one below all-ones, mantissa all ones.
    static constexpr basic_float max() noexcept
    {
        return from_bits(static_cast<bits_type>(
            (exponent_mask - (bits_type{1} << M)) | mantissa_mask));
    }

    // Smallest positive normal: exponent 1, mantissa zero.
    static constexpr basic_float min_normal() noexcept
    {
        return from_bits(static_cast<bits_type>(bits_type{1} << M));
    }

    // Smallest positive subnormal.
    static constexpr basic_float denorm_min() noexcept
    {
        return from_bits(bits_type{1});
    }

    // 2^-M, the gap above 1.
    static constexpr basic_float epsilon() noexcept
    {
        return from_bits(static_cast<bits_type>(
            static_cast<bits_type>(bias - static_cast<int>(M)) << M));
    }

    friend constexpr bool operator==(basic_float a, basic_float b) noexcept
    {
        if (a.is_nan() || b.is_nan())
            return false;
        if (a.is_zero() && b.is_zero())
            return true;  // +0 == -0
        return a.bits == b.bits;
    }
};

// ---------------------------------------------------------------------------
// The named formats
// ---------------------------------------------------------------------------

using binary16  = basic_float<5, 10>;
using binary32  = basic_float<8, 23>;
using binary64  = basic_float<11, 52>;
// binary128 is not declared. Its storage needs 128 bits and uint_for stops at
// 64, so bfloat64 - which would be binary128's high half - is not declared
// either. Both are a widening of uint_for away, and neither is faked here.

// bfloat<W> is the top W bits of the interchange format of width 2W. The
// exponent field is taken whole, which is the entire point: the same range as
// a format twice the size, at a quarter to a half of the significand.
template<unsigned W>
using bfloat = basic_float<ieee_exponent_bits(2 * W),
                           W - 1 - ieee_exponent_bits(2 * W)>;

using bfloat16 = bfloat<16>;   // 1 + 8  + 7
using bfloat32 = bfloat<32>;   // 1 + 11 + 20
// bfloat64 would be basic_float<15, 48>; its host is binary128, which no
// unsigned integer here can hold, so it is not declared rather than declared
// and broken.

static_assert(bfloat16::exponent_bits == binary32::exponent_bits);
static_assert(bfloat16::width == 16);
static_assert(bfloat32::exponent_bits == binary64::exponent_bits);
static_assert(bfloat32::width == 32);
static_assert(bfloat16::bias == binary32::bias);
static_assert(bfloat32::bias == binary64::bias);

// ---------------------------------------------------------------------------
// Truncation: the relationship that makes all of this work
// ---------------------------------------------------------------------------
//
// Narrow is the high half of Wide when they share an exponent field and the
// narrow mantissa is the wide one's top bits. Then the whole conversion is a
// shift by exactly Narrow::width.

template<typename Narrow, typename Wide>
inline constexpr bool is_truncation_of =
    Narrow::exponent_bits == Wide::exponent_bits &&
    Wide::width == 2 * Narrow::width &&
    Wide::mantissa_bits == Narrow::mantissa_bits + Narrow::width;

static_assert(is_truncation_of<bfloat16, binary32>);
static_assert(is_truncation_of<bfloat32, binary64>);

// Widening a truncation is exact - there is nothing to round.
template<typename Wide, typename Narrow>
constexpr Wide widen(Narrow n) noexcept
{
    static_assert(is_truncation_of<Narrow, Wide>,
        "widen() is only for a format that is the high half of the target");
    return Wide::from_bits(
        static_cast<typename Wide::bits_type>(
            static_cast<typename Wide::bits_type>(n.to_bits()) << Narrow::width));
}

// Narrowing rounds to nearest, ties to even.
//
// Because the fields are contiguous, adding the rounding bias to the whole
// pattern is the whole algorithm: a mantissa carry runs into the exponent and
// an exponent carry runs into infinity, and both are the right answer. The
// only case that needs its own line is NaN, which must stay NaN rather than
// being rounded into infinity, and is forced quiet on the way through.
template<typename Narrow, typename Wide>
constexpr Narrow narrow(Wide w) noexcept
{
    static_assert(is_truncation_of<Narrow, Wide>,
        "narrow() is only for a format that is the high half of the source");
    using WB = typename Wide::bits_type;
    constexpr unsigned shift = Narrow::width;

    const WB x = w.to_bits();
    if (w.is_nan()) {
        return Narrow::from_bits(static_cast<typename Narrow::bits_type>(
            static_cast<typename Narrow::bits_type>(x >> shift) |
            Narrow::quiet_mask));
    }
    const WB lsb = static_cast<WB>((x >> shift) & WB{1});
    const WB bias = static_cast<WB>(((WB{1} << (shift - 1)) - 1) + lsb);
    return Narrow::from_bits(
        static_cast<typename Narrow::bits_type>((x + bias) >> shift));
}

// ---------------------------------------------------------------------------
// The native types, and what may be computed in what
// ---------------------------------------------------------------------------

template<typename F> struct native_type { using type = void; };
template<> struct native_type<binary32> { using type = float; };
template<> struct native_type<binary64> { using type = double; };
#if defined(__FLT16_MANT_DIG__) && !defined(__STDC_NO_FLOAT16__)
template<> struct native_type<binary16> { using type = _Float16; };
#endif

template<typename F>
using native_type_t = typename native_type<F>::type;

template<typename F>
inline constexpr bool has_native = !std::is_same_v<native_type_t<F>, void>;

static_assert(has_native<binary32>);
static_assert(has_native<binary64>);
static_assert(sizeof(float) * 8 == binary32::width);
static_assert(sizeof(double) * 8 == binary64::width);

// The format arithmetic on F is actually performed in. A format with a native
// type computes in itself. A truncation computes in the format it is the high
// half of.
template<typename F> struct host_format { using type = F; };
template<> struct host_format<bfloat16> { using type = binary32; };
template<> struct host_format<bfloat32> { using type = binary64; };

template<typename F>
using host_format_t = typename host_format<F>::type;

// Figueroa's condition. Computing a single +, -, * or / in a format of
// precision q and rounding once to a format of precision p gives the same
// answer as computing directly in the narrow format, provided q >= 2p + 2.
// Every format in this family satisfies it in its host, which is why
// delegating arithmetic is not an approximation:
//
//   bfloat16 p=8   in binary32 q=24    24 >= 18
//   bfloat32 p=21  in binary64 q=53    53 >= 44
//
// If a future format does not satisfy it, this fails to compile rather than
// quietly double-rounding.
template<typename F>
inline constexpr bool host_rounds_once =
    std::is_same_v<F, host_format_t<F>> ||
    host_format_t<F>::precision >= 2 * F::precision + 2;

static_assert(host_rounds_once<bfloat16>);
static_assert(host_rounds_once<bfloat32>);

// ---------------------------------------------------------------------------
// Moving between a format and its native type
// ---------------------------------------------------------------------------

template<typename F>
constexpr native_type_t<host_format_t<F>> to_native(F v) noexcept
{
    using Host = host_format_t<F>;
    static_assert(has_native<Host>, "this format has no computable host");
    if constexpr (std::is_same_v<F, Host>) {
        return std::bit_cast<native_type_t<Host>>(v.to_bits());
    } else {
        return std::bit_cast<native_type_t<Host>>(widen<Host>(v).to_bits());
    }
}

template<typename F>
constexpr F from_native(native_type_t<host_format_t<F>> x) noexcept
{
    using Host = host_format_t<F>;
    const auto hb = std::bit_cast<typename Host::bits_type>(x);
    if constexpr (std::is_same_v<F, Host>) {
        return F::from_bits(hb);
    } else {
        return narrow<F>(Host::from_bits(hb));
    }
}

// ---------------------------------------------------------------------------
// Arithmetic, delegated
// ---------------------------------------------------------------------------

#define PBSD_FP_BINOP(op, name)                                               \
    template<typename F>                                                      \
    constexpr F name(F a, F b) noexcept                                       \
    {                                                                         \
        static_assert(host_rounds_once<F>,                                    \
            "this format's host is too narrow to round only once");           \
        return from_native<F>(to_native(a) op to_native(b));                  \
    }

PBSD_FP_BINOP(+, add)
PBSD_FP_BINOP(-, sub)
PBSD_FP_BINOP(*, mul)
PBSD_FP_BINOP(/, div)

#undef PBSD_FP_BINOP

template<typename F>
constexpr F neg(F a) noexcept
{
    return F::from_bits(static_cast<typename F::bits_type>(a.to_bits() ^ F::sign_mask));
}

template<typename F>
constexpr F abs(F a) noexcept
{
    return F::from_bits(static_cast<typename F::bits_type>(a.to_bits() & ~F::sign_mask));
}

// Comparison is done in the host too, so NaN and signed zero behave as the
// hardware says rather than as a bit pattern comparison would.
template<typename F>
constexpr bool less(F a, F b) noexcept { return to_native(a) < to_native(b); }

template<typename F>
constexpr bool equal(F a, F b) noexcept { return to_native(a) == to_native(b); }

// ---------------------------------------------------------------------------
// Conversion between any two formats in the family
// ---------------------------------------------------------------------------
//
// Everything reaches everything else through the native hosts, so this needs
// no per-pair rounding code: the compiler's float-to-float conversion is
// correctly rounded, and the truncations round once on the way in or out.

template<typename To, typename From>
constexpr To convert(From v) noexcept
{
    using ToHost = native_type_t<host_format_t<To>>;
    return from_native<To>(static_cast<ToHost>(to_native(v)));
}

}  // namespace pbsd::fp
