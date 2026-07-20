export module pbsd.port.wave6.hbsd.src.sys.netgraph.ng_hole;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netgraph/ng_hole.c
// void ng_hole_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netgraph/ng_hole.c wave=wave6 loc=225
export namespace pbsd::port::wave6::hbsd::src::sys::netgraph::ng_hole {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netgraph::ng_hole
