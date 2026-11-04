module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.fminimuml;

/// fminimuml from hbsd/src/lib/msun/src/s_fminimuml.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline long double fminimuml(long double x, long double y) noexcept { return x < y ? x : y; }

} // namespace pbsd::userland::msun
