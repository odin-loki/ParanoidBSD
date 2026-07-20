module;
#include <cmath>
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.tanhf;

/// tanhf from hbsd/src/lib/msun/src/e_tanhf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline float tanhf_val(float x) noexcept { return ::tanhf(x); }

} // namespace pbsd::userland::msun
