module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.rsqrtl;

/// rsqrtl from hbsd/src/lib/msun/src/s_rsqrtl.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline long double rsqrtl_val(long double x) noexcept { return x; }

} // namespace pbsd::userland::msun
