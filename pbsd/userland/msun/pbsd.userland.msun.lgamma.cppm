module;

#include <cmath>

export module pbsd.userland.msun.lgamma;

/// lgamma/lgammaf from hbsd/src/lib/msun/src/e_lgamma.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double lgamma(double x) noexcept { return std::lgamma(x); }

[[nodiscard]] inline float lgammaf(float x) noexcept { return std::lgammaf(x); }

} // namespace pbsd::userland::msun
