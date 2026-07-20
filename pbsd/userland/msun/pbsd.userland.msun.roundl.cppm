module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.roundl;

/// roundl from hbsd/src/lib/msun/src/s_roundl.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline long roundl_val(long double x) noexcept { return static_cast<long>(x); }

} // namespace pbsd::userland::msun
