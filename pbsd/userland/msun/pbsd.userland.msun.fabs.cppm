module;

#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.fabs;

/// fabs from hbsd/src/lib/msun/src/s_fabs.c (IEEE754 sign clear)
export namespace pbsd::userland::msun {

[[nodiscard]] inline double fabs(double x) noexcept {
    std::uint64_t u;
    std::memcpy(&u, &x, sizeof(u));
    u &= 0x7FFFFFFFFFFFFFFFULL;
    std::memcpy(&x, &u, sizeof(u));
    return x;
}

[[nodiscard]] inline float fabsf(float x) noexcept {
    std::uint32_t u;
    std::memcpy(&u, &x, sizeof(u));
    u &= 0x7FFFFFFFU;
    std::memcpy(&x, &u, sizeof(u));
    return x;
}

} // namespace pbsd::userland::msun
