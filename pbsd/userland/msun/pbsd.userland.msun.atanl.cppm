module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.atanl;

/// atanl from hbsd/src/lib/msun/src/e_atanl.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double atanl_val(double x) noexcept { (void)x; return 0.0; }
[[nodiscard]] inline float atanlf_val(float x) noexcept { (void)x; return 0.0f; }

} // namespace pbsd::userland::msun
