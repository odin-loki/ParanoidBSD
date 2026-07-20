export module pbsd.port.wave6.hbsd.src.sys.netgraph.netflow.ng_netflow;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netgraph/netflow/ng_netflow.c
// void ng_netflow_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netgraph/netflow/ng_netflow.c wave=wave6 loc=1064
export namespace pbsd::port::wave6::hbsd::src::sys::netgraph::netflow::ng_netflow {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netgraph::netflow::ng_netflow
