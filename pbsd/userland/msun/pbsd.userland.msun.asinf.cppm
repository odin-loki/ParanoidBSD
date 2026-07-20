module;
#include <cmath>
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.asinf;

/// asinf from hbsd/src/lib/msun/src/e_asinf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline float asinf_val(float x) noexcept { return ::asinf(x); }

} // namespace pbsd::userland::msun
