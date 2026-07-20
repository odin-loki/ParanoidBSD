export module pbsd.port.wave6.hbsd.src.sys.netgraph.ng_iface;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netgraph/ng_iface.c
// void ng_iface_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netgraph/ng_iface.c wave=wave6 loc=812
export namespace pbsd::port::wave6::hbsd::src::sys::netgraph::ng_iface {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netgraph::ng_iface
