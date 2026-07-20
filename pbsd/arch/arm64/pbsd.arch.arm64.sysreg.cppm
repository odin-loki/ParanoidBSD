module;
#include <cstdint>

export module pbsd.arch.arm64.sysreg;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/arm64/include/armreg.h, hypervisor.h, arm64/include/vmm.h
export namespace pbsd::arch::arm64::sysreg {

inline constexpr unsigned long long kSctlrRes1 = 0x30d00800;

enum class SctlrEl1Flag : unsigned long long {
    M         = 1ull << 0,
    A         = 1ull << 1,
    C         = 1ull << 2,
    Sa        = 1ull << 3,
    Cp15ben   = 1ull << 5,
    I         = 1ull << 12,
    Wxn       = 1ull << 19,
    Ee        = 1ull << 25,
    Epan      = 1ull << 57,
};

inline constexpr unsigned long long kSctlrEl2Res1 = 0x30c50830;

enum class SctlrEl2Flag : unsigned long long {
    M   = 1ull << 0,
    A   = 1ull << 1,
    C   = 1ull << 2,
    Sa  = 1ull << 3,
    I   = 1ull << 12,
    Wxn = 1ull << 19,
    Ee  = 1ull << 25,
};

enum class IdAa64Mmfr0Parange : unsigned int {
    Range4G   = 0,
    Range64G  = 1,
    Range1T   = 2,
    Range4T   = 3,
    Range16T  = 4,
    Range256T = 5,
    Range4P   = 6,
};

enum class VmExitCode : unsigned int {
    Bogus       = 0,
    InstEmul    = 1,
    RegEmul     = 2,
    Hvc         = 3,
    Suspended   = 4,
    Hyp         = 5,
    Wfi         = 6,
    Paging      = 7,
    Smccc       = 8,
    Debug       = 9,
    Brk         = 10,
    Ss          = 11,
    Wfe         = 12,
    WfiExit     = 13,
};

inline constexpr VmExitCode kGuestExitCodes[] = {
    VmExitCode::InstEmul,
    VmExitCode::RegEmul,
    VmExitCode::Hvc,
    VmExitCode::Wfi,
    VmExitCode::Paging,
    VmExitCode::Smccc,
    VmExitCode::Debug,
    VmExitCode::Brk,
    VmExitCode::Wfe,
    VmExitCode::WfiExit,
};

[[nodiscard]] inline constexpr std::size_t guest_exit_code_count() noexcept {
    return sizeof(kGuestExitCodes) / sizeof(kGuestExitCodes[0]);
}

inline constexpr unsigned kTpidrEl0  = 0x5e82;
inline constexpr unsigned kTpidrroEl0 = 0x5e83;
inline constexpr unsigned kCntvctEl0 = 0x5f01;
inline constexpr unsigned kCntfrqEl0 = 0x5f00;
inline constexpr unsigned kMpidrEl1  = 0xc005;
inline constexpr unsigned kVbarEl1   = 0xc000;
inline constexpr unsigned kTtbr0El1  = 0xc100;
inline constexpr unsigned kTtbr1El1  = 0xc101;
inline constexpr unsigned kTcrEl1    = 0xc102;
inline constexpr unsigned kEsEl1     = 0xc021;
inline constexpr unsigned kCptrEl2   = 0xe08a;

enum class HcrEl2Flag : unsigned long long {
    Vm    = 1ull << 0,
    Tsw   = 1ull << 1,
    Tup   = 1ull << 2,
    Imo   = 1ull << 4,
    Fmo   = 1ull << 3,
    Amo   = 1ull << 5,
    Vse   = 1ull << 8,
    Vi    = 1ull << 9,
    Vf    = 1ull << 10,
    Tsc   = 1ull << 19,
    Tge   = 1ull << 27,
    Tlr   = 1ull << 38,
};

[[nodiscard]] inline constexpr unsigned long long sctlr_el1_reset() noexcept {
    return kSctlrRes1 | static_cast<unsigned long long>(SctlrEl1Flag::Cp15ben);
}

[[nodiscard]] inline constexpr unsigned long long sctlr_el2_host() noexcept {
    return kSctlrEl2Res1
         | static_cast<unsigned long long>(SctlrEl2Flag::M)
         | static_cast<unsigned long long>(SctlrEl2Flag::C)
         | static_cast<unsigned long long>(SctlrEl2Flag::I)
         | static_cast<unsigned long long>(SctlrEl2Flag::A)
         | static_cast<unsigned long long>(SctlrEl2Flag::Sa)
         | static_cast<unsigned long long>(SctlrEl2Flag::Wxn);
}

[[nodiscard]] inline Status validate_hcr_el2(unsigned long long val) noexcept {
    auto flags = static_cast<HcrEl2Flag>(val);
    if ((static_cast<unsigned long long>(flags) & static_cast<unsigned long long>(HcrEl2Flag::Vm)) == 0
        && (static_cast<unsigned long long>(flags) & static_cast<unsigned long long>(HcrEl2Flag::Tge)) != 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_sctlr_el1(unsigned long long val) noexcept {
    if ((val & static_cast<unsigned long long>(SctlrEl1Flag::M)) != 0
        && (val & static_cast<unsigned long long>(SctlrEl1Flag::C)) == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline constexpr unsigned parange_bits(IdAa64Mmfr0Parange p) noexcept {
    switch (p) {
    case IdAa64Mmfr0Parange::Range4G:   return 32;
    case IdAa64Mmfr0Parange::Range64G:  return 36;
    case IdAa64Mmfr0Parange::Range1T:   return 40;
    case IdAa64Mmfr0Parange::Range4T:   return 42;
    case IdAa64Mmfr0Parange::Range16T:  return 44;
    case IdAa64Mmfr0Parange::Range256T: return 48;
    case IdAa64Mmfr0Parange::Range4P:   return 52;
    }
    return 32;
}

} // namespace pbsd::arch::arm64::sysreg
