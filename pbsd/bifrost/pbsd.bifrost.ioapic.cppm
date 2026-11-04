module;
#include <cstdint>

export module pbsd.bifrost.ioapic;

import pbsd.core;
import pbsd.bifrost.hypervisor;

/// PROVENANCE: hbsd/src/sys/x86/x86/ioapic.h — IOAPIC redirection entry stubs.
export namespace pbsd::bifrost::ioapic {

inline constexpr unsigned kMaxIrqs = 24;
inline constexpr unsigned kRegId = 0x00;
inline constexpr unsigned kRegVer = 0x01;
inline constexpr unsigned kRegArb = 0x02;

enum class DeliveryMode : unsigned char {
    Fixed = 0,
    Lowest = 1,
    Smi = 2,
    Nmi = 4,
    Init = 5,
    ExtInt = 7,
};

struct RedirEntry {
    unsigned vector{};
    DeliveryMode mode{DeliveryMode::Fixed};
    bool masked{false};
};

[[nodiscard]] inline Status validate_vector(unsigned vector) noexcept {
    if (vector < 16 || vector > 255) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_index(unsigned idx) noexcept {
    if (idx >= kMaxIrqs) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::bifrost::ioapic
