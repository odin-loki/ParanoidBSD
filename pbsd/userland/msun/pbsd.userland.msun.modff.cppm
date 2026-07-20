module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.modff;

/// modff from hbsd/src/lib/msun/src/s_modff.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline float modff_val(float x, float* iptr) noexcept {
    std::uint32_t hx;
    std::memcpy(&hx, &x, sizeof(hx));
    const unsigned exp = (hx >> 23) & 0xffu;

    if (exp == 0xffu) {
        // NaN or Inf
        if (iptr != nullptr) {
            *iptr = x;
        }
        return x - x;
    }
    if (exp < 127u) {
        // |x| < 1 → integer part is signed zero
        if (iptr != nullptr) {
            const std::uint32_t sx = hx & 0x80000000u;
            std::memcpy(iptr, &sx, sizeof(sx));
        }
        return x;
    }
    if (exp >= 127u + 23u) {
        // no fractional bits
        if (iptr != nullptr) {
            *iptr = x;
        }
        const std::uint32_t sx = hx & 0x80000000u;
        float z;
        std::memcpy(&z, &sx, sizeof(z));
        return z;
    }

    const unsigned shift = exp - 127u;
    const std::uint32_t mask = 0x007fffffu >> shift;
    const std::uint32_t i_bits = hx & ~mask;
    float ipart;
    std::memcpy(&ipart, &i_bits, sizeof(ipart));
    if (iptr != nullptr) {
        *iptr = ipart;
    }
    return x - ipart;
}

} // namespace pbsd::userland::msun
