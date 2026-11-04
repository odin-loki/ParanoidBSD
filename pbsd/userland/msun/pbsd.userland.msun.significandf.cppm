module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.significandf;

/// significandf from hbsd/src/lib/msun/src/s_significandf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline float significandf_val(float x) noexcept { return x; }

} // namespace pbsd::userland::msun
