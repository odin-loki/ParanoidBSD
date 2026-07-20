export module pbsd.port.wave6.hbsd.src.sys.netgraph.ng_tcpmss;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netgraph/ng_tcpmss.c
// void ng_tcpmss_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netgraph/ng_tcpmss.c wave=wave6 loc=451
export namespace pbsd::port::wave6::hbsd::src::sys::netgraph::ng_tcpmss {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netgraph::ng_tcpmss
