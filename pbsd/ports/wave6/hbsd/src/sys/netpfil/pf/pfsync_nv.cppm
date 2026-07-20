export module pbsd.port.wave6.hbsd.src.sys.netpfil.pf.pfsync_nv;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netpfil/pf/pfsync_nv.c
// void pfsync_nv_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netpfil/pf/pfsync_nv.c wave=wave6 loc=161
export namespace pbsd::port::wave6::hbsd::src::sys::netpfil::pf::pfsync_nv {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netpfil::pf::pfsync_nv
