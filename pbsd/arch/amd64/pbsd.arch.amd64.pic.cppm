module;
#include <cstdint>

export module pbsd.arch.amd64.pic;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/isa/pic/i8259.h — legacy 8259 PIC I/O ports.
export namespace pbsd::arch::amd64::pic {

inline constexpr unsigned kMasterCmd = 0x20;
inline constexpr unsigned kMasterData = 0x21;
inline constexpr unsigned kSlaveCmd = 0xA0;
inline constexpr unsigned kSlaveData = 0xA1;
inline constexpr unsigned kIrqBase = 0x20;
inline constexpr unsigned kSlaveIrq = 2;

enum class Icw1 : unsigned char {
    Icw4 = 0x01,
    Single = 0x02,
    Interval4 = 0x04,
    Level = 0x08,
    Init = 0x10,
};

[[nodiscard]] inline Status validate_irq(unsigned irq) noexcept {
    return irq < 16 ? Status::Ok : Status::Invalid;
}

[[nodiscard]] inline bool is_slave_line(unsigned irq) noexcept {
    return irq >= 8;
}

} // namespace pbsd::arch::amd64::pic
