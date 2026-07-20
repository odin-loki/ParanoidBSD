export module pbsd.port.wave6.hbsd.src.sys.netgraph.ng_vlan;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netgraph/ng_vlan.c
// void ng_vlan_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netgraph/ng_vlan.c wave=wave6 loc=707
export namespace pbsd::port::wave6::hbsd::src::sys::netgraph::ng_vlan {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netgraph::ng_vlan
