module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.sinl;

/// sinl from hbsd/src/lib/msun/src/s_sinl.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline long double sinl_val(long double x) noexcept { return x; }

} // namespace pbsd::userland::msun
