module;
#include <cstdint>

export module pbsd.fs.vfs_aio;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/kern/vfs_aio.c — async I/O state machine.
export namespace pbsd::fs::vfs_aio {

enum class State : unsigned char {
    None = 0,
    Queued = 1,
    InProgress = 2,
    Done = 3,
    Cancelled = 4,
};

enum class Flag : unsigned int {
    Read = 0x0001,
    Write = 0x0002,
    Sync = 0x0004,
};

[[nodiscard]] inline Status validate_state(State s) noexcept {
    switch (s) {
    case State::None:
    case State::Queued:
    case State::InProgress:
    case State::Done:
    case State::Cancelled:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

[[nodiscard]] inline bool is_terminal(State s) noexcept {
    return s == State::Done || s == State::Cancelled;
}

} // namespace pbsd::fs::vfs_aio
