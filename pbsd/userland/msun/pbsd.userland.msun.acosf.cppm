module;
#include <cmath>
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.acosf;

/// acosf from hbsd/src/lib/msun/src/e_acosf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline float acosf_val(float x) noexcept { return ::acosf(x); }

} // namespace pbsd::userland::msun
