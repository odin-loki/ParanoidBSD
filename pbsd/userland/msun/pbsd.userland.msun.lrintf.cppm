module;
#include <cmath>
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.lrintf;

/// lrintf from hbsd/src/lib/msun/src/s_lrintf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline float lrintf_val(float x) noexcept { return ::lrintf(x); }

} // namespace pbsd::userland::msun
