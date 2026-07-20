module;

#include <cmath>

export module pbsd.userland.msun.remainder;

/// remainder/remainderf from hbsd/src/lib/msun/src/e_remainder.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double remainder(double x, double y) noexcept {
    return std::remainder(x, y);
}

[[nodiscard]] inline float remainderf(float x, float y) noexcept {
    return std::remainderf(x, y);
}

} // namespace pbsd::userland::msun
