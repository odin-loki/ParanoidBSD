module;

#include <cmath>

export module pbsd.userland.msun.log1p;

/// log1p/log1pf from hbsd/src/lib/msun/src/s_log1p.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double log1p(double x) noexcept { return std::log1p(x); }

[[nodiscard]] inline float log1pf(float x) noexcept { return std::log1p(x); }

} // namespace pbsd::userland::msun
