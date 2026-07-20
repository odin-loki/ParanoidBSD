module;

export module pbsd.userland.msun.log;

import pbsd.userland.msun.frexp;

/// log/logf from hbsd/src/lib/msun/src/e_log.c (frexp + ln series)
export namespace pbsd::userland::msun {

namespace detail {
inline constexpr double kLn2 = 0.69314718055994530942;

inline double ln_mantissa(double m) noexcept {
    const double y = (m - 1.0) / (m + 1.0);
    const double y2 = y * y;
    return 2.0 * y * (1.0 + y2 * (1.0 / 3.0 + y2 * (1.0 / 5.0 + y2 / 7.0)));
}
} // namespace detail

[[nodiscard]] inline double log(double x) noexcept {
    if (x <= 0.0) {
        return x / 0.0;
    }
    int exp = 0;
    const double m = frexp(x, &exp);
    return detail::ln_mantissa(m) + static_cast<double>(exp) * detail::kLn2;
}

[[nodiscard]] inline float logf(float x) noexcept {
    return static_cast<float>(log(static_cast<double>(x)));
}

} // namespace pbsd::userland::msun
