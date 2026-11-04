module;
#include <cstdint>

export module pbsd.net.tcp6;

import pbsd.core;
import pbsd.rights;
import pbsd.net.sockopt;

using pbsd::CapabilityRights;
using pbsd::has_right;

/// PROVENANCE: hbsd/src/sys/netinet/tcp.h, tcp_fsm.h — IPv6 TCP socket options.
export namespace pbsd::net::tcp6 {

enum class Option : int {
    NoDelay   = 0x0001,
    MaxSeg    = 0x0002,
    KeepIdle  = 0x0003,
    KeepIntvl = 0x0004,
    KeepCnt   = 0x0005,
    Pktinfo   = 0x0006,
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

} // namespace pbsd::net::tcp6
