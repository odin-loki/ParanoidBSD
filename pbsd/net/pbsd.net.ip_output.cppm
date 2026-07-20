module;
#include <cstdint>

export module pbsd.net.ip_output;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/ip_output.c — IPv4 output path flags.
export namespace pbsd::net::ip_output {

enum class Flag : unsigned {
    Routing  = 0x0001,
    Fragment = 0x0002,
    Options  = 0x0004,
    Drop     = 0x0008,
};

struct Stats {
    unsigned total{};
    unsigned fragments{};
    unsigned dropped{};
};

[[nodiscard]] inline Status validate_flags(unsigned flags) noexcept {
    constexpr unsigned kAll = static_cast<unsigned>(Flag::Routing)
                            | static_cast<unsigned>(Flag::Fragment)
                            | static_cast<unsigned>(Flag::Options)
                            | static_cast<unsigned>(Flag::Drop);
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
    if ((flags & static_cast<unsigned>(Flag::Fragment)) != 0) {
        ++st.fragments;
    }
    if ((flags & static_cast<unsigned>(Flag::Drop)) != 0) {
        ++st.dropped;
    }
    return Status::Ok;
}

} // namespace pbsd::net::ip_output
