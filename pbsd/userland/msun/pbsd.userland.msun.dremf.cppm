module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.dremf;

/// dremf from hbsd/src/lib/msun/src/w_dremf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline float dremf_val(float x, float y) noexcept { return x - y; }

} // namespace pbsd::userland::msun
