module;
#include <cstddef>

export module pbsd.userland.msun.llrint;

export import pbsd.core;

/// scaffold from hbsd/src/lib/msun/src/s_llrint.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline long llrint_val(double x) noexcept { (void)x; return 0L; }

} // namespace pbsd::userland::msun
