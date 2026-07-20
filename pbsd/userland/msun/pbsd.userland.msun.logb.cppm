module;

export module pbsd.userland.msun.logb;

import pbsd.userland.msun.ilogb;

/// logb/logbf from hbsd/src/lib/msun/src/s_logb.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double logb(double x) noexcept {
    return static_cast<double>(ilogb(x));
}

[[nodiscard]] inline float logbf(float x) noexcept {
    return static_cast<float>(ilogb(x));
}

} // namespace pbsd::userland::msun
