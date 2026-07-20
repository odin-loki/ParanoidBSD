module;
#include <cstddef>

export module pbsd.userland.msun.sinpi;

export import pbsd.core;

/// scaffold from hbsd/src/lib/msun/src/s_sinpi.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double sinpi_val(double x) noexcept { (void)x; return 0.0; }

} // namespace pbsd::userland::msun
