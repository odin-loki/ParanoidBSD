module;
#include <cstdint>

export module pbsd.arch.amd64.apic;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/x86/include/apicvar.h — APIC delivery modes.
export namespace pbsd::arch::amd64::apic {

enum class DeliveryMode : unsigned char {
    Fixed     = 0,
    Lowest    = 1,
    Smi       = 2,
    Nmi       = 4,
    Init      = 5,
    Startup   = 6,
    ExtInt    = 7,
};

enum class TriggerMode : unsigned char {
    Edge  = 0,
    Level = 1,
};

enum class DestinationMode : unsigned char {
    Physical = 0,
    Logical  = 1,
};

[[nodiscard]] inline Status validate_delivery(DeliveryMode m) noexcept {
    if (static_cast<unsigned char>(m) > static_cast<unsigned char>(DeliveryMode::ExtInt)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::arch::amd64::apic
