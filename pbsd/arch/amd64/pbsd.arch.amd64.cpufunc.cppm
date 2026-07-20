module;
#include <cstdint>

export module pbsd.arch.amd64.cpufunc;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/x86/include/cpufunc.h — x86 control-register bits.
export namespace pbsd::arch::amd64::cpufunc {

enum class Cr0Flag : unsigned long long {
    Pe  = 1ull << 0,
    Mp  = 1ull << 1,
    Em  = 1ull << 2,
    Ts  = 1ull << 3,
    Et  = 1ull << 4,
    Ne  = 1ull << 5,
    Wp  = 1ull << 16,
    Am  = 1ull << 18,
    Nw  = 1ull << 29,
    Cd  = 1ull << 30,
    Pg  = 1ull << 31,
};

enum class Cr4Flag : unsigned long long {
    Vme = 1ull << 0,
    Pvi = 1ull << 1,
    Tsd = 1ull << 2,
    De  = 1ull << 3,
    Pse = 1ull << 4,
    Pae = 1ull << 5,
    Mce = 1ull << 6,
    Pge = 1ull << 7,
    Pce = 1ull << 8,
    Osfxsr = 1ull << 9,
    Osxmmexcpt = 1ull << 10,
    Umip = 1ull << 11,
    La57 = 1ull << 12,
    VmxE = 1ull << 13,
    SmxE = 1ull << 14,
    Fsgsbase = 1ull << 16,
    Pcid = 1ull << 17,
    Osxsave = 1ull << 18,
    Smep = 1ull << 20,
    Smap = 1ull << 21,
};

[[nodiscard]] inline Status validate_cr0(unsigned long long val) noexcept {
    if ((val & static_cast<unsigned long long>(Cr0Flag::Pg))
        && !(val & static_cast<unsigned long long>(Cr0Flag::Pe))) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_cr4_vmx(unsigned long long val) noexcept {
    if ((val & static_cast<unsigned long long>(Cr4Flag::VmxE))
        && !(val & static_cast<unsigned long long>(Cr4Flag::Pae))) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::arch::amd64::cpufunc
