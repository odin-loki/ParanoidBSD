module;
#include <cstdint>

export module pbsd.arch.amd64.cpuid;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/amd64 — CPUID leaf decode scaffold.
export namespace pbsd::arch::amd64::cpuid {

struct Leaf {
    std::uint32_t eax{};
    std::uint32_t ebx{};
    std::uint32_t ecx{};
    std::uint32_t edx{};
};

[[nodiscard]] inline bool has_bit(std::uint32_t reg, unsigned bit) noexcept {
    return bit < 32 && ((reg >> bit) & 1u) != 0;
}

[[nodiscard]] inline Status validate_leaf(std::uint32_t leaf) noexcept {
    // Soft upper bound for known architectural leaves in scaffold.
    if (leaf > 0x80000008u && leaf < 0x80000000u) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline bool sse2(const Leaf& l1) noexcept {
    return has_bit(l1.edx, 26);
}

[[nodiscard]] inline bool avx(const Leaf& l1) noexcept {
    return has_bit(l1.ecx, 28);
}

} // namespace pbsd::arch::amd64::cpuid
