module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.creall;

/// creall from hbsd/src/lib/msun/src/s_creall.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double creall_val(double x) noexcept { (void)x; return 0.0; }
[[nodiscard]] inline float creallf_val(float x) noexcept { (void)x; return 0.0f; }

} // namespace pbsd::userland::msun
