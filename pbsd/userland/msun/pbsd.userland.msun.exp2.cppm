module;

#include <cmath>

export module pbsd.userland.msun.exp2;

/// exp2/exp2f from hbsd/src/lib/msun/src/s_exp2.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double exp2(double x) noexcept { return std::exp2(x); }

[[nodiscard]] inline float exp2f(float x) noexcept { return std::exp2(x); }

} // namespace pbsd::userland::msun
