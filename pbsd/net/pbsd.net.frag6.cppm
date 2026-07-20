module;
#include <cstdint>

export module pbsd.net.frag6;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet6/frag6.c — IPv6 fragmentation/reassembly flags.
export namespace pbsd::net::frag6 {

enum class Flag : unsigned {
    MoreFrags = 0x0001,
    Reassemble = 0x0002,
    Drop = 0x0004,
    Forward = 0x0008,
};

struct Stats {
    unsigned frags{};
    unsigned reassembled{};
    unsigned dropped{};
};

[[nodiscard]] inline Status validate_flags(unsigned flags) noexcept {
    constexpr unsigned kAll = static_cast<unsigned>(Flag::MoreFrags)
                            | static_cast<unsigned>(Flag::Reassemble)
                            | static_cast<unsigned>(Flag::Drop)
                            | static_cast<unsigned>(Flag::Forward);
    if ((flags & ~kAll) != 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status account(Stats& st, unsigned flags) noexcept {
    if (validate_flags(flags) != Status::Ok) {
        return Status::Invalid;
    }
    ++st.frags;
    if ((flags & static_cast<unsigned>(Flag::Drop)) != 0) {
        ++st.dropped;
    }
    if ((flags & static_cast<unsigned>(Flag::Reassemble)) != 0) {
        ++st.reassembled;
    }
    return Status::Ok;
}

} // namespace pbsd::net::frag6
