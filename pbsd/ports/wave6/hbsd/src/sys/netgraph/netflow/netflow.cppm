export module pbsd.port.wave6.hbsd.src.sys.netgraph.netflow.netflow;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netgraph/netflow/netflow.c
// void netflow_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netgraph/netflow/netflow.c wave=wave6 loc=1179
export namespace pbsd::port::wave6::hbsd::src::sys::netgraph::netflow::netflow {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netgraph::netflow::netflow
