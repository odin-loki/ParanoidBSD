module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.fmaximum;

/// fmaximum from hbsd/src/lib/msun/src/s_fmaximum.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double fmaximum(double x, double y) noexcept { return x > y ? x : y; }

} // namespace pbsd::userland::msun
