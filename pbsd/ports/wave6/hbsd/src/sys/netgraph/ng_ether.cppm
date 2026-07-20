export module pbsd.port.wave6.hbsd.src.sys.netgraph.ng_ether;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netgraph/ng_ether.c
// void ng_ether_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netgraph/ng_ether.c wave=wave6 loc=880
export namespace pbsd::port::wave6::hbsd::src::sys::netgraph::ng_ether {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netgraph::ng_ether
