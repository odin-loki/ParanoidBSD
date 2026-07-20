export module pbsd.port.wave6.hbsd.src.sys.netpfil.ipfilter.netinet.ip_proxy;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netpfil/ipfilter/netinet/ip_proxy.c
// void ip_proxy_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netpfil/ipfilter/netinet/ip_proxy.c wave=wave6 loc=1417
export namespace pbsd::port::wave6::hbsd::src::sys::netpfil::ipfilter::netinet::ip_proxy {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netpfil::ipfilter::netinet::ip_proxy
