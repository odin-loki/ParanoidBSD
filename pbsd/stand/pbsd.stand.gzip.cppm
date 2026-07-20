module;
#include <cstdint>

export module pbsd.stand.gzip;

import pbsd.core;

/// PROVENANCE: hbsd/src/stand — gzip inflate window scaffold for loader.
export namespace pbsd::stand::gzip {

inline constexpr unsigned kWindowBits = 15;
inline constexpr unsigned kWindowSize = 1u << kWindowBits;

enum class State : unsigned char {
    Idle = 0,
    Header = 1,
    Inflate = 2,
    Done = 3,
    Error = 4,
};

struct Stream {
    State state{State::Idle};
    std::uint32_t in_off{};
    std::uint32_t out_off{};
};

[[nodiscard]] inline Status begin(Stream& s) noexcept {
    s = Stream{};
    s.state = State::Header;
    return Status::Ok;
}

[[nodiscard]] inline Status feed(Stream& s, std::uint32_t in_len) noexcept {
    if (s.state == State::Error || s.state == State::Done) {
        return Status::Invalid;
    }
    if (in_len == 0) {
        return Status::Invalid;
    }
    s.in_off += in_len;
    s.state = State::Inflate;
    return Status::Ok;
}

[[nodiscard]] inline Status finish(Stream& s) noexcept {
    if (s.state != State::Inflate && s.state != State::Header) {
        return Status::Invalid;
    }
    s.state = State::Done;
    return Status::Ok;
}

} // namespace pbsd::stand::gzip
