module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.ccoshf;

/// ccoshf from hbsd/src/lib/msun/src/s_ccoshf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double ccoshf_val(double x) noexcept { (void)x; return 0.0; }
[[nodiscard]] inline float ccoshff_val(float x) noexcept { (void)x; return 0.0f; }

} // namespace pbsd::userland::msun
