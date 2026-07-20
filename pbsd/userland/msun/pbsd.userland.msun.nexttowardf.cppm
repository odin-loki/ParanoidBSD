module;
#include <cstddef>

export module pbsd.userland.msun.nexttowardf;

export import pbsd.core;

/// scaffold from hbsd/src/lib/msun/src/s_nexttowardf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline float nexttowardf_val(float x, long double y) noexcept { (void)x; (void)y; return 0.0f; }

} // namespace pbsd::userland::msun
