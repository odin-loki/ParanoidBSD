module;

#include <cmath>

export module pbsd.userland.msun.fmax;

/// fmax/fmaxf from hbsd/src/lib/msun/src/s_fmax.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double fmax(double x, double y) noexcept {
    if (std::isnan(x)) {
        return y;
    }
    if (std::isnan(y)) {
        return x;
    }
    return x > y ? x : y;
}

[[nodiscard]] inline float fmaxf(float x, float y) noexcept {
    if (std::isnan(x)) {
        return y;
    }
    if (std::isnan(y)) {
        return x;
    }
    return x > y ? x : y;
}

} // namespace pbsd::userland::msun
