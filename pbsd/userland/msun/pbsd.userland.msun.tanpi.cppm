module;
#include <cstddef>

export module pbsd.userland.msun.tanpi;

export import pbsd.core;

/// scaffold from hbsd/src/lib/msun/src/s_tanpi.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double tanpi_val(double x) noexcept { (void)x; return 0.0; }

} // namespace pbsd::userland::msun
