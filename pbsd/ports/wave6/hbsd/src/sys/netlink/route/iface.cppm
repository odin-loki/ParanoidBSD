export module pbsd.port.wave6.hbsd.src.sys.netlink.route.iface;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netlink/route/iface.c
// void iface_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netlink/route/iface.c wave=wave6 loc=1528
export namespace pbsd::port::wave6::hbsd::src::sys::netlink::route::iface {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netlink::route::iface
