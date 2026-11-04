module;

#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.isnan;

/// isnan from hbsd/src/lib/msun/src/s_isnan.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline bool isnan(double x) noexcept {
    std::uint64_t u;
    std::memcpy(&u, &x, sizeof(u));
    const auto exp = (u >> 52) & 0x7FFu;
    const auto mant = u & 0x000FFFFFFFFFFFFFULL;
    return exp == 0x7FF && mant != 0;
}

[[nodiscard]] inline bool isnanf(float x) noexcept {
    return isnan(static_cast<double>(x));
}

} // namespace pbsd::userland::msun
