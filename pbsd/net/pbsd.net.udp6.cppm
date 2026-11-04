module;
#include <cstdint>

export module pbsd.net.udp6;

import pbsd.core;
import pbsd.rights;
import pbsd.net.sockopt;

using pbsd::CapabilityRights;
using pbsd::has_right;

/// PROVENANCE: hbsd/src/sys/netinet/in.h, udp.h — IPv6 UDP socket options.
export namespace pbsd::net::udp6 {

enum class Option : int {
    Checksum = 0x0001,
    Encap    = 0x0002,
    Pktinfo  = 0x0003,
    Recvpktinfo = 0x0004,
};

[[nodiscard]] inline Status check_option(CapabilityRights rights, Option opt,
                                         sockopt::OptDir dir) noexcept {
    (void)opt;
    if (dir == sockopt::OptDir::Set && !has_right(rights, CapabilityRights::Write)) {
        return Status::Denied;
    }
    if (dir == sockopt::OptDir::Get && !has_right(rights, CapabilityRights::Read)) {
        return Status::Denied;
    }
    return Status::Ok;
}

} // namespace pbsd::net::udp6
