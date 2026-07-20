module;

#include <cmath>

export module pbsd.userland.msun.scalbln;

/// scalbln/scalblnf from hbsd/src/lib/msun/src/s_scalbln.c
export namespace pbsd::userland::msun {

inline constexpr long kNMax = 65536;
inline constexpr long kNMin = -65536;

[[nodiscard]] inline int clamp_exponent(long n) noexcept {
    if (n > kNMax) {
        return static_cast<int>(kNMax);
    }
    if (n < kNMin) {
        return static_cast<int>(kNMin);
    }
    return static_cast<int>(n);
}

[[nodiscard]] inline double scalbln(double x, long n) noexcept {
    return std::scalbn(x, clamp_exponent(n));
}

[[nodiscard]] inline float scalblnf(float x, long n) noexcept {
    return std::scalbnf(x, clamp_exponent(n));
}

} // namespace pbsd::userland::msun
