module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.fmaximum_num;

/// fmaximum_num from hbsd/src/lib/msun/src/s_fmaximum_num.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double fmaximum_num(double x, double y) noexcept { return x > y ? x : y; }

} // namespace pbsd::userland::msun
