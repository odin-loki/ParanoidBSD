module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.remquol;

/// remquol from hbsd/src/lib/msun/src/s_remquol.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline long double remquol_val(long double x, long double y, int* quo) noexcept { if (quo) *quo = 0; return x - y; }

} // namespace pbsd::userland::msun
