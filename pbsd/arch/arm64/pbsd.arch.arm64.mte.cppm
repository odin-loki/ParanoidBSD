module;
#include <cstdint>

export module pbsd.arch.arm64.mte;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/arm64/arm64/mte.c
export namespace pbsd::arch::arm64::mte {

enum class TagMode : unsigned char {
    Disabled = 0,
    Sync = 1,
    Async = 2,
};

[[nodiscard]] inline Status validate_mode(unsigned mode) noexcept {
    return mode <= static_cast<unsigned>(TagMode::Async) ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::arch::arm64::mte
