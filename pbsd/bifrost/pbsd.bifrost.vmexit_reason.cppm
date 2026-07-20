module;
#include <cstdint>

export module pbsd.bifrost.vmexit_reason;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/amd64/vmm/intel/vmx.c
export namespace pbsd::bifrost::vmexit_reason {

enum class Reason : unsigned char {
    ExternalInterrupt = 1,
    Hlt = 12,
    IoInstruction = 30,
    MsrRead = 31,
    MsrWrite = 32,
    EptViolation = 48,
};

[[nodiscard]] inline Status validate_reason(unsigned reason) noexcept {
    return reason <= static_cast<unsigned>(Reason::EptViolation) ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::bifrost::vmexit_reason
