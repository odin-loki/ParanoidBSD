module;
#include <cstdint>

export module pbsd.arch.amd64.ioapic;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/x86/x86/ioapic.c
export namespace pbsd::arch::amd64::ioapic {

inline constexpr std::uint32_t kRegSelect = 0x00;
inline constexpr std::uint32_t kRegWindow = 0x10;

[[nodiscard]] inline Status validate_pin(unsigned pin) noexcept {
    return pin < 24 ? Status::Ok : Status::Invalid;
}

[[nodiscard]] inline Status validate_redir(unsigned index) noexcept {
    return index < 48 ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::arch::amd64::ioapic
