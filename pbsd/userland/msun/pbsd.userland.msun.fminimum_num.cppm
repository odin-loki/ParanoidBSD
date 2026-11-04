module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.fminimum_num;

/// fminimum_num from hbsd/src/lib/msun/src/s_fminimum_num.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double fminimum_num(double x, double y) noexcept { return x < y ? x : y; }

} // namespace pbsd::userland::msun
