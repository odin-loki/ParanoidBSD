export module pbsd.port.wave6.hbsd.src.sys.net.route;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/net/route.c
// void route_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/net/route.c wave=wave6 loc=706
export namespace pbsd::port::wave6::hbsd::src::sys::net::route {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::net::route
