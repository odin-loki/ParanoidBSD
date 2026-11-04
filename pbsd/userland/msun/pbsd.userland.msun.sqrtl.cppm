module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.sqrtl;

/// sqrtl from hbsd/src/lib/msun/src/e_sqrtl.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline long double sqrtl_val(long double x) noexcept { return x; }

} // namespace pbsd::userland::msun
