module;
#include <cstdint>

export module pbsd.bifrost.cr_shadow;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/amd64/vmm/vmm_cr.c
export namespace pbsd::bifrost::cr_shadow {

struct Shadow {
    std::uint64_t cr0{};
    std::uint64_t cr3{};
    std::uint64_t cr4{};
};

[[nodiscard]] inline Status sync(Shadow& s, std::uint64_t cr0, std::uint64_t cr4) noexcept {
    s.cr0 = cr0;
    s.cr4 = cr4;
    return Status::Ok;
}

[[nodiscard]] inline Status set_cr3(Shadow& s, std::uint64_t cr3) noexcept {
    if (cr3 == 0) {
        return Status::Invalid;
    }
    s.cr3 = cr3;
    return Status::Ok;
}

} // namespace pbsd::bifrost::cr_shadow
