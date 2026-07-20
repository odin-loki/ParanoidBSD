module;
#include <cmath>
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.sqrtf;

/// sqrtf from hbsd/src/lib/msun/src/e_sqrtf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline float sqrtf_val(float x) noexcept { return ::sqrtf(x); }

} // namespace pbsd::userland::msun
