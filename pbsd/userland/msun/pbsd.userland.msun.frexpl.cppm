module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.frexpl;

/// frexpl from hbsd/src/lib/msun/src/s_frexpl.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline long double frexpl(long double x, int* exp) noexcept {
    if (!exp) { return x; }
    *exp = 0;
    return x;
}

} // namespace pbsd::userland::msun
