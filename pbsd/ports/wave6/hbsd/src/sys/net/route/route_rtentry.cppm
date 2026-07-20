export module pbsd.port.wave6.hbsd.src.sys.net.route.route_rtentry;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/net/route/route_rtentry.c
// void route_rtentry_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/net/route/route_rtentry.c wave=wave6 loc=331
export namespace pbsd::port::wave6::hbsd::src::sys::net::route::route_rtentry {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::net::route::route_rtentry
