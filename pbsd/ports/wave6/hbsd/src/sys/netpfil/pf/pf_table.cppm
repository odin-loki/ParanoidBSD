export module pbsd.port.wave6.hbsd.src.sys.netpfil.pf.pf_table;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netpfil/pf/pf_table.c
// void pf_table_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netpfil/pf/pf_table.c wave=wave6 loc=2477
export namespace pbsd::port::wave6::hbsd::src::sys::netpfil::pf::pf_table {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netpfil::pf::pf_table
