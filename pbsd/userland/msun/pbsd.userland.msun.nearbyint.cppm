module;

#include <cmath>

export module pbsd.userland.msun.nearbyint;

/// nearbyint/nearbyintf from hbsd/src/lib/msun/src/s_nearbyint.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double nearbyint(double x) noexcept { return std::nearbyint(x); }

[[nodiscard]] inline float nearbyintf(float x) noexcept { return std::nearbyintf(x); }

} // namespace pbsd::userland::msun
