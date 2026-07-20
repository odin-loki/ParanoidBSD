module;
#include <cmath>
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.logbf;

/// logbf from hbsd/src/lib/msun/src/s_logbf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline float logbf_val(float x) noexcept { return ::logbf(x); }

} // namespace pbsd::userland::msun
