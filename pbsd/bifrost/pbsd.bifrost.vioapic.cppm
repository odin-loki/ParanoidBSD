module;
#include <cstdint>

export module pbsd.bifrost.vioapic;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/amd64/vmm/io/vioapic.h — virtual IOAPIC MMIO window.
export namespace pbsd::bifrost::vioapic {

inline constexpr std::uint64_t kBase = 0xFEC0'0000ull;
inline constexpr unsigned kSize = 4096;
inline constexpr unsigned kMaxIrq = 24;

[[nodiscard]] inline Status validate_gpa(std::uint64_t gpa) noexcept {
    if (gpa < kBase || gpa >= kBase + kSize) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_irq(int irq) noexcept {
    if (irq < 0 || static_cast<unsigned>(irq) >= kMaxIrq) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::bifrost::vioapic
