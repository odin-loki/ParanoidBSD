module;

#include <cmath>

export module pbsd.userland.msun.rintf;

/// rintf from hbsd/src/lib/msun/src/s_rintf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline float rintf(float x) noexcept { return std::nearbyint(x); }

} // namespace pbsd::userland::msun
