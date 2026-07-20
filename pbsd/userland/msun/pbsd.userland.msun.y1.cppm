module;
#include <cmath>

export module pbsd.userland.msun.y1;

/// y1 from hbsd/src/lib/msun/src/e_y1.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double y1(double x) noexcept {
#if defined(__cpp_lib_math_special_functions) && __cpp_lib_math_special_functions >= 201603L
    return std::cyl_neumann(1, x);
#else
    (void)x;
    return 0.0;
#endif
}
[[nodiscard]] inline float y1f(float x) noexcept {
    return static_cast<float>(y1(static_cast<double>(x)));
}

} // namespace pbsd::userland::msun
