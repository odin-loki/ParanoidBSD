module;
#include <cstdint>

export module pbsd.arch.amd64.amd;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/x86/x86/identcpu.c (AMD vendor path)
export namespace pbsd::arch::amd64::amd {

inline constexpr unsigned kVendorEbx = 0x68747541u; // "Auth"
inline constexpr unsigned kVendorEdx = 0x69746E65u; // "enti"
inline constexpr unsigned kVendorEcx = 0x444D4163u; // "cAMD"

struct VendorLeaf {
    std::uint32_t ebx{kVendorEbx};
    std::uint32_t edx{kVendorEdx};
    std::uint32_t ecx{kVendorEcx};
};

[[nodiscard]] inline bool is_amd_vendor(const VendorLeaf& v) noexcept {
    return v.ebx == kVendorEbx && v.edx == kVendorEdx && v.ecx == kVendorEcx;
}

} // namespace pbsd::arch::amd64::amd
