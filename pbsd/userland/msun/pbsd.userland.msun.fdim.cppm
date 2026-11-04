module;

#include <cmath>

export module pbsd.userland.msun.fdim;

/// fdim from hbsd/src/lib/msun/src/s_fdim.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double fdim(double x, double y) noexcept { return std::fdim(x, y); }

[[nodiscard]] inline float fdimf(float x, float y) noexcept { return std::fdim(x, y); }

} // namespace pbsd::userland::msun
