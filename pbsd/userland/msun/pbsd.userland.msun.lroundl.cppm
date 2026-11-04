module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.lroundl;

/// lroundl from hbsd/src/lib/msun/src/s_lroundl.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline long lroundl_val(long double x) noexcept { return static_cast<long>(x); }

} // namespace pbsd::userland::msun
