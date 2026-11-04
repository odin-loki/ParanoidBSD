module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.logbl;

/// logbl from hbsd/src/lib/msun/src/s_logbl.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline long double logbl(long double x) noexcept { return x == 0.0L ? -5000.0L : 0.0L; }

} // namespace pbsd::userland::msun
