module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.modf;

/// modf/modff from hbsd/src/lib/msun/src/s_modf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double modf(double x, double* iptr) noexcept {
    std::uint64_t hx;
    std::memcpy(&hx, &x, sizeof(hx));
    const unsigned exp = static_cast<unsigned>((hx >> 52) & 0x7ffu);
    if (exp == 0x7ffu) {
        if (iptr != nullptr) {
            *iptr = x;
        }
        return x - x;
    }
    if (exp < 1023u) {
        if (iptr != nullptr) {
            const std::uint64_t sx = hx & 0x8000000000000000ULL;
            std::memcpy(iptr, &sx, sizeof(sx));
        }
        return x;
    }
    if (exp >= 1023u + 52u) {
        if (iptr != nullptr) {
            *iptr = x;
        }
        const std::uint64_t sx = hx & 0x8000000000000000ULL;
        double z;
        std::memcpy(&z, &sx, sizeof(z));
        return z;
    }
    const unsigned shift = exp - 1023u;
    const std::uint64_t mask = 0x000fffffffffffffULL >> shift;
    const std::uint64_t i_bits = hx & ~mask;
    double ipart;
    std::memcpy(&ipart, &i_bits, sizeof(ipart));
    if (iptr != nullptr) {
        *iptr = ipart;
    }
    return x - ipart;
}

[[nodiscard]] inline float modff(float x, float* iptr) noexcept {
    std::uint32_t hx;
    std::memcpy(&hx, &x, sizeof(hx));
    const unsigned exp = (hx >> 23) & 0xffu;
    if (exp == 0xffu) {
        if (iptr != nullptr) {
            *iptr = x;
        }
        return x - x;
    }
    if (exp < 127u) {
        if (iptr != nullptr) {
            const std::uint32_t sx = hx & 0x80000000u;
            std::memcpy(iptr, &sx, sizeof(sx));
        }
        return x;
    }
    if (exp >= 127u + 23u) {
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
