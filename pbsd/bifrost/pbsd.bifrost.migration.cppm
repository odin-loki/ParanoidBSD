module;
#include <cstdint>

export module pbsd.bifrost.migration;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/amd64/vmm/vmm_snapshot.c
export namespace pbsd::bifrost::migration {

enum class Phase : unsigned char {
    Idle = 0,
    PreCopy = 1,
    StopCopy = 2,
    Resume = 3,
};

[[nodiscard]] inline Status validate_phase(unsigned phase) noexcept {
    return phase <= static_cast<unsigned>(Phase::Resume) ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::bifrost::migration
