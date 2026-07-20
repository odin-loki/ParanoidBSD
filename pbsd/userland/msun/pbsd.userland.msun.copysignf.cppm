module;
#include <cmath>

export module pbsd.userland.msun.copysignf;

/// copysignf from hbsd/src/lib/msun/src/s_copysignf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline float copysignf_val(float x, float y) noexcept { return std::copysignf(x, y); }

} // namespace pbsd::userland::msun
