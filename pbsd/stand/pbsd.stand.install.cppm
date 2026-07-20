module;
#include <cstddef>
#include <cstdint>

export module pbsd.stand.install;

import pbsd.core;

/// PROVENANCE: hbsd/src/stand/install/install.c
export namespace pbsd::stand::install {

enum class Mode : unsigned char {
    Copy = 0,
    Move = 1,
};

[[nodiscard]] inline Status validate_mode(unsigned mode) noexcept {
    return mode <= static_cast<unsigned>(Mode::Move) ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::stand::install
