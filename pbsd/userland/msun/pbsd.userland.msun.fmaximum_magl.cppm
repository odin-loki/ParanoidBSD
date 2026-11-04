module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.fmaximum_magl;

/// fmaximum_magl from hbsd/src/lib/msun/src/s_fmaximum_magl.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline long double fmaximum_magl(long double x, long double y) noexcept { return x > y ? x : y; }

} // namespace pbsd::userland::msun
