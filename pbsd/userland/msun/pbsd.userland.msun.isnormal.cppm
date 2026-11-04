module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.isnormal;

/// isnormal from hbsd/src/lib/msun/src/s_isnormal.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline int isnormal_val(double x) noexcept {
    std::uint64_t hx;
    std::memcpy(&hx, &x, sizeof(hx));
    const auto exp = (hx >> 52) & 0x7ffu;
    return exp > 0 && exp < 0x7ff;
}

} // namespace pbsd::userland::msun
