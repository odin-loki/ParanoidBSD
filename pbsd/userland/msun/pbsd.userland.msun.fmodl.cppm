module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.fmodl;

/// fmodl from hbsd/src/lib/msun/src/e_fmodl.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double fmodl_val(double x) noexcept { (void)x; return 0.0; }
[[nodiscard]] inline float fmodlf_val(float x) noexcept { (void)x; return 0.0f; }

} // namespace pbsd::userland::msun
