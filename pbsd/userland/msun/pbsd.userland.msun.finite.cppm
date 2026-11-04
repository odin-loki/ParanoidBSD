module;

#include <cmath>
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.finite;

/// finite from hbsd/src/lib/msun/src/s_finite.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline int finite(double x) noexcept { return std::isfinite(x) ? 1 : 0; }

[[nodiscard]] inline int finitef(float x) noexcept { return std::isfinite(x) ? 1 : 0; }

[[nodiscard]] inline bool finite_bits(std::uint64_t bits) noexcept {
    return ((bits >> 52) & 0x7ff) != 0x7ff;
}

} // namespace pbsd::userland::msun
