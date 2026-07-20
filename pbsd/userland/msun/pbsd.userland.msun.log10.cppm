module;

#include <cmath>

export module pbsd.userland.msun.log10;

import pbsd.userland.msun.log;

/// log10/log10f from hbsd/src/lib/msun/src/e_log10.c
export namespace pbsd::userland::msun {

inline constexpr double kLog10Inv = 0.43429448190325182765;

[[nodiscard]] inline double log10(double x) noexcept {
    return log(x) * kLog10Inv;
}

[[nodiscard]] inline float log10f(float x) noexcept {
    return static_cast<float>(log10(static_cast<double>(x)));
}

} // namespace pbsd::userland::msun
