export module pbsd.port.wave6.hbsd.src.sys.netpfil.ipfw.dn_sched_fifo;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netpfil/ipfw/dn_sched_fifo.c
// void dn_sched_fifo_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netpfil/ipfw/dn_sched_fifo.c wave=wave6 loc=131
export namespace pbsd::port::wave6::hbsd::src::sys::netpfil::ipfw::dn_sched_fifo {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netpfil::ipfw::dn_sched_fifo
