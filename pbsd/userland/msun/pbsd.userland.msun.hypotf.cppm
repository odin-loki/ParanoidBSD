module;
#include <cmath>

export module pbsd.userland.msun.hypotf;

/// hypotf from hbsd/src/lib/msun/src/e_hypotf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline float hypotf_val(float x, float y) noexcept { return std::hypotf(x, y); }

} // namespace pbsd::userland::msun
