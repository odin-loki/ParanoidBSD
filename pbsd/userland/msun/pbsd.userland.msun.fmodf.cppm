module;
#include <cmath>
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.fmodf;

/// fmodf from hbsd/src/lib/msun/src/e_fmodf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline float fmodf_val(float x, float y) noexcept { return ::fmodf(x, y); }

} // namespace pbsd::userland::msun
