export module pbsd.port.wave6.hbsd.src.sys.netpfil.ipfilter.netinet.ip_nat;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netpfil/ipfilter/netinet/ip_nat.c
// void ip_nat_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netpfil/ipfilter/netinet/ip_nat.c wave=wave6 loc=8416
export namespace pbsd::port::wave6::hbsd::src::sys::netpfil::ipfilter::netinet::ip_nat {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netpfil::ipfilter::netinet::ip_nat
