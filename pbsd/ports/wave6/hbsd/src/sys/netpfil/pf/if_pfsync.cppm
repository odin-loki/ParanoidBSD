export module pbsd.port.wave6.hbsd.src.sys.netpfil.pf.if_pfsync;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netpfil/pf/if_pfsync.c
// void if_pfsync_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netpfil/pf/if_pfsync.c wave=wave6 loc=3453
export namespace pbsd::port::wave6::hbsd::src::sys::netpfil::pf::if_pfsync {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netpfil::pf::if_pfsync
