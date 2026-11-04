module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.cimagf;

/// cimagf from hbsd/src/lib/msun/src/s_cimagf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double cimagf_val(double x) noexcept { (void)x; return 0.0; }
[[nodiscard]] inline float cimagff_val(float x) noexcept { (void)x; return 0.0f; }

} // namespace pbsd::userland::msun
