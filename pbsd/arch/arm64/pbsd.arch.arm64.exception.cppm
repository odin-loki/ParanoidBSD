module;
#include <cstdint>

export module pbsd.arch.arm64.exception;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/arm64/include/exception.h — ESR_ELx exception classes.
export namespace pbsd::arch::arm64::exception {

enum class Ec : unsigned char {
    Unknown         = 0x00,
    Wfi             = 0x01,
    MrcMcrCp15      = 0x03,
    MrcMcrCp14      = 0x04,
    LdcStc          = 0x05,
    SimdFp          = 0x07,
    MrcMcrCp10      = 0x08,
    BranchTarget    = 0x0A,
    IllegalState    = 0x0E,
    Svc32           = 0x11,
    Svc64           = 0x15,
    InstAbtLower    = 0x20,
    InstAbtCurrent  = 0x21,
    DataAbtLower    = 0x24,
    DataAbtCurrent  = 0x25,
    IeAbtLower      = 0x28,
    IeAbtCurrent    = 0x29,
    SError          = 0x2F,
};

[[nodiscard]] inline unsigned ec_from_esr(unsigned long long esr) noexcept {
    return static_cast<unsigned>((esr >> 26) & 0x3F);
}

[[nodiscard]] inline Status validate_ec(Ec ec) noexcept {
    if (static_cast<unsigned char>(ec) > static_cast<unsigned char>(Ec::SError)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline bool is_sync_abort(Ec ec) noexcept {
    return ec == Ec::InstAbtLower || ec == Ec::InstAbtCurrent
        || ec == Ec::DataAbtLower || ec == Ec::DataAbtCurrent;
}

} // namespace pbsd::arch::arm64::exception
