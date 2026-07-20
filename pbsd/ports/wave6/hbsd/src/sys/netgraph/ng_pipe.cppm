export module pbsd.port.wave6.hbsd.src.sys.netgraph.ng_pipe;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netgraph/ng_pipe.c
// void ng_pipe_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netgraph/ng_pipe.c wave=wave6 loc=1013
export namespace pbsd::port::wave6::hbsd::src::sys::netgraph::ng_pipe {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netgraph::ng_pipe
