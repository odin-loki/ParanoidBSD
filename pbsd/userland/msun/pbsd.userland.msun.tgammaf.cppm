module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.tgammaf;

/// tgammaf from hbsd/src/lib/msun/src/s_tgammaf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline float tgammaf_val(float x) noexcept { return x; }

} // namespace pbsd::userland::msun
