module;
#include <cmath>
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.y0f;

/// y0f from hbsd/src/lib/msun/src/e_y0f.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline float y0f_val(float x) noexcept { return ::y0f(x); }

} // namespace pbsd::userland::msun
