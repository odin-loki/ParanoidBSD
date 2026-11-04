module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.conj;

/// conj from hbsd/src/lib/msun/src/s_conj.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double conj_val(double x) noexcept { (void)x; return 0.0; }
[[nodiscard]] inline float conjf_val(float x) noexcept { (void)x; return 0.0f; }

} // namespace pbsd::userland::msun
