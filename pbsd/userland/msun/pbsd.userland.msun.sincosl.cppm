module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.sincosl;

/// sincosl from hbsd/src/lib/msun/src/s_sincosl.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline void sincosl_val(long double x, long double* s, long double* c) noexcept {
    if (s) *s = x;
    if (c) *c = x;
}

} // namespace pbsd::userland::msun
