export module pbsd.port.wave6.hbsd.src.sys.netgraph.ng_macfilter;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netgraph/ng_macfilter.c
// void ng_macfilter_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netgraph/ng_macfilter.c wave=wave6 loc=876
export namespace pbsd::port::wave6::hbsd::src::sys::netgraph::ng_macfilter {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netgraph::ng_macfilter
