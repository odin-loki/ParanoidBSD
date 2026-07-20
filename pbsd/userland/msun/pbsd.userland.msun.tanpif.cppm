module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.tanpif;

/// tanpif from hbsd/src/lib/msun/src/s_tanpif.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline float tanpif_val(float x) noexcept {
    std::uint32_t hx;
    std::memcpy(&hx, &x, sizeof(hx));
    const unsigned exp = (hx >> 23) & 0xffu;
    if (exp == 0xffu) return x - x;
    const std::uint32_t sx = hx & 0x80000000u;
    float z;
    std::memcpy(&z, &sx, sizeof(z));
    return z;
}

} // namespace pbsd::userland::msun
