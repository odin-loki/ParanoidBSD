export module pbsd.port.wave2.hbsd.src.usr_bin.netstat.route_netlink;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.bin/netstat/route_netlink.c
// void route_netlink_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.bin/netstat/route_netlink.c wave=wave2 loc=341
export namespace pbsd::port::wave2::hbsd::src::usr_bin::netstat::route_netlink {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_bin::netstat::route_netlink
