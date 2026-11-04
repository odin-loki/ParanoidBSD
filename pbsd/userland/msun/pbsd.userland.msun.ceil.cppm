module;

#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.ceil;

/// ceil from hbsd/src/lib/msun/src/s_ceil.c (bit twiddling, double only)
export namespace pbsd::userland::msun {

namespace detail {
inline void extract_words(double x, std::uint32_t& hi, std::uint32_t& lo) noexcept {
    std::uint64_t u;
    std::memcpy(&u, &x, sizeof(u));
    hi = static_cast<std::uint32_t>(u >> 32);
    lo = static_cast<std::uint32_t>(u);
}

inline void insert_words(double& x, std::uint32_t hi, std::uint32_t lo) noexcept {
    const std::uint64_t u = (static_cast<std::uint64_t>(hi) << 32) | lo;
    std::memcpy(&x, &u, sizeof(u));
}
} // namespace detail

[[nodiscard]] inline double ceil(double x) noexcept {
    std::uint32_t i0;
    std::uint32_t i1;
    detail::extract_words(x, i0, i1);
    const int j0 = static_cast<int>(((i0 >> 20) & 0x7ffU) - 0x3ffU);
    if (j0 < 20) {
        if (j0 < 0) {
            if ((i0 | i1) != 0U) {
                if (i0 < 0x80000000U) {
                    i0 = 0x3ff00000U;
                    i1 = 0;
                } else if ((i0 & 0x7fffffffU) | i1) {
                    i0 = 0;
                    i1 = 0;
                }
            }
        } else {
            const std::uint32_t i = 0x000fffffU >> j0;
            if (((i0 & i) | i1) == 0U) {
                return x;
            }
            if (i0 < 0x80000000U) {
                i0 += 0x00100000U >> j0;
            }
            i0 &= ~i;
            i1 = 0;
        }
    } else if (j0 > 51) {
        if (j0 == 0x400) {
            return x + x;
        }
        return x;
    } else {
        const std::uint32_t i = 0xffffffffU >> (j0 - 20);
        if ((i1 & i) == 0U) {
            return x;
        }
        if (i0 < 0x80000000U) {
            if (j0 == 20) {
                i0 += 1;
            } else {
                std::uint32_t j = i1 + (1U << (52 - j0));
                if (j < i1) {
                    i0 += 1;
                }
                i1 = j;
            }
        }
        i1 &= ~i;
    }
    detail::insert_words(x, i0, i1);
    return x;
}

} // namespace pbsd::userland::msun
