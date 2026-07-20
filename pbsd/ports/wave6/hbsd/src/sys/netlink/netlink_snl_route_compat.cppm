export module pbsd.port.wave6.hbsd.src.sys.netlink.netlink_snl_route_compat;

module;
// Header bridge — replace #include of hbsd/src/sys/netlink/netlink_snl_route_compat.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netlink/netlink_snl_route_compat.h wave=wave6 loc=53
export namespace pbsd::port::wave6::hbsd::src::sys::netlink::netlink_snl_route_compat {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netlink::netlink_snl_route_compat
