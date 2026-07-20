module;

#include <cstdint>
#include <cstring>
#include <limits>

export module pbsd.userland.msun.ilogb;

/// ilogb/ilogbf from hbsd/src/lib/msun/src/s_ilogb.c
export namespace pbsd::userland::msun {

inline constexpr int kFpIlogb0 = std::numeric_limits<int>::min();
inline constexpr int kFpIlogbNan = std::numeric_limits<int>::min();

[[nodiscard]] inline int ilogb(double x) noexcept {
    std::uint64_t u;
    std::memcpy(&u, &x, sizeof(u));
    const auto exp = static_cast<int>((u >> 52) & 0x7FFu);
    const auto mant = u & 0x000FFFFFFFFFFFFFULL;
    if (exp == 0) {
        return mant == 0 ? kFpIlogb0 : -1023;
    }
    if (exp == 0x7FF) {
        return mant != 0 ? kFpIlogbNan : std::numeric_limits<int>::max();
    }
    return exp - 1023;
}

[[nodiscard]] inline int ilogbf(float x) noexcept {
    return ilogb(static_cast<double>(x));
}

} // namespace pbsd::userland::msun
