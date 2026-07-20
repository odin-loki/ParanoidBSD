module;
#include <cstdint>

export module pbsd.arch.amd64.cr;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/x86/include/specialreg.h — CR0 control register bits.
export namespace pbsd::arch::amd64::cr {

enum class Cr0 : unsigned int {
    Pe = 0x00000001,
    Mp = 0x00000002,
    Em = 0x00000004,
    Ts = 0x00000008,
    Ne = 0x00000020,
    Wp = 0x00010000,
    Am = 0x00040000,
    Nw = 0x20000000,
    Cd = 0x40000000,
    Pg = 0x80000000,
};

[[nodiscard]] inline constexpr bool has(unsigned val, Cr0 bit) noexcept {
    return (val & static_cast<unsigned>(bit)) != 0;
}

[[nodiscard]] inline Status validate_paging(unsigned cr0) noexcept {
    if (has(cr0, Cr0::Pg) && !has(cr0, Cr0::Pe)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::arch::amd64::cr
