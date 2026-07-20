module;

#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.sqrt;

/// sqrt from hbsd/src/lib/msun/src/e_sqrt.c (portable Newton refinement)
export namespace pbsd::userland::msun {

[[nodiscard]] inline double sqrt(double x) noexcept {
    if (x <= 0.0) {
        if (x == 0.0) {
            return 0.0;
        }
        std::uint64_t u;
        std::memcpy(&u, &x, sizeof(u));
        if ((u & 0x7fffffffffffffffULL) > 0x7ff0000000000000ULL) {
            return x;
        }
        return x / x;
    }
    double guess = x;
    for (int i = 0; i < 12; ++i) {
        guess = 0.5 * (guess + x / guess);
    }
    return guess;
}

[[nodiscard]] inline float sqrtf(float x) noexcept {
    return static_cast<float>(sqrt(static_cast<double>(x)));
}

} // namespace pbsd::userland::msun
