export module pbsd.port.wave6.hbsd.src.sys.netgraph.ng_nat;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netgraph/ng_nat.c
// void ng_nat_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netgraph/ng_nat.c wave=wave6 loc=993
export namespace pbsd::port::wave6::hbsd::src::sys::netgraph::ng_nat {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netgraph::ng_nat
