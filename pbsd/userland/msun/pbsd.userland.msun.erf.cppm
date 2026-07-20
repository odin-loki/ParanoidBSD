module;

#include <cmath>

export module pbsd.userland.msun.erf;

/// erf/erff from hbsd/src/lib/msun/src/s_erf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double erf(double x) noexcept { return std::erf(x); }

[[nodiscard]] inline float erff(float x) noexcept { return std::erff(x); }

} // namespace pbsd::userland::msun
