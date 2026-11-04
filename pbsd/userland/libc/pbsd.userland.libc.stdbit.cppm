module;

#include <cstdint>

export module pbsd.userland.libc.stdbit;

/// stdc_count_* / stdc_has_single_bit from hbsd/src/lib/libc/stdbit/*.c
export namespace pbsd::userland::libc {

namespace detail {
[[nodiscard]] inline unsigned popcount_u32(unsigned x) noexcept {
#if defined(__GNUC__) || defined(__clang__)
    return static_cast<unsigned>(__builtin_popcount(x));
#else
    unsigned n = 0;
    while (x) {
        n += x & 1u;
        x >>= 1;
    }
    return n;
#endif
}

[[nodiscard]] inline unsigned popcount_u64(std::uint64_t x) noexcept {
#if defined(__GNUC__) || defined(__clang__)
    return static_cast<unsigned>(__builtin_popcountll(x));
#else
    unsigned n = 0;
    while (x) {
        n += static_cast<unsigned>(x & 1u);
        x >>= 1;
    }
    return n;
#endif
}
} // namespace detail

[[nodiscard]] inline unsigned stdc_count_ones_uc(unsigned char x) noexcept {
    return detail::popcount_u32(x);
}
[[nodiscard]] inline unsigned stdc_count_ones_us(unsigned short x) noexcept {
    return detail::popcount_u32(x);
}
[[nodiscard]] inline unsigned stdc_count_ones_ui(unsigned x) noexcept {
    return detail::popcount_u32(x);
}
[[nodiscard]] inline unsigned stdc_count_ones_ul(unsigned long x) noexcept {
    return detail::popcount_u64(static_cast<std::uint64_t>(x));
}
[[nodiscard]] inline unsigned stdc_count_ones_ull(unsigned long long x) noexcept {
    return detail::popcount_u64(x);
}

[[nodiscard]] inline unsigned stdc_count_zeros_uc(unsigned char x) noexcept {
    return static_cast<unsigned>(sizeof(unsigned char) * 8) - stdc_count_ones_uc(x);
}
[[nodiscard]] inline unsigned stdc_count_zeros_ui(unsigned x) noexcept {
    return static_cast<unsigned>(sizeof(unsigned) * 8) - stdc_count_ones_ui(x);
}

[[nodiscard]] inline bool stdc_has_single_bit_ui(unsigned x) noexcept {
    return x != 0 && (x & (x - 1u)) == 0u;
}

} // namespace pbsd::userland::libc
