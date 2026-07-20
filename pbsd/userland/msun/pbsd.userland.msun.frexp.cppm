module;

#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.frexp;

/// frexp/frexpf from hbsd/src/lib/msun/src/s_frexp.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double frexp(double x, int* exp) noexcept {
    std::uint64_t u;
    std::memcpy(&u, &x, sizeof(u));
    const int e = static_cast<int>((u >> 52) & 0x7ff) - 1022;
    if (((u >> 52) & 0x7ff) == 0) {
        if (exp != nullptr) {
            *exp = 0;
        }
        return x;
    }
    u &= 0x800fffffffffffffULL;
    u |= static_cast<std::uint64_t>(1022) << 52;
    std::memcpy(&x, &u, sizeof(u));
    if (exp != nullptr) {
        *exp = e;
    }
    return x;
}

[[nodiscard]] inline float frexpf(float x, int* exp) noexcept {
    std::uint32_t u;
    std::memcpy(&u, &x, sizeof(u));
    const int e = static_cast<int>((u >> 23) & 0xff) - 126;
    if (((u >> 23) & 0xff) == 0) {
        if (exp != nullptr) {
            *exp = 0;
        }
        return x;
    }
    u &= 0x807fffffU;
    u |= 126U << 23;
    std::memcpy(&x, &u, sizeof(x));
    if (exp != nullptr) {
        *exp = e;
    }
    return x;
}

} // namespace pbsd::userland::msun
