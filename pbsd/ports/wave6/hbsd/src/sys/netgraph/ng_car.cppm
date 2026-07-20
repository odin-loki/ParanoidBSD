export module pbsd.port.wave6.hbsd.src.sys.netgraph.ng_car;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netgraph/ng_car.c
// void ng_car_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netgraph/ng_car.c wave=wave6 loc=812
export namespace pbsd::port::wave6::hbsd::src::sys::netgraph::ng_car {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netgraph::ng_car
