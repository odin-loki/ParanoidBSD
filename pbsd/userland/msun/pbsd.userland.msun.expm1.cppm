module;

#include <cmath>

export module pbsd.userland.msun.expm1;

/// expm1/expm1f from hbsd/src/lib/msun/src/s_expm1.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double expm1(double x) noexcept { return std::expm1(x); }

[[nodiscard]] inline float expm1f(float x) noexcept { return std::expm1f(x); }

} // namespace pbsd::userland::msun
