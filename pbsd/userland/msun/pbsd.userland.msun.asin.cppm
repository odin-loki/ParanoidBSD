module;

#include <cmath>

export module pbsd.userland.msun.asin;

import pbsd.userland.msun.atan;

/// asin/asinf from hbsd/src/lib/msun/src/e_asin.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double asin(double x) noexcept {
    if (x < -1.0 || x > 1.0) {
        return 0.0 / 0.0;
    }
    return atan(x / std::sqrt(1.0 - x * x));
}

[[nodiscard]] inline float asinf(float x) noexcept {
    return static_cast<float>(asin(static_cast<double>(x)));
}

} // namespace pbsd::userland::msun
