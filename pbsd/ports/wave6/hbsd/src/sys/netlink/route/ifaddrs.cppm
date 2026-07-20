export module pbsd.port.wave6.hbsd.src.sys.netlink.route.ifaddrs;

module;
// Header bridge — replace #include of hbsd/src/sys/netlink/route/ifaddrs.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netlink/route/ifaddrs.h wave=wave6 loc=99
export namespace pbsd::port::wave6::hbsd::src::sys::netlink::route::ifaddrs {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netlink::route::ifaddrs
