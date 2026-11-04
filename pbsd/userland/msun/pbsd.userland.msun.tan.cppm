module;

export module pbsd.userland.msun.tan;

import pbsd.userland.msun.sin;
import pbsd.userland.msun.cos;

/// tan/tanf from hbsd/src/lib/msun/src/s_tan.c (ratio on reduced range)
export namespace pbsd::userland::msun {

[[nodiscard]] inline double tan(double x) noexcept {
    const double c = cos(x);
    if (c == 0.0) {
        return 0.0 / c;
    }
    return sin(x) / c;
}

[[nodiscard]] inline float tanf(float x) noexcept {
    return static_cast<float>(tan(static_cast<double>(x)));
}

} // namespace pbsd::userland::msun
