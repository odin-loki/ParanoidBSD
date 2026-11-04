module;
#include <cmath>
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.expf;

/// expf from hbsd/src/lib/msun/src/e_expf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline float expf_val(float x) noexcept { return ::expf(x); }

} // namespace pbsd::userland::msun
