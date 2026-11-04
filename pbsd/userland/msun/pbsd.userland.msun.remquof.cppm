module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.remquof;

/// remquof from hbsd/src/lib/msun/src/s_remquof.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline float remquof_val(float x, float y, int* quo) noexcept { if (quo) *quo = 0; return x - y; }

} // namespace pbsd::userland::msun
