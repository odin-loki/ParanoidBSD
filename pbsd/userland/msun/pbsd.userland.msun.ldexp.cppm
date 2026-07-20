module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.ldexp;

/// ldexp from hbsd/src/lib/msun/src/s_ldexp.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double ldexp_val(double x, int exp) noexcept {
    std::uint64_t hx;
    std::memcpy(&hx, &x, sizeof(hx));
    const unsigned ue = static_cast<unsigned>((hx >> 52) & 0x7ffu);
    if (ue == 0 || ue == 0x7ffu) {
        return x;
    }
    const int ne = static_cast<int>(ue) + exp;
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

} // namespace pbsd::userland::msun
