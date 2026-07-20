module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.ceill;

/// ceill from hbsd/src/lib/msun/src/s_ceill.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline long double ceill_val(long double x) noexcept { return x; }

} // namespace pbsd::userland::msun
