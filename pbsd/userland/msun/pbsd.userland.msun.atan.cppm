module;

export module pbsd.userland.msun.atan;

/// atan/atanf from hbsd/src/lib/msun/src/e_atan.c (Taylor on |x|<=1)
export namespace pbsd::userland::msun {

namespace detail {
inline constexpr double kPiOver2 = 1.57079632679489661923;

inline double taylor_atan(double x) noexcept {
    double x2 = x * x;
    double term = x;
    double sum = x;
    term *= -x2 / 3.0;
    sum += term;
    term *= -x2 * 2.0 / 5.0;
    sum += term;
    term *= -x2 * 3.0 / 7.0;
    sum += term;
    term *= -x2 * 4.0 / 9.0;
    sum += term;
    return sum;
}
} // namespace detail

[[nodiscard]] inline double atan(double x) noexcept {
    if (x > 1.0) {
        return detail::kPiOver2 - detail::taylor_atan(1.0 / x);
    }
    if (x < -1.0) {
        return -detail::kPiOver2 - detail::taylor_atan(1.0 / x);
    }
    return detail::taylor_atan(x);
}

[[nodiscard]] inline float atanf(float x) noexcept {
    return static_cast<float>(atan(static_cast<double>(x)));
}

} // namespace pbsd::userland::msun
