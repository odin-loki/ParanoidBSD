module;
#include <cstdint>

export module pbsd.bifrost.cpuid;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/amd64/vmm/vmm_cpuid.c
export namespace pbsd::bifrost::cpuid {

struct Leaf {
    std::uint32_t eax{};
    std::uint32_t ebx{};
    std::uint32_t ecx{};
    std::uint32_t edx{};
};

[[nodiscard]] inline Status validate_leaf(std::uint32_t leaf) noexcept {
    if (leaf > 0x80000008u && leaf < 0x80000000u) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status emulate(Leaf& out, std::uint32_t leaf) noexcept {
    if (validate_leaf(leaf) != Status::Ok) {
        return Status::Invalid;
    }
    out.eax = leaf;
    return Status::Ok;
}

} // namespace pbsd::bifrost::cpuid
