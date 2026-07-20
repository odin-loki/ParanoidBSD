module;
#include <cmath>
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.sinhf;

/// sinhf from hbsd/src/lib/msun/src/e_sinhf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline float sinhf_val(float x) noexcept { return ::sinhf(x); }

} // namespace pbsd::userland::msun
