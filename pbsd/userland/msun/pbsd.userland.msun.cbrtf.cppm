module;
#include <cmath>
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.cbrtf;

/// cbrtf from hbsd/src/lib/msun/src/e_cbrtf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline float cbrtf_val(float x) noexcept { return ::cbrtf(x); }

} // namespace pbsd::userland::msun
