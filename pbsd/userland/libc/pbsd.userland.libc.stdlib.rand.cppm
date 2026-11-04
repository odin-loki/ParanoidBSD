module;

#include <cstdint>

export module pbsd.userland.libc.stdlib.rand;

/// rand/srand/rand_r from hbsd/src/lib/libc/stdlib/rand.c (Park-Miller LCG)
export namespace pbsd::userland::libc {

namespace detail {
[[nodiscard]] inline int do_rand(unsigned long* ctx) noexcept {
    long hi;
    long lo;
    long x;

    x = (*ctx % 0x7ffffffeUL) + 1;
    hi = x / 127773;
    lo = x % 127773;
    x = 16807 * lo - 2836 * hi;
    if (x < 0) {
        x += 0x7fffffff;
    }
    x--;
    *ctx = static_cast<unsigned long>(x);
    return static_cast<int>(x);
}
} // namespace detail

inline unsigned long& rand_seed() noexcept {
    static unsigned long next = 1;
    return next;
}

[[nodiscard]] inline int rand() noexcept {
    return detail::do_rand(&rand_seed());
}

inline void srand(unsigned seed) noexcept {
    rand_seed() = seed;
}

[[nodiscard]] inline int rand_r(unsigned* ctx) noexcept {
    unsigned long val = *ctx;
    const int r = detail::do_rand(&val);
    *ctx = static_cast<unsigned>(val);
    return r;
}

} // namespace pbsd::userland::libc
