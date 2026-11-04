module;
#include <cstdint>

export module pbsd.arch.amd64.lapic;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/amd64 — local APIC MMIO register map scaffold.
export namespace pbsd::arch::amd64::lapic {

inline constexpr std::uint32_t kDefaultBase = 0xFEE00000u;
inline constexpr std::uint32_t kId = 0x20;
inline constexpr std::uint32_t kVersion = 0x30;
inline constexpr std::uint32_t kEoi = 0xB0;
inline constexpr std::uint32_t kSpurious = 0xF0;
inline constexpr std::uint32_t kIcrLow = 0x300;
inline constexpr std::uint32_t kIcrHigh = 0x310;
inline constexpr std::uint32_t kTimer = 0x320;
inline constexpr std::uint32_t kTimerInit = 0x380;
inline constexpr std::uint32_t kTimerCurr = 0x390;

struct SoftRegs {
    std::uint32_t id{};
    std::uint32_t version{0x14};
    std::uint32_t spurious{0x1FF};
    std::uint32_t timer_init{};
    bool enabled{false};
};

[[nodiscard]] inline Status enable(SoftRegs& r) noexcept {
    r.enabled = true;
    r.spurious |= 0x100u;
    return Status::Ok;
}

[[nodiscard]] inline Status eoi(SoftRegs& /*r*/) noexcept {
    return Status::Ok;
}

[[nodiscard]] inline Status arm_timer(SoftRegs& r, std::uint32_t ticks) noexcept {
    if (!r.enabled || ticks == 0) {
        return Status::Invalid;
    }
    r.timer_init = ticks;
    return Status::Ok;
}

} // namespace pbsd::arch::amd64::lapic
