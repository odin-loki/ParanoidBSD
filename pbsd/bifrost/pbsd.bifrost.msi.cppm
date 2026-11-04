module;
#include <cstdint>

export module pbsd.bifrost.msi;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/amd64/vmm/vmm_msi.c
export namespace pbsd::bifrost::msi {

struct Message {
    std::uint32_t address{};
    std::uint32_t data{};
};

[[nodiscard]] inline Status validate_message(const Message& m) noexcept {
    if (m.address == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status inject(Message const& m) noexcept {
    return validate_message(m);
}

} // namespace pbsd::bifrost::msi
