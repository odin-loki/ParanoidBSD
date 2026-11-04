module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.lrintl;

/// lrintl from hbsd/src/lib/msun/src/s_lrintl.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline long lrintl_val(long double x) noexcept { return static_cast<long>(x); }

} // namespace pbsd::userland::msun
