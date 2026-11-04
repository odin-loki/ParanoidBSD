module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.tanhl;

/// tanhl from hbsd/src/lib/msun/src/s_tanhl.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline long double tanhl_val(long double x) noexcept { return x; }

} // namespace pbsd::userland::msun
