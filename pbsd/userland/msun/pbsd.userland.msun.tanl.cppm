module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.tanl;

/// tanl from hbsd/src/lib/msun/src/s_tanl.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline long double tanl_val(long double x) noexcept { return x; }

} // namespace pbsd::userland::msun
