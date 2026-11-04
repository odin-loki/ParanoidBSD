module;

#include <cmath>
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.isfinite;

/// isfinite from hbsd/src/lib/msun/src/s_isfinite.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline bool isfinite(double x) noexcept { return std::isfinite(x); }

[[nodiscard]] inline bool isfinitef(float x) noexcept { return std::isfinite(x); }

[[nodiscard]] inline bool isfinite_bits(std::uint64_t bits) noexcept {
    return ((bits >> 52) & 0x7ff) != 0x7ff;
}

} // namespace pbsd::userland::msun
