module;
#include <cmath>
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.expm1f;

/// expm1f from hbsd/src/lib/msun/src/e_expm1f.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline float expm1f_val(float x) noexcept { return ::expm1f(x); }

} // namespace pbsd::userland::msun
