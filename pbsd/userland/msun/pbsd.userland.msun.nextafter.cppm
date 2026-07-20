module;

#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.nextafter;

/// nextafter/nextafterf from hbsd/src/lib/msun/src/s_nextafter.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double nextafter(double x, double y) noexcept {
    if (x == y) {
        return y;
    }
    std::uint64_t ux;
    std::memcpy(&ux, &x, sizeof(ux));
    if (ux == 0) {
        std::uint64_t hy;
        std::memcpy(&hy, &y, sizeof(hy));
        ux = (hy & 0x8000000000000000ULL) | 1ULL;
        std::memcpy(&x, &ux, sizeof(x));
        return x;
    }
    const bool upward = (x < y) ^ ((ux >> 63) != 0);
    if (upward) {
        ++ux;
    } else {
        --ux;
    }
    std::memcpy(&x, &ux, sizeof(x));
    return x;
}

[[nodiscard]] inline float nextafterf(float x, float y) noexcept {
    if (x == y) {
        return y;
    }
    std::uint32_t ux;
    std::memcpy(&ux, &x, sizeof(ux));
    if (ux == 0) {
        std::uint32_t hy;
        std::memcpy(&hy, &y, sizeof(hy));
        ux = (hy & 0x80000000U) | 1U;
        std::memcpy(&x, &ux, sizeof(x));
        return x;
    }
    const bool upward = (x < y) ^ ((ux >> 31) != 0);
    if (upward) {
        ++ux;
    } else {
        --ux;
    }
    std::memcpy(&x, &ux, sizeof(x));
    return x;
}

} // namespace pbsd::userland::msun
