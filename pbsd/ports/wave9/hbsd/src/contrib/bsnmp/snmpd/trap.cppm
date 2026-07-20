export module pbsd.port.wave9.hbsd.src.contrib.bsnmp.snmpd.trap;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/bsnmp/snmpd/trap.c
// void trap_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/bsnmp/snmpd/trap.c wave=wave9 loc=910
export namespace pbsd::port::wave9::hbsd::src::contrib::bsnmp::snmpd::trap {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::bsnmp::snmpd::trap
