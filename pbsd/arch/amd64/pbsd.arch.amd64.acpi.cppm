module;
#include <cstdint>

export module pbsd.arch.amd64.acpi;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/contrib/dev/acpica
export namespace pbsd::arch::amd64::acpi {

inline constexpr std::uint32_t kRsdpSignature = 0x20445352u; // "RSD "

enum class TableType : unsigned char {
    Unknown = 0,
    Rsdp = 1,
    Rsdt = 2,
    Madt = 3,
};

[[nodiscard]] inline Status validate_signature(std::uint32_t sig) noexcept {
    return sig == kRsdpSignature ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::arch::amd64::acpi
