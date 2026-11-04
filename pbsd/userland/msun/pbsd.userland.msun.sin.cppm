module;

export module pbsd.userland.msun.sin;

/// sin/sinf from hbsd/src/lib/msun/src/{k_sin,s_sin}.c (Taylor on reduced range)
export namespace pbsd::userland::msun {

namespace detail {
inline constexpr double kPi = 3.14159265358979323846;
inline constexpr double kTwoPi = 2.0 * kPi;

inline double reduce_pi(double x) noexcept {
    if (x >= -kPi && x <= kPi) {
        return x;
    }
    const double n = static_cast<double>(static_cast<long long>(x / kTwoPi));
    return x - n * kTwoPi;
}

inline double taylor_sin(double x) noexcept {
    double x2 = x * x;
    double term = x;
    double sum = x;
    term *= -x2 / (2.0 * 3.0);
    sum += term;
    term *= -x2 / (4.0 * 5.0);
    sum += term;
    term *= -x2 / (6.0 * 7.0);
    sum += term;
    term *= -x2 / (8.0 * 9.0);
    sum += term;
    return sum;
}
} // namespace detail

[[nodiscard]] inline double sin(double x) noexcept {
    const double r = detail::reduce_pi(x);
    return detail::taylor_sin(r);
}

[[nodiscard]] inline float sinf(float x) noexcept {
    return static_cast<float>(sin(static_cast<double>(x)));
}

} // namespace pbsd::userland::msun
