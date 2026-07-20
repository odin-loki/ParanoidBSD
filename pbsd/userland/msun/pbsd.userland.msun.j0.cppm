module;
#include <cmath>

export module pbsd.userland.msun.j0;

/// j0/j0f Bessel from hbsd/src/lib/msun/src/e_j0.c (delegates to libm).
export namespace pbsd::userland::msun {

[[nodiscard]] inline double j0(double x) noexcept {
#if defined(__cpp_lib_math_special_functions) && __cpp_lib_math_special_functions >= 201603L
    return std::cyl_bessel_j(0, x);
#else
    (void)x;
    return 0.0;
#endif
}

[[nodiscard]] inline float j0f(float x) noexcept {
    return static_cast<float>(j0(static_cast<double>(x)));
}

} // namespace pbsd::userland::msun
