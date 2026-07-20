export module pbsd.port.wave6.hbsd.src.sys.netpfil.ipfw.dn_sched_prio;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netpfil/ipfw/dn_sched_prio.c
// void dn_sched_prio_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netpfil/ipfw/dn_sched_prio.c wave=wave6 loc=237
export namespace pbsd::port::wave6::hbsd::src::sys::netpfil::ipfw::dn_sched_prio {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netpfil::ipfw::dn_sched_prio
