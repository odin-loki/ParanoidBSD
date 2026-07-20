module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.fminimum_magl;

/// fminimum_magl from hbsd/src/lib/msun/src/s_fminimum_magl.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline long double fminimum_magl(long double x, long double y) noexcept { return x < y ? x : y; }

} // namespace pbsd::userland::msun
