module;
#include <cstddef>

export module pbsd.userland.msun.scalbnl;

export import pbsd.core;

/// scaffold from hbsd/src/lib/msun/src/s_scalbnl.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline long double scalbnl_val(long double x, int n) noexcept { (void)x; (void)n; return 0.0L; }

} // namespace pbsd::userland::msun
