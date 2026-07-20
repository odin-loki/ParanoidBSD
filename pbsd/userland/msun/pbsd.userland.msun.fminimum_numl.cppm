module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.fminimum_numl;

/// fminimum_numl from hbsd/src/lib/msun/src/s_fminimum_numl.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline long double fminimum_numl(long double x, long double y) noexcept { return x < y ? x : y; }

} // namespace pbsd::userland::msun
