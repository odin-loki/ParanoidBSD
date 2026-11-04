module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.nextafterf;

/// nextafterf from hbsd/src/lib/msun/src/e_nextafterf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline float nextafterf_val(float x, float y) noexcept {
    std::uint32_t hx;
    std::uint32_t hy;
    std::memcpy(&hx, &x, sizeof(hx));
    std::memcpy(&hy, &y, sizeof(hy));
    if (((hx & 0x7fffffffu) > 0x7f800000u) || ((hy & 0x7fffffffu) > 0x7f800000u)) {
        return x + y; // NaN
    }
    if (hx == hy) {
        return y;
    }
    if ((hx & 0x7fffffffu) == 0) {
        // x is ±0 → smallest subnormal toward y
        const std::uint32_t r = (hy & 0x80000000u) | 1u;
        float z;
        std::memcpy(&z, &r, sizeof(z));
        return z;
    }
    if ((hx & 0x80000000u) == 0u) {
        if (hx > hy) {
            --hx;
        } else {
            ++hx;
        }
    } else {
        if (hy < 0u || hx > hy) {
            ++hx;
        } else {
            --hx;
        }
    }
    float z;
    std::memcpy(&z, &hx, sizeof(z));
    return z;
}

} // namespace pbsd::userland::msun
