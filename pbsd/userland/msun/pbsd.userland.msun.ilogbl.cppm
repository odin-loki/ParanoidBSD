module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.ilogbl;

/// ilogbl from hbsd/src/lib/msun/src/s_ilogbl.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline int ilogbl(long double x) noexcept { return x == 0.0L ? -5000 : 0; }

} // namespace pbsd::userland::msun
