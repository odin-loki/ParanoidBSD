module;
#include <cmath>

export module pbsd.userland.msun.asinh;

/// asinh from hbsd/src/lib/msun/src/e_asinh.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double asinh(double x) noexcept {
    return std::asinh(x);
}

[[nodiscard]] inline float asinhf(float x) noexcept {
    return std::asinhf(x);
}

} // namespace pbsd::userland::msun
