module;
#include <cstdint>

export module pbsd.net.ip_input;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/ip_input.c — IPv4 input path flags.
export namespace pbsd::net::ip_input {

enum class Flag : unsigned {
    Forward  = 0x0001,
    Deliver  = 0x0002,
    Drop     = 0x0004,
    Reassemble = 0x0008,
    Options  = 0x0010,
};

struct Stats {
    unsigned total{};
    unsigned delivered{};
    unsigned forwarded{};
    unsigned dropped{};
};

[[nodiscard]] inline Status validate_flags(unsigned flags) noexcept {
    constexpr unsigned kAll = static_cast<unsigned>(Flag::Forward)
                            | static_cast<unsigned>(Flag::Deliver)
                            | static_cast<unsigned>(Flag::Drop)
                            | static_cast<unsigned>(Flag::Reassemble)
                            | static_cast<unsigned>(Flag::Options);
    if ((flags & ~kAll) != 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status account(Stats& st, unsigned flags) noexcept {
    if (validate_flags(flags) != Status::Ok) {
        return Status::Invalid;
    }
    ++st.total;
    if ((flags & static_cast<unsigned>(Flag::Drop)) != 0) {
        ++st.dropped;
    }
    if ((flags & static_cast<unsigned>(Flag::Deliver)) != 0) {
        ++st.delivered;
    }
    if ((flags & static_cast<unsigned>(Flag::Forward)) != 0) {
        ++st.forwarded;
    }
    return Status::Ok;
}

} // namespace pbsd::net::ip_input
