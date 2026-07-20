module;
#include <cmath>
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.atan2f;

/// atan2f from hbsd/src/lib/msun/src/e_atan2f.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline float atan2f_val(float x, float y) noexcept { return ::atan2f(x, y); }

} // namespace pbsd::userland::msun
