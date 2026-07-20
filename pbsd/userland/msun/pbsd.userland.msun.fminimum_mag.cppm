module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.fminimum_mag;

/// fminimum_mag from hbsd/src/lib/msun/src/s_fminimum_mag.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double fminimum_mag(double x, double y) noexcept { return x < y ? x : y; }

} // namespace pbsd::userland::msun
