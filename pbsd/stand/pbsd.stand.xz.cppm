module;
#include <cstdint>

export module pbsd.stand.xz;

import pbsd.core;

/// PROVENANCE: hbsd/src/stand/libsa/xz.c
export namespace pbsd::stand::xz {

inline constexpr std::uint8_t kHeaderMagic[6]{0xFD, '7', 'z', 'X', 'Z', 0x00};

enum class State : unsigned char {
    Idle = 0,
    Header = 1,
    Block = 2,
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
    if (s.state == State::Error || s.state == State::Done || in_len == 0) {
        return Status::Invalid;
    }
    s.in_off += in_len;
    s.state = State::Block;
    return Status::Ok;
}

[[nodiscard]] inline Status finish(Stream& s) noexcept {
    if (s.state != State::Block && s.state != State::Header) {
        return Status::Invalid;
    }
    s.state = State::Done;
    return Status::Ok;
}

} // namespace pbsd::stand::xz
