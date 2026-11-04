module;
#include <cstddef>

export module pbsd.userland.msun.nexttoward;

export import pbsd.core;

/// scaffold from hbsd/src/lib/msun/src/s_nexttoward.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double nexttoward_val(double x, long double y) noexcept { (void)x; (void)y; return 0.0; }

} // namespace pbsd::userland::msun
