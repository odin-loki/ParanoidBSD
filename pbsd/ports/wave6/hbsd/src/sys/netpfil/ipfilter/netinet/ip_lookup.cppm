export module pbsd.port.wave6.hbsd.src.sys.netpfil.ipfilter.netinet.ip_lookup;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netpfil/ipfilter/netinet/ip_lookup.c
// void ip_lookup_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netpfil/ipfilter/netinet/ip_lookup.c wave=wave6 loc=933
export namespace pbsd::port::wave6::hbsd::src::sys::netpfil::ipfilter::netinet::ip_lookup {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netpfil::ipfilter::netinet::ip_lookup
