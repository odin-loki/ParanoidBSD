export module pbsd.port.wave6.hbsd.src.sys.netpfil.ipfilter.netinet.ip_nat6;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netpfil/ipfilter/netinet/ip_nat6.c
// void ip_nat6_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netpfil/ipfilter/netinet/ip_nat6.c wave=wave6 loc=3996
export namespace pbsd::port::wave6::hbsd::src::sys::netpfil::ipfilter::netinet::ip_nat6 {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netpfil::ipfilter::netinet::ip_nat6
