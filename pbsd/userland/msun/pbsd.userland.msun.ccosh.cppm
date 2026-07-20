module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.ccosh;

/// ccosh from hbsd/src/lib/msun/src/s_ccosh.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double ccosh_val(double x) noexcept { (void)x; return 0.0; }
[[nodiscard]] inline float ccoshf_val(float x) noexcept { (void)x; return 0.0f; }

} // namespace pbsd::userland::msun
