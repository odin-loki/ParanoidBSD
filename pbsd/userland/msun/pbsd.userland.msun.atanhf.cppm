module;
#include <cmath>
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.atanhf;

/// atanhf from hbsd/src/lib/msun/src/e_atanhf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline float atanhf_val(float x) noexcept { return ::atanhf(x); }

} // namespace pbsd::userland::msun
