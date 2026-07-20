module;
#include <cmath>
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.j0f;

/// j0f from hbsd/src/lib/msun/src/e_j0f.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline float j0f_val(float x) noexcept { return ::j0f(x); }

} // namespace pbsd::userland::msun
