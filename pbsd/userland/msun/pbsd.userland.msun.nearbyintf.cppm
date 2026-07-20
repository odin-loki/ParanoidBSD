module;
#include <cmath>

export module pbsd.userland.msun.nearbyintf;

/// nearbyintf from hbsd/src/lib/msun/src/s_nearbyintf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline float nearbyintf_val(float x) noexcept { return std::nearbyintf(x); }

} // namespace pbsd::userland::msun
