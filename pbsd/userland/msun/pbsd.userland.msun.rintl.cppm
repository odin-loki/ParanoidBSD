module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.rintl;

/// rintl from hbsd/src/lib/msun/src/s_rintl.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline long rintl_val(long double x) noexcept { return static_cast<long>(x); }

} // namespace pbsd::userland::msun
