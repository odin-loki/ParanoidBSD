export module pbsd.port.wave6.hbsd.src.sys.netpfil.pf.pf_if;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netpfil/pf/pf_if.c
// void pf_if_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netpfil/pf/pf_if.c wave=wave6 loc=1190
export namespace pbsd::port::wave6::hbsd::src::sys::netpfil::pf::pf_if {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netpfil::pf::pf_if
