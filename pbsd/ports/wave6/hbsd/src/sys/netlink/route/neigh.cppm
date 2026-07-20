export module pbsd.port.wave6.hbsd.src.sys.netlink.route.neigh;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netlink/route/neigh.c
// void neigh_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netlink/route/neigh.c wave=wave6 loc=601
export namespace pbsd::port::wave6::hbsd::src::sys::netlink::route::neigh {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netlink::route::neigh
