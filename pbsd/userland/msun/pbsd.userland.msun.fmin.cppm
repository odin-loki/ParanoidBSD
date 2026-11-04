module;

#include <cmath>

export module pbsd.userland.msun.fmin;

/// fmin/fminf from hbsd/src/lib/msun/src/s_fmin.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double fmin(double x, double y) noexcept {
    if (std::isnan(x)) {
        return y;
    }
    if (std::isnan(y)) {
        return x;
    }
    return x < y ? x : y;
}

[[nodiscard]] inline float fminf(float x, float y) noexcept {
    if (std::isnan(x)) {
        return y;
    }
    if (std::isnan(y)) {
        return x;
    }
    return x < y ? x : y;
}

} // namespace pbsd::userland::msun
