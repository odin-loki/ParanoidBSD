module;
#include <cstdint>

export module pbsd.stand.acpi;

import pbsd.core;

/// PROVENANCE: hbsd/src/stand/efi/acpi.c
export namespace pbsd::stand::acpi {

inline constexpr unsigned kRsdpSignature = 0x20445352u; // "RSD "
inline constexpr unsigned kXsdtSignature = 0x54445358u; // "XSDT"

struct TableHeader {
    std::uint32_t signature{};
    std::uint32_t length{};
    std::uint8_t revision{};
};

[[nodiscard]] inline Status validate_header(const TableHeader& h) noexcept {
    if (h.length < sizeof(TableHeader)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::stand::acpi
