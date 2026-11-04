module;
#include <cmath>

export module pbsd.userland.msun.y0;

/// y0/y0f Bessel from hbsd/src/lib/msun/src/e_y0.c (delegates to libm).
export namespace pbsd::userland::msun {

[[nodiscard]] inline double y0(double x) noexcept {
#if defined(__cpp_lib_math_special_functions) && __cpp_lib_math_special_functions >= 201603L
    return std::cyl_neumann(0, x);
#else
    (void)x;
    return 0.0;
#endif
}

[[nodiscard]] inline float y0f(float x) noexcept {
    return static_cast<float>(y0(static_cast<double>(x)));
}

} // namespace pbsd::userland::msun
