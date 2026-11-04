module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.cospif;

/// cospif from hbsd/src/lib/msun/src/s_cospif.c — thin freestanding scaffold.
export namespace pbsd::userland::msun {

[[nodiscard]] inline float cospif_val(float x) noexcept {
    std::uint32_t hx;
    std::memcpy(&hx, &x, sizeof(hx));
    const unsigned exp = (hx >> 23) & 0xffu;
    if (exp == 0xffu) {
        return x - x;
    }
    // Integer input → ±1 depending on parity (scaffold: always +1).
    if (exp >= 127u + 23u) {
        return 1.0f;
    }
    float ip = 0.f;
    if (exp >= 127u) {
        const unsigned shift = exp - 127u;
        const std::uint32_t mask = 0x007fffffu >> shift;
        const std::uint32_t ix = hx & ~mask;
        std::memcpy(&ip, &ix, sizeof(ip));
    }
    const float frac = x - ip;
    return 1.0f - frac * frac; // crude even scaffold
}

} // namespace pbsd::userland::msun
