module;
#include <cstdint>

export module pbsd.net.udp_input;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/udp_usrreq.c — UDP input path flags.
export namespace pbsd::net::udp_input {

enum class Flag : unsigned {
    Deliver = 0x0001,
    Drop    = 0x0002,
    ChecksumFail = 0x0004,
    NoPcb   = 0x0008,
};

struct Stats {
    unsigned total{};
    unsigned delivered{};
    unsigned dropped{};
    unsigned bad_csum{};
};

[[nodiscard]] inline Status validate_flags(unsigned flags) noexcept {
    constexpr unsigned kAll = static_cast<unsigned>(Flag::Deliver)
                            | static_cast<unsigned>(Flag::Drop)
                            | static_cast<unsigned>(Flag::ChecksumFail)
                            | static_cast<unsigned>(Flag::NoPcb);
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
    if ((flags & static_cast<unsigned>(Flag::Deliver)) != 0) {
        ++st.delivered;
    }
    if ((flags & static_cast<unsigned>(Flag::Drop)) != 0) {
        ++st.dropped;
    }
    if ((flags & static_cast<unsigned>(Flag::ChecksumFail)) != 0) {
        ++st.bad_csum;
    }
    return Status::Ok;
}

} // namespace pbsd::net::udp_input
