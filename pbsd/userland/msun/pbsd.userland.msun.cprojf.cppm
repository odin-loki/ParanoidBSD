module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.cprojf;

/// cprojf from hbsd/src/lib/msun/src/s_cprojf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double cprojf_val(double x) noexcept { (void)x; return 0.0; }
[[nodiscard]] inline float cprojff_val(float x) noexcept { (void)x; return 0.0f; }

} // namespace pbsd::userland::msun
