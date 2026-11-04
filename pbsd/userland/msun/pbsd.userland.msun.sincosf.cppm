module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.sincosf;

/// sincosf from hbsd/src/lib/msun/src/s_sincosf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline void sincosf_val(float x, float* s, float* c) noexcept {
    if (s) *s = x;
    if (c) *c = x;
}

} // namespace pbsd::userland::msun
