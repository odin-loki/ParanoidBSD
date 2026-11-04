module;

#include <cstdint>

export module pbsd.userland.libc.gdtoa.ldis;

/// machdep_ldisd helpers from hbsd/src/lib/libc/gdtoa/machdep_ldisd.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline int extract_exponent_bits(std::uint64_t bits) noexcept {
    return static_cast<int>((bits >> 52) & 0x7ff) - 1023;
}

[[nodiscard]] inline bool is_nan_bits(std::uint64_t bits) noexcept {
    return ((bits >> 52) & 0x7ff) == 0x7ff && (bits & 0xfffffffffffffULL) != 0;
}

} // namespace pbsd::userland::libc
