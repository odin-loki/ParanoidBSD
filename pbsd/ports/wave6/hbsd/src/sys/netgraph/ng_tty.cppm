export module pbsd.port.wave6.hbsd.src.sys.netgraph.ng_tty;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netgraph/ng_tty.c
// void ng_tty_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netgraph/ng_tty.c wave=wave6 loc=506
export namespace pbsd::port::wave6::hbsd::src::sys::netgraph::ng_tty {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netgraph::ng_tty
