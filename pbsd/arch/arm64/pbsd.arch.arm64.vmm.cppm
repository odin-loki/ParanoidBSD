module;
#include <cstdint>

export module pbsd.arch.arm64.vmm;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/arm64/include/vmm.h
export namespace pbsd::arch::arm64::vmm {

enum class SuspendHow : unsigned char {
    Reset = 0,
    Poweroff = 1,
    Halt = 2,
    TripleFault = 3,
    Msi = 4,
    Stop = 5,
};

enum class RegName : unsigned char {
    Pc = 0,
    Sp = 1,
    Cpsr = 2,
    X0 = 3,
    X1 = 4,
};

inline constexpr std::uint64_t kGuestBaseIpa = 0x8000'0000ull;
inline constexpr unsigned kMaxMmioRegions = 4;
inline constexpr unsigned kMaxSpecialRegs = 16;

enum class CapType : unsigned char {
    None = 0,
    Mmu = 1,
    UnmappedOnly = 2,
};

[[nodiscard]] inline Status validate_reg(RegName reg) noexcept {
    return static_cast<unsigned>(reg) <= static_cast<unsigned>(RegName::X1)
               ? Status::Ok
               : Status::Invalid;
}

} // namespace pbsd::arch::arm64::vmm
