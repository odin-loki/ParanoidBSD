module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.llrintl;

/// llrintl from hbsd/src/lib/msun/src/s_llrintl.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline long long llrintl_val(long double x) noexcept { return static_cast<long long>(x); }

} // namespace pbsd::userland::msun
