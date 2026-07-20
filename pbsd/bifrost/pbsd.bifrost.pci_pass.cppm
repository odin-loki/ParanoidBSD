module;
#include <cstdint>

export module pbsd.bifrost.pci_pass;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/amd64/vmm/io/vmmio.c
export namespace pbsd::bifrost::pci_pass {

enum class PassMode : unsigned char {
    Disabled = 0,
    Passthrough = 1,
    Emulated = 2,
};

[[nodiscard]] inline Status validate_mode(unsigned mode) noexcept {
    return mode <= static_cast<unsigned>(PassMode::Emulated) ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::bifrost::pci_pass
