module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.fmaxl;

/// fmaxl from hbsd/src/lib/msun/src/s_fmaxl.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline long double fmaxl(long double x, long double y) noexcept { return x > y ? x : y; }

} // namespace pbsd::userland::msun
