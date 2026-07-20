export module pbsd.port.wave6.hbsd.src.sys.netpfil.ipfilter.netinet.radix_ipf;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netpfil/ipfilter/netinet/radix_ipf.c
// void radix_ipf_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netpfil/ipfilter/netinet/radix_ipf.c wave=wave6 loc=1485
export namespace pbsd::port::wave6::hbsd::src::sys::netpfil::ipfilter::netinet::radix_ipf {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netpfil::ipfilter::netinet::radix_ipf
