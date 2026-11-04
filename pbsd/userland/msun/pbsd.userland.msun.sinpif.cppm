module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.sinpif;

/// sinpif from hbsd/src/lib/msun/src/s_sinpif.c — thin freestanding scaffold.
export namespace pbsd::userland::msun {

[[nodiscard]] inline float sinpif_val(float x) noexcept {
    // Reduce mod 2 via integer part; scaffold returns exact zeros on integers.
    std::uint32_t hx;
    std::memcpy(&hx, &x, sizeof(hx));
    const unsigned exp = (hx >> 23) & 0xffu;
    if (exp == 0xffu) {
        return x - x; // NaN
    }
    // Integer input → 0 (sin(n*π)=0)
    if (exp >= 127u + 23u) {
        const std::uint32_t sx = hx & 0x80000000u;
        float z;
        std::memcpy(&z, &sx, sizeof(z));
        return z;
    }
    // Non-integer: return a crude odd-function scaffold proportional to frac.
    float ip = 0.f;
    std::uint32_t ix = hx;
    if (exp >= 127u) {
        const unsigned shift = exp - 127u;
        const std::uint32_t mask = 0x007fffffu >> shift;
        ix = hx & ~mask;
        std::memcpy(&ip, &ix, sizeof(ip));
    }
    const float frac = x - ip;
    return frac; // placeholder odd map; replaced by full kernel later
}

} // namespace pbsd::userland::msun
