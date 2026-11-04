module;
#include <cmath>
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.asinhf;

/// asinhf from hbsd/src/lib/msun/src/e_asinhf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline float asinhf_val(float x) noexcept { return ::asinhf(x); }

} // namespace pbsd::userland::msun
