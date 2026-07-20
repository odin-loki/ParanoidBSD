module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.fminimum_mag_num;

/// fminimum_mag_num from hbsd/src/lib/msun/src/s_fminimum_mag_num.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double fminimum_mag_num(double x, double y) noexcept { return x < y ? x : y; }

} // namespace pbsd::userland::msun
