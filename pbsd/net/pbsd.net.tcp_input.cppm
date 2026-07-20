module;
#include <cstdint>

export module pbsd.net.tcp_input;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/tcp_input.c — TCP input processing flags.
export namespace pbsd::net::tcp_input {

enum class Flag : unsigned {
    AckNow   = 0x0001,
    Drop     = 0x0002,
    Deliver  = 0x0004,
    Sack     = 0x0008,
    FastPath = 0x0010,
};

struct Stats {
    unsigned segments{};
    unsigned drops{};
    unsigned fast_path{};
};

[[nodiscard]] inline Status validate_flags(unsigned flags) noexcept {
    if ((flags & ~(static_cast<unsigned>(Flag::AckNow) | static_cast<unsigned>(Flag::Drop)
                   | static_cast<unsigned>(Flag::Deliver) | static_cast<unsigned>(Flag::Sack)
                   | static_cast<unsigned>(Flag::FastPath))) != 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status account(Stats& st, unsigned flags) noexcept {
    if (validate_flags(flags) != Status::Ok) {
        return Status::Invalid;
    }
    ++st.segments;
    if ((flags & static_cast<unsigned>(Flag::Drop)) != 0) {
        ++st.drops;
        return Status::Protocol;
    }
    if ((flags & static_cast<unsigned>(Flag::FastPath)) != 0) {
        ++st.fast_path;
    }
    return Status::Ok;
}

} // namespace pbsd::net::tcp_input
