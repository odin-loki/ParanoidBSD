module;

#include <cmath>

export module pbsd.userland.msun.atan2;

import pbsd.userland.msun.atan;

/// atan2/atan2f from hbsd/src/lib/msun/src/e_atan2.c
export namespace pbsd::userland::msun {

inline constexpr double kPi = 3.14159265358979323846;

[[nodiscard]] inline double atan2(double y, double x) noexcept {
    if (x > 0.0) {
        return atan(y / x);
    }
    if (x < 0.0) {
        return atan(y / x) + (y >= 0.0 ? kPi : -kPi);
    }
    if (y > 0.0) {
        return kPi / 2.0;
    }
    if (y < 0.0) {
        return -kPi / 2.0;
    }
    return 0.0;
}

[[nodiscard]] inline float atan2f(float y, float x) noexcept {
    return static_cast<float>(atan2(static_cast<double>(y), static_cast<double>(x)));
}

} // namespace pbsd::userland::msun
