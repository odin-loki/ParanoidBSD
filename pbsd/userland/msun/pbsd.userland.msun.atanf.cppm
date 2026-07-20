module;
#include <cmath>
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.atanf;

/// atanf from hbsd/src/lib/msun/src/e_atanf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline float atanf_val(float x) noexcept { return ::atanf(x); }

} // namespace pbsd::userland::msun
