module;
#include <cstdint>

export module pbsd.net.icmp6_input;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet6/icmp6.c — ICMPv6 input accounting.
export namespace pbsd::net::icmp6_input {

enum class Type : unsigned char {
    EchoRequest = 128,
    EchoReply   = 129,
    NeighborSol = 135,
    NeighborAdv = 136,
    RouterSol   = 133,
    RouterAdv   = 134,
};

struct Stats {
    unsigned total{};
    unsigned echo{};
    unsigned nd{};
    unsigned dropped{};
};

[[nodiscard]] inline Status account(Stats& st, Type t) noexcept {
    ++st.total;
    switch (t) {
    case Type::EchoRequest:
    case Type::EchoReply:
        ++st.echo;
        return Status::Ok;
    case Type::NeighborSol:
    case Type::NeighborAdv:
    case Type::RouterSol:
    case Type::RouterAdv:
        ++st.nd;
        return Status::Ok;
    }
    ++st.dropped;
    return Status::Invalid;
}

} // namespace pbsd::net::icmp6_input
