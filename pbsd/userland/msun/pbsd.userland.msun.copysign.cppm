module;

#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.copysign;

/// copysign/copysignf from hbsd/src/lib/msun/src/s_copysign.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double copysign(double x, double y) noexcept {
    std::uint64_t hx;
    std::uint64_t hy;
    std::memcpy(&hx, &x, sizeof(hx));
    std::memcpy(&hy, &y, sizeof(hy));
    hx = (hx & 0x7FFFFFFFFFFFFFFFULL) | (hy & 0x8000000000000000ULL);
    std::memcpy(&x, &hx, sizeof(x));
    return x;
}

[[nodiscard]] inline float copysignf(float x, float y) noexcept {
    std::uint32_t hx;
    std::uint32_t hy;
    std::memcpy(&hx, &x, sizeof(hx));
    std::memcpy(&hy, &y, sizeof(hy));
    hx = (hx & 0x7FFFFFFFU) | (hy & 0x80000000U);
    std::memcpy(&x, &hx, sizeof(x));
    return x;
}

} // namespace pbsd::userland::msun
