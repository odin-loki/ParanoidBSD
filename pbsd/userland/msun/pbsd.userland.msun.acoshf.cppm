module;
#include <cmath>
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.acoshf;

/// acoshf from hbsd/src/lib/msun/src/e_acoshf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline float acoshf_val(float x) noexcept { return ::acoshf(x); }

} // namespace pbsd::userland::msun
