export module pbsd.port.wave6.hbsd.src.sys.netlink.netlink_route;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netlink/netlink_route.c
// void netlink_route_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netlink/netlink_route.c wave=wave6 loc=143
export namespace pbsd::port::wave6::hbsd::src::sys::netlink::netlink_route {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netlink::netlink_route
