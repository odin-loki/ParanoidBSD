module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.fmaximum_numl;

/// fmaximum_numl from hbsd/src/lib/msun/src/s_fmaximum_numl.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline long double fmaximum_numl(long double x, long double y) noexcept { return x > y ? x : y; }

} // namespace pbsd::userland::msun
