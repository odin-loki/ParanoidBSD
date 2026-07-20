module;
#include <cmath>
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.log2f;

/// log2f from hbsd/src/lib/msun/src/e_log2f.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline float log2f_val(float x) noexcept { return ::log2f(x); }

} // namespace pbsd::userland::msun
