export module pbsd.port.wave6.hbsd.src.sys.netpfil.pf.pf_nv;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netpfil/pf/pf_nv.c
// void pf_nv_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netpfil/pf/pf_nv.c wave=wave6 loc=1230
export namespace pbsd::port::wave6::hbsd::src::sys::netpfil::pf::pf_nv {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netpfil::pf::pf_nv
