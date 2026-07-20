export module pbsd.port.wave6.hbsd.src.sys.netpfil.ipfilter.netinet.mlfk_ipl;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netpfil/ipfilter/netinet/mlfk_ipl.c
// void mlfk_ipl_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netpfil/ipfilter/netinet/mlfk_ipl.c wave=wave6 loc=653
export namespace pbsd::port::wave6::hbsd::src::sys::netpfil::ipfilter::netinet::mlfk_ipl {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netpfil::ipfilter::netinet::mlfk_ipl
