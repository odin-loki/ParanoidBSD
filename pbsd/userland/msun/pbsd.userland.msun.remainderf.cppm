module;
#include <cmath>

export module pbsd.userland.msun.remainderf;

/// remainderf from hbsd/src/lib/msun/src/e_remainderf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline float remainderf_val(float x, float y) noexcept { return std::remainderf(x, y); }

} // namespace pbsd::userland::msun
