module;
#include <cstdint>

export module pbsd.stand.secureboot;

import pbsd.core;

/// PROVENANCE: hbsd/src/stand/efi/secureboot.c
export namespace pbsd::stand::secureboot {

enum class Mode : unsigned char {
    Disabled = 0,
    Setup = 1,
    Deployed = 2,
};

[[nodiscard]] inline Status validate_mode(unsigned mode) noexcept {
    return mode <= static_cast<unsigned>(Mode::Deployed) ? Status::Ok : Status::Invalid;
}

[[nodiscard]] inline bool requires_signature(Mode m) noexcept {
    return m == Mode::Deployed;
}

} // namespace pbsd::stand::secureboot
