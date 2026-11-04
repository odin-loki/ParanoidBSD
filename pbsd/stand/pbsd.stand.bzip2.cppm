module;
#include <cstdint>

export module pbsd.stand.bzip2;

import pbsd.core;

/// PROVENANCE: hbsd/src/stand/libsa/bzip2.c
export namespace pbsd::stand::bzip2 {

inline constexpr char kMagic0 = 'B';
inline constexpr char kMagic1 = 'Z';

enum class State : unsigned char {
    Idle = 0,
    Header = 1,
    Block = 2,
    Done = 3,
    Error = 4,
};

struct Stream {
    State state{State::Idle};
    std::uint32_t block_size{};
};

[[nodiscard]] inline Status begin(Stream& s, char magic0, char magic1) noexcept {
    if (magic0 != kMagic0 || magic1 != kMagic1) {
        s.state = State::Error;
        return Status::Invalid;
    }
    s = Stream{};
    s.state = State::Header;
    return Status::Ok;
}

[[nodiscard]] inline Status set_block_size(Stream& s, std::uint32_t size) noexcept {
    if (s.state != State::Header || size == 0) {
        return Status::Invalid;
    }
    s.block_size = size;
    s.state = State::Block;
    return Status::Ok;
}

} // namespace pbsd::stand::bzip2
