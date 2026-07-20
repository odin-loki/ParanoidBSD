export module pbsd.port.wave6.hbsd.src.sys.netgraph.ng_split;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netgraph/ng_split.c
// void ng_split_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netgraph/ng_split.c wave=wave6 loc=176
export namespace pbsd::port::wave6::hbsd::src::sys::netgraph::ng_split {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netgraph::ng_split
