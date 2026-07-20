module;

export module pbsd.userland.msun.exp;

/// exp/expf from hbsd/src/lib/msun/src/e_exp.c (Taylor series)
export namespace pbsd::userland::msun {

namespace detail {
inline double taylor_exp(double x) noexcept {
    double term = 1.0;
    double sum = 1.0;
    for (int i = 1; i <= 12; ++i) {
        term *= x / static_cast<double>(i);
        sum += term;
    }
    return sum;
}
} // namespace detail

[[nodiscard]] inline double exp(double x) noexcept {
    if (x > 709.0) {
        return x / 0.0;
    }
    if (x < -745.0) {
        return 0.0;
    }
    const int n = static_cast<int>(x / 0.69314718055994530942);
    const double r = x - static_cast<double>(n) * 0.69314718055994530942;
    const double scale = static_cast<double>(1ULL << n);
    return detail::taylor_exp(r) * scale;
}

[[nodiscard]] inline float expf(float x) noexcept {
    return static_cast<float>(exp(static_cast<double>(x)));
}

} // namespace pbsd::userland::msun
