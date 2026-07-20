module;
#include <cstddef>

export module pbsd.userland.msun.rsqrt;

export import pbsd.core;

/// scaffold from hbsd/src/lib/msun/src/s_rsqrt.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double rsqrt_val(double x) noexcept { (void)x; return 0.0; }

} // namespace pbsd::userland::msun
