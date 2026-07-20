module;
#include <cmath>
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.lroundf;

/// lroundf from hbsd/src/lib/msun/src/s_lroundf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline float lroundf_val(float x) noexcept { return ::lroundf(x); }

} // namespace pbsd::userland::msun
