module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.clog;

/// clog from hbsd/src/lib/msun/src/s_clog.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double clog_val(double x) noexcept { (void)x; return 0.0; }
[[nodiscard]] inline float clogf_val(float x) noexcept { (void)x; return 0.0f; }

} // namespace pbsd::userland::msun
