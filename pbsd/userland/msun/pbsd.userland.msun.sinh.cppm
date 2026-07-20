module;

#include <cmath>

export module pbsd.userland.msun.sinh;

import pbsd.userland.msun.exp;

/// sinh/sinhf from hbsd/src/lib/msun/src/s_sinh.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double sinh(double x) noexcept {
    return (exp(x) - exp(-x)) / 2.0;
}

[[nodiscard]] inline float sinhf(float x) noexcept {
    return static_cast<float>(sinh(static_cast<double>(x)));
}

} // namespace pbsd::userland::msun
