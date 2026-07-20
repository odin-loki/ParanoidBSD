module;
#include <cstdint>

export module pbsd.net.ip6_output;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet6/ip6_output.c — IPv6 output path flags.
export namespace pbsd::net::ip6_output {

enum class Flag : unsigned {
    Unicast   = 0x0001,
    Mcast     = 0x0002,
    Fragment  = 0x0004,
    DontFrag  = 0x0008,
    RawOut    = 0x0010,
};

struct Stats {
    unsigned total{};
    unsigned fragmented{};
    unsigned mcast{};
};

[[nodiscard]] inline Status validate_flags(unsigned flags) noexcept {
    constexpr unsigned kAll = static_cast<unsigned>(Flag::Unicast)
                            | static_cast<unsigned>(Flag::Mcast)
                            | static_cast<unsigned>(Flag::Fragment)
                            | static_cast<unsigned>(Flag::DontFrag)
                            | static_cast<unsigned>(Flag::RawOut);
    if ((flags & ~kAll) != 0) {
        return Status::Invalid;
    }
    if ((flags & static_cast<unsigned>(Flag::Fragment)) != 0
        && (flags & static_cast<unsigned>(Flag::DontFrag)) != 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status account(Stats& st, unsigned flags) noexcept {
    if (validate_flags(flags) != Status::Ok) {
        return Status::Invalid;
    }
    ++st.total;
    if ((flags & static_cast<unsigned>(Flag::Fragment)) != 0) {
        ++st.fragmented;
    }
    if ((flags & static_cast<unsigned>(Flag::Mcast)) != 0) {
        ++st.mcast;
    }
    return Status::Ok;
}

} // namespace pbsd::net::ip6_output
