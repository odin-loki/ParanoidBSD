module;
#include <cstdint>

export module pbsd.net.raw_ip;

import pbsd.core;
import pbsd.rights;
import pbsd.net.sockopt;

using pbsd::CapabilityRights;
using pbsd::has_right;

/// PROVENANCE: hbsd/src/sys/netinet/raw_ip.c — raw IP socket options.
export namespace pbsd::net::raw_ip {

enum class Option : int {
    HeaderIncl = 0x0002,
    Ttl        = 0x0004,
    Tos        = 0x0005,
    Recvif     = 0x0006,
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

} // namespace pbsd::net::raw_ip
