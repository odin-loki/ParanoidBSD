module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.fmaximum_mag;

/// fmaximum_mag from hbsd/src/lib/msun/src/s_fmaximum_mag.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double fmaximum_mag(double x, double y) noexcept { return x > y ? x : y; }

} // namespace pbsd::userland::msun
