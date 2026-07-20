module;
#include <cstdint>

export module pbsd.arch.amd64.nmi;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/x86/x86/nmi.c
export namespace pbsd::arch::amd64::nmi {

enum class Source : unsigned char {
    Unknown = 0,
    IoPort = 1,
    Local = 2,
};

struct Event {
    Source source{Source::Unknown};
    std::uint8_t vector{2};
};

[[nodiscard]] inline Status validate_vector(std::uint8_t vector) noexcept {
    return vector >= 32 ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::arch::amd64::nmi
