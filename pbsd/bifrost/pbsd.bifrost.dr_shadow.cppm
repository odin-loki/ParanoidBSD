module;
#include <cstdint>

export module pbsd.bifrost.dr_shadow;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/amd64/vmm/vmm_dr.c
export namespace pbsd::bifrost::dr_shadow {

struct Shadow {
    std::uint64_t dr6{};
    std::uint64_t dr7{};
};

[[nodiscard]] inline Status update(Shadow& s, std::uint64_t dr6, std::uint64_t dr7) noexcept {
    s.dr6 = dr6;
    s.dr7 = dr7;
    return Status::Ok;
}

} // namespace pbsd::bifrost::dr_shadow
