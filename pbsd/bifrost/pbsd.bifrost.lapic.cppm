module;
#include <cstdint>

export module pbsd.bifrost.lapic;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/x86/x86/lapic.h — LAPIC timer and ICR stubs.
export namespace pbsd::bifrost::lapic {

inline constexpr unsigned kRegId = 0x020;
inline constexpr unsigned kRegVersion = 0x030;
inline constexpr unsigned kRegTpr = 0x080;
inline constexpr unsigned kRegEoi = 0x0B0;
inline constexpr unsigned kRegSvr = 0x0F0;
inline constexpr unsigned kRegIcrLow = 0x300;
inline constexpr unsigned kRegIcrHigh = 0x310;

enum class Mode : unsigned char {
    OneShot = 0,
    Periodic = 1,
    TscDeadline = 2,
};

[[nodiscard]] inline Status validate_mode(Mode m) noexcept {
    switch (m) {
    case Mode::OneShot:
    case Mode::Periodic:
    case Mode::TscDeadline:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

[[nodiscard]] inline Status validate_vector(unsigned vector) noexcept {
    if (vector < 16 || vector > 255) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::bifrost::lapic
