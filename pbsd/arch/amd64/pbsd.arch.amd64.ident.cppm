module;
#include <cstdint>

export module pbsd.arch.amd64.ident;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/x86/include/specialreg.h, cpuid.h — CPUID leaf/feature bits.
export namespace pbsd::arch::amd64::ident {

inline constexpr unsigned kLeafBasic = 0;
inline constexpr unsigned kLeafFeatures = 1;
inline constexpr unsigned kLeafExtended = 0x80000000u;
inline constexpr unsigned kLeafBrand = 0x80000002u;

enum class FeatureEcx : unsigned int {
    Sse3 = 1u << 0,
    Pclmul = 1u << 1,
    Ssse3 = 1u << 9,
    Fma = 1u << 12,
    Avx = 1u << 28,
    Rdrnd = 1u << 30,
};

enum class FeatureEdx : unsigned int {
    Msr = 1u << 5,
    Apic = 1u << 9,
    Cx8 = 1u << 8,
    Mmx = 1u << 23,
    Sse = 1u << 25,
    Sse2 = 1u << 26,
};

[[nodiscard]] inline Status validate_leaf(unsigned leaf) noexcept {
    if (leaf > 0xFFFF'FFFFu) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline bool has_ecx(unsigned ecx, FeatureEcx f) noexcept {
    return (ecx & static_cast<unsigned>(f)) != 0;
}

} // namespace pbsd::arch::amd64::ident
