module;
#include <cstdint>

export module pbsd.arch.amd64.svm;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/amd64/amd64/svm.c
export namespace pbsd::arch::amd64::svm {

inline constexpr unsigned kFeatureBit = 2;

[[nodiscard]] inline bool enabled(std::uint32_t ecx) noexcept {
    return (ecx & (1u << kFeatureBit)) != 0;
}

[[nodiscard]] inline Status validate_asid(std::uint32_t asid) noexcept {
    return asid != 0 ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::arch::amd64::svm
