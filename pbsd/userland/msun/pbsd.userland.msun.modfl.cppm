module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.modfl;

/// modfl from hbsd/src/lib/msun/src/s_modfl.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline long double modfl(long double x, long double* iptr) noexcept {
    if (iptr != nullptr) {
        *iptr = x;
    }
    return 0.0L;
}

} // namespace pbsd::userland::msun
