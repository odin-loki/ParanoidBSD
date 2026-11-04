module;

export module pbsd.userland.msun.hypot;

import pbsd.userland.msun.sqrt;
import pbsd.userland.msun.fabs;

/// hypot/hypotf from hbsd/src/lib/msun/src/e_hypot.c (scaled sqrt)
export namespace pbsd::userland::msun {

[[nodiscard]] inline double hypot(double x, double y) noexcept {
    x = fabs(x);
    y = fabs(y);
    if (x < y) {
        const double t = x;
        x = y;
        y = t;
    }
    if (y == 0.0) {
        return x;
    }
    const double r = y / x;
    return x * sqrt(1.0 + r * r);
}

[[nodiscard]] inline float hypotf(float x, float y) noexcept {
    return static_cast<float>(hypot(static_cast<double>(x), static_cast<double>(y)));
}

} // namespace pbsd::userland::msun
