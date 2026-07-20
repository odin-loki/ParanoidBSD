module;
#include <cstdint>

export module pbsd.bifrost.msix;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/amd64/vmm/vmm_msix.c
export namespace pbsd::bifrost::msix {

inline constexpr unsigned kMaxVectors = 2048;

struct TableEntry {
    std::uint64_t msg_addr{};
    std::uint32_t msg_data{};
    std::uint32_t vector_control{};
};

[[nodiscard]] inline Status validate_vector(unsigned vec) noexcept {
    return vec < kMaxVectors ? Status::Ok : Status::Invalid;
}

[[nodiscard]] inline Status validate_entry(const TableEntry& e) noexcept {
    if (e.msg_addr == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::bifrost::msix
