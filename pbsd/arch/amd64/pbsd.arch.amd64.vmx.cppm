module;
#include <cstdint>

export module pbsd.arch.amd64.vmx;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/amd64/amd64/vmx.c
export namespace pbsd::arch::amd64::vmx {

inline constexpr unsigned kFeatureBit = 5;

[[nodiscard]] inline bool enabled(std::uint32_t ecx) noexcept {
    return (ecx & (1u << kFeatureBit)) != 0;
}

[[nodiscard]] inline Status validate_revision(std::uint32_t rev) noexcept {
    return rev != 0 ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::arch::amd64::vmx
