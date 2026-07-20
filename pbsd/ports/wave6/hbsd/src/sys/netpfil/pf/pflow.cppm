export module pbsd.port.wave6.hbsd.src.sys.netpfil.pf.pflow;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netpfil/pf/pflow.c
// void pflow_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netpfil/pf/pflow.c wave=wave6 loc=1842
export namespace pbsd::port::wave6::hbsd::src::sys::netpfil::pf::pflow {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netpfil::pf::pflow
