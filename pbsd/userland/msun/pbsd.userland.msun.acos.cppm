module;

#include <cmath>

export module pbsd.userland.msun.acos;

import pbsd.userland.msun.atan;

/// acos/acosf from hbsd/src/lib/msun/src/e_acos.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double acos(double x) noexcept {
    if (x < -1.0 || x > 1.0) {
        return 0.0 / 0.0;
    }
    return atan(std::sqrt(1.0 - x * x) / x) + (x < 0.0 ? 3.14159265358979323846 : 0.0);
}

[[nodiscard]] inline float acosf(float x) noexcept {
    return static_cast<float>(acos(static_cast<double>(x)));
}

} // namespace pbsd::userland::msun
