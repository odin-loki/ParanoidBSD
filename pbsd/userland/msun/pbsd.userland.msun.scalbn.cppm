module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.scalbn;

/// scalbn/scalbnf from hbsd/src/lib/msun/src/s_scalbn.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double scalbn(double x, int n) noexcept {
    std::uint64_t hx;
    std::memcpy(&hx, &x, sizeof(hx));
    const unsigned ue = static_cast<unsigned>((hx >> 52) & 0x7ffu);
    if (ue == 0 || ue == 0x7ffu) {
        return x;
    }
    const int ne = static_cast<int>(ue) + n;
    if (ne <= 0) {
        const std::uint64_t sx = hx & 0x8000000000000000ULL;
        std::memcpy(&x, &sx, sizeof(x));
        return x;
    }
    if (ne >= 0x7ff) {
        const std::uint64_t sx = (hx & 0x8000000000000000ULL) | 0x7ff0000000000000ULL;
        std::memcpy(&x, &sx, sizeof(x));
        return x;
    }
    hx = (hx & 0x800fffffffffffffULL) | (static_cast<std::uint64_t>(ne) << 52);
    std::memcpy(&x, &hx, sizeof(x));
    return x;
}

[[nodiscard]] inline float scalbnf(float x, int n) noexcept {
    std::uint32_t hx;
    std::memcpy(&hx, &x, sizeof(hx));
    const unsigned ue = (hx >> 23) & 0xffu;
    if (ue == 0 || ue == 0xffu) {
        return x;
    }
    const int ne = static_cast<int>(ue) + n;
    if (ne <= 0) {
        const std::uint32_t sx = hx & 0x80000000u;
        float z;
        std::memcpy(&z, &sx, sizeof(z));
        return z;
    }
    if (ne >= 0xff) {
        const std::uint32_t sx = (hx & 0x80000000u) | 0x7f800000u;
        float z;
        std::memcpy(&z, &sx, sizeof(z));
        return z;
    }
    hx = (hx & 0x807fffffu) | (static_cast<std::uint32_t>(ne) << 23);
    std::memcpy(&x, &hx, sizeof(x));
    return x;
}

} // namespace pbsd::userland::msun
