module;
#include <cstddef>

export module pbsd.userland.msun.significand;

export import pbsd.core;

/// scaffold from hbsd/src/lib/msun/src/s_significand.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double significand_val(double x) noexcept { (void)x; return 0.0; }

} // namespace pbsd::userland::msun
