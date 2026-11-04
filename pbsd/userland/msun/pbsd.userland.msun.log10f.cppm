module;
#include <cmath>
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.log10f;

/// log10f from hbsd/src/lib/msun/src/e_log10f.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline float log10f_val(float x) noexcept { return ::log10f(x); }

} // namespace pbsd::userland::msun
