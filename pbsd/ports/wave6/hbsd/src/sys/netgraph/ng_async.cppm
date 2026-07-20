export module pbsd.port.wave6.hbsd.src.sys.netgraph.ng_async;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netgraph/ng_async.c
// void ng_async_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netgraph/ng_async.c wave=wave6 loc=626
export namespace pbsd::port::wave6::hbsd::src::sys::netgraph::ng_async {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netgraph::ng_async
