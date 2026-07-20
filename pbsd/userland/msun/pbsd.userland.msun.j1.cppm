module;
#include <cmath>

export module pbsd.userland.msun.j1;

/// j1 from hbsd/src/lib/msun/src/e_j1.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double j1(double x) noexcept {
#if defined(__cpp_lib_math_special_functions) && __cpp_lib_math_special_functions >= 201603L
    return std::cyl_bessel_j(1, x);
#else
    (void)x;
    return 0.0;
#endif
}
[[nodiscard]] inline float j1f(float x) noexcept {
    return static_cast<float>(j1(static_cast<double>(x)));
}

} // namespace pbsd::userland::msun
