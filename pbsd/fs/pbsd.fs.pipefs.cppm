module;
#include <cstdint>

export module pbsd.fs.pipefs;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/fs/pipefs/pipe.h — pipe buffer and state flags.
export namespace pbsd::fs::pipefs {

inline constexpr unsigned kPipeBuf = 16384;

enum class State : unsigned char {
    Open   = 0,
    Closed = 1,
    Broken = 2,
};

enum class Flag : unsigned int {
    Async  = 0x00000001,
    Direct = 0x00000002,
    NonBlock = 0x00000004,
};

[[nodiscard]] inline Status validate_state(State s) noexcept {
    switch (s) {
    case State::Open:
    case State::Closed:
    case State::Broken:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

[[nodiscard]] inline Status validate_io_len(unsigned len) noexcept {
    if (len > kPipeBuf) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::fs::pipefs
