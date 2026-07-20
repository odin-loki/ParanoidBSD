module;

#include <cmath>
#include <limits>

export module pbsd.userland.msun.drem;

/// drem/dremf from hbsd/src/lib/msun/src/w_dremf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double drem(double x, double y) noexcept {
    if (y == 0.0) {
        return std::numeric_limits<double>::quiet_NaN();
    }
    const double q = std::nearbyint(x / y);
    return x - q * y;
}

[[nodiscard]] inline float dremf(float x, float y) noexcept {
    if (y == 0.0f) {
        return std::numeric_limits<float>::quiet_NaN();
    }
    const float q = std::nearbyint(x / y);
    return x - q * y;
}

} // namespace pbsd::userland::msun
