module;

#include <cmath>

export module pbsd.userland.msun.cbrt;

/// cbrt/cbrtf from hbsd/src/lib/msun/src/s_cbrt.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double cbrt(double x) noexcept { return std::cbrt(x); }

[[nodiscard]] inline float cbrtf(float x) noexcept { return std::cbrtf(x); }

} // namespace pbsd::userland::msun
