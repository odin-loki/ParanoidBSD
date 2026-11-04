module;
#include <cmath>
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.sinf;

/// sinf from hbsd/src/lib/msun/src/e_sinf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline float sinf_val(float x) noexcept { return ::sinf(x); }

} // namespace pbsd::userland::msun
