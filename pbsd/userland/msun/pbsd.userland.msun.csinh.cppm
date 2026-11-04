module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.csinh;

/// csinh from hbsd/src/lib/msun/src/s_csinh.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double csinh_val(double x) noexcept { (void)x; return 0.0; }
[[nodiscard]] inline float csinhf_val(float x) noexcept { (void)x; return 0.0f; }

} // namespace pbsd::userland::msun
