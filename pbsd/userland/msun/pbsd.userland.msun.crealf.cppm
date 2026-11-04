module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.crealf;

/// crealf from hbsd/src/lib/msun/src/s_crealf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double crealf_val(double x) noexcept { (void)x; return 0.0; }
[[nodiscard]] inline float crealff_val(float x) noexcept { (void)x; return 0.0f; }

} // namespace pbsd::userland::msun
