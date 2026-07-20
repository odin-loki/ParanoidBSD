module;

#include <climits>
#include <cstdint>

export module pbsd.userland.libc.string.bit.ext;

/// ffsll/flsll from hbsd/src/lib/libc/string/{ffsll,flsll}.c
export namespace pbsd::userland::libc {

namespace detail {
[[nodiscard]] inline int ctz_u64(std::uint64_t x) noexcept {
    if (x == 0) {
        return 64;
    }
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_ctzll(x);
#else
    int n = 0;
    while ((x & 1u) == 0) {
        x >>= 1;
        ++n;
    }
    return n;
#endif
}

[[nodiscard]] inline int clz_u64(std::uint64_t x) noexcept {
    if (x == 0) {
        return 64;
    }
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_clzll(x);
#else
    int n = 0;
    while (x <= 0x7FFFFFFFFFFFFFFFULL) {
        x <<= 1;
        ++n;
    }
    return n;
#endif
}
} // namespace detail

[[nodiscard]] inline int ffsll(long long mask) noexcept {
    return mask == 0 ? 0 : detail::ctz_u64(static_cast<std::uint64_t>(mask)) + 1;
}

[[nodiscard]] inline int flsll(long long mask) noexcept {
    return mask == 0 ? 0
                      : static_cast<int>(sizeof(long long) * CHAR_BIT) -
                            detail::clz_u64(static_cast<std::uint64_t>(mask));
}

} // namespace pbsd::userland::libc
