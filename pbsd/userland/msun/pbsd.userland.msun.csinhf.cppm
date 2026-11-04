module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.csinhf;

/// csinhf from hbsd/src/lib/msun/src/s_csinhf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double csinhf_val(double x) noexcept { (void)x; return 0.0; }
[[nodiscard]] inline float csinhff_val(float x) noexcept { (void)x; return 0.0f; }

} // namespace pbsd::userland::msun
