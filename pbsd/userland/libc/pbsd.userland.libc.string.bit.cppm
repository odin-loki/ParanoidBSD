module;

#include <climits>
#include <cstdint>

export module pbsd.userland.libc.string.bit;

/// ffs/ffsl/fls/flsl from hbsd/src/lib/libc/string/{ffs,ffsl,fls,flsl}.c
export namespace pbsd::userland::libc {

namespace detail {
[[nodiscard]] inline int ctz_u32(unsigned x) noexcept {
    if (x == 0) {
        return 32;
    }
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_ctz(x);
#else
    int n = 0;
    while ((x & 1u) == 0) {
        x >>= 1;
        ++n;
    }
    return n;
#endif
}

[[nodiscard]] inline int clz_u32(unsigned x) noexcept {
    if (x == 0) {
        return 32;
    }
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_clz(x);
#else
    int n = 0;
    while (x <= 0x7fffffffU) {
        x <<= 1;
        ++n;
    }
    return n;
#endif
}
} // namespace detail

[[nodiscard]] inline int ffs(int mask) noexcept {
    return mask == 0 ? 0 : detail::ctz_u32(static_cast<unsigned>(mask)) + 1;
}

[[nodiscard]] inline int ffsl(long mask) noexcept {
    return mask == 0 ? 0 : detail::ctz_u32(static_cast<unsigned long>(mask)) + 1;
}

[[nodiscard]] inline int fls(int mask) noexcept {
    return mask == 0 ? 0
                      : static_cast<int>(sizeof(int) * CHAR_BIT) -
                            detail::clz_u32(static_cast<unsigned>(mask));
}

[[nodiscard]] inline int flsl(long mask) noexcept {
    return mask == 0 ? 0
                      : static_cast<int>(sizeof(long) * CHAR_BIT) -
                            detail::clz_u32(static_cast<unsigned long>(mask));
}

} // namespace pbsd::userland::libc
