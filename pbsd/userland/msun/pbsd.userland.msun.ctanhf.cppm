module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.ctanhf;

/// ctanhf from hbsd/src/lib/msun/src/s_ctanhf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double ctanhf_val(double x) noexcept { (void)x; return 0.0; }
[[nodiscard]] inline float ctanhff_val(float x) noexcept { (void)x; return 0.0f; }

} // namespace pbsd::userland::msun
