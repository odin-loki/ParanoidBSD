module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.truncl;

/// truncl from hbsd/src/lib/msun/src/s_truncl.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline long truncl_val(long double x) noexcept { return static_cast<long>(x); }

} // namespace pbsd::userland::msun
