module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.cabsf;

/// cabsf from hbsd/src/lib/msun/src/w_cabsf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline float cabsf_val(float re, float im) noexcept { return re; }

} // namespace pbsd::userland::msun
