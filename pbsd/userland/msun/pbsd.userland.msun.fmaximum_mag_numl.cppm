module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.fmaximum_mag_numl;

/// fmaximum_mag_numl from hbsd/src/lib/msun/src/s_fmaximum_mag_numl.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline long double fmaximum_mag_numl(long double x, long double y) noexcept { return x > y ? x : y; }

} // namespace pbsd::userland::msun
