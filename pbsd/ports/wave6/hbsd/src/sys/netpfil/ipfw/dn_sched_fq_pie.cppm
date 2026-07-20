export module pbsd.port.wave6.hbsd.src.sys.netpfil.ipfw.dn_sched_fq_pie;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netpfil/ipfw/dn_sched_fq_pie.c
// void dn_sched_fq_pie_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netpfil/ipfw/dn_sched_fq_pie.c wave=wave6 loc=1237
export namespace pbsd::port::wave6::hbsd::src::sys::netpfil::ipfw::dn_sched_fq_pie {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netpfil::ipfw::dn_sched_fq_pie
