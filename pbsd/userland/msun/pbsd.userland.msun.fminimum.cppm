module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.fminimum;

/// fminimum from hbsd/src/lib/msun/src/s_fminimum.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double fminimum(double x, double y) noexcept { return x < y ? x : y; }

} // namespace pbsd::userland::msun
