module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.catrig;

/// catrig from hbsd/src/lib/msun/src/s_catrig.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double catrig_val(double x) noexcept { (void)x; return 0.0; }
[[nodiscard]] inline float catrigf_val(float x) noexcept { (void)x; return 0.0f; }

} // namespace pbsd::userland::msun
