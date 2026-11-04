module;
#include <cstdint>

export module pbsd.arch.arm64.ident;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/arm64/include/armreg.h — MIDR/ID register fields.
export namespace pbsd::arch::arm64::ident {

inline constexpr unsigned kImplementerArm = 0x41;
inline constexpr unsigned kImplementerApple = 0x61;
inline constexpr unsigned kImplementerAmpere = 0xC0;

enum class Implementer : unsigned int {
    Arm = kImplementerArm,
    Apple = kImplementerApple,
    Ampere = kImplementerAmpere,
};

[[nodiscard]] inline unsigned implementer_from_midr(unsigned long long midr) noexcept {
    return static_cast<unsigned>((midr >> 24) & 0xFF);
}

[[nodiscard]] inline unsigned variant_from_midr(unsigned long long midr) noexcept {
    return static_cast<unsigned>((midr >> 20) & 0xF);
}

[[nodiscard]] inline Status validate_midr(unsigned long long midr) noexcept {
    if (implementer_from_midr(midr) == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::arch::arm64::ident
