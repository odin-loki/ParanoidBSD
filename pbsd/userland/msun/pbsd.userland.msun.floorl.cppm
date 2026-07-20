module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.floorl;

/// floorl from hbsd/src/lib/msun/src/s_floorl.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline long double floorl_val(long double x) noexcept { return x; }

} // namespace pbsd::userland::msun
