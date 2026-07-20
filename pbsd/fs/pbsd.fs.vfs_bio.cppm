module;
#include <cstdint>

export module pbsd.fs.vfs_bio;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/kern/vfs_bio.c — buffer cache states.
export namespace pbsd::fs::vfs_bio {

enum class State : unsigned char {
    Empty = 0,
    Busy  = 1,
    Done  = 2,
    Invalid = 3,
};

struct Buf {
    State state{State::Empty};
    unsigned bsize{};
    unsigned bcount{};
};

[[nodiscard]] inline Status validate_state(State s) noexcept {
    switch (s) {
    case State::Empty:
    case State::Busy:
    case State::Done:
    case State::Invalid:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

[[nodiscard]] inline Status start_io(Buf& b, unsigned count) noexcept {
    if (b.state != State::Empty || count == 0) {
        return Status::Invalid;
    }
    b.state = State::Busy;
    b.bcount = count;
    return Status::Ok;
}

[[nodiscard]] inline Status complete_io(Buf& b) noexcept {
    if (b.state != State::Busy) {
        return Status::Invalid;
    }
    b.state = State::Done;
    return Status::Ok;
}

} // namespace pbsd::fs::vfs_bio
