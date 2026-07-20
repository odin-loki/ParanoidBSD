module;
#include <cmath>
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.log1pf;

/// log1pf from hbsd/src/lib/msun/src/e_log1pf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline float log1pf_val(float x) noexcept { return ::log1pf(x); }

} // namespace pbsd::userland::msun
