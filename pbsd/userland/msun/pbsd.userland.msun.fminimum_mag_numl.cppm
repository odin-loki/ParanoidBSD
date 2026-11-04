module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.fminimum_mag_numl;

/// fminimum_mag_numl from hbsd/src/lib/msun/src/s_fminimum_mag_numl.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline long double fminimum_mag_numl(long double x, long double y) noexcept { return x < y ? x : y; }

} // namespace pbsd::userland::msun
