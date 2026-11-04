module;
#include <cstdint>

export module pbsd.bifrost.debug;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/amd64/vmm/vmm_debug.c
export namespace pbsd::bifrost::debug {

enum class Level : unsigned char {
    Off = 0,
    Exit = 1,
    Verbose = 2,
};

[[nodiscard]] inline Status validate_level(unsigned lvl) noexcept {
    return lvl <= static_cast<unsigned>(Level::Verbose) ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::bifrost::debug
