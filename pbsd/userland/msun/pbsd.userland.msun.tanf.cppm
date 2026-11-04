module;
#include <cmath>
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.tanf;

/// tanf from hbsd/src/lib/msun/src/e_tanf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline float tanf_val(float x) noexcept { return ::tanf(x); }

} // namespace pbsd::userland::msun
