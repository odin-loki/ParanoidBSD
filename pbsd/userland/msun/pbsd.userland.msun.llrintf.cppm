module;
#include <cstddef>

export module pbsd.userland.msun.llrintf;

export import pbsd.core;

/// scaffold from hbsd/src/lib/msun/src/s_llrintf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline long llrintf_val(float x) noexcept { (void)x; return 0L; }

} // namespace pbsd::userland::msun
