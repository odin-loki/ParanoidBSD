export module pbsd.port.wave6.hbsd.src.sys.net.route.route_subscription;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/net/route/route_subscription.c
// void route_subscription_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/net/route/route_subscription.c wave=wave6 loc=209
export namespace pbsd::port::wave6::hbsd::src::sys::net::route::route_subscription {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::net::route::route_subscription
