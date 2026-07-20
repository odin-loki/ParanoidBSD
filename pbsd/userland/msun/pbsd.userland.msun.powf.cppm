module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.powf;

/// powf from hbsd/src/lib/msun/src/e_powf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double powf_val(double x) noexcept { (void)x; return 0.0; }
[[nodiscard]] inline float powff_val(float x) noexcept { (void)x; return 0.0f; }

} // namespace pbsd::userland::msun
