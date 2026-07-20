export module pbsd.port.wave6.hbsd.src.sys.netpfil.pf.pf_nl;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netpfil/pf/pf_nl.c
// void pf_nl_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netpfil/pf/pf_nl.c wave=wave6 loc=2524
export namespace pbsd::port::wave6::hbsd::src::sys::netpfil::pf::pf_nl {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netpfil::pf::pf_nl
