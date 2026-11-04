module;
#include <cmath>
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.ilogbf;

/// ilogbf from hbsd/src/lib/msun/src/s_ilogbf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline float ilogbf_val(float x) noexcept { return ::ilogbf(x); }

} // namespace pbsd::userland::msun
