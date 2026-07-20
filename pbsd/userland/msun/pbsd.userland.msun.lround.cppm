module;
#include <cstddef>

export module pbsd.userland.msun.lround;

export import pbsd.core;

/// scaffold from hbsd/src/lib/msun/src/s_lround.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline long lround_val(double x) noexcept { (void)x; return 0L; }

} // namespace pbsd::userland::msun
