export module pbsd.port.wave6.hbsd.src.sys.netpfil.ipfilter.netinet.ip_htable;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netpfil/ipfilter/netinet/ip_htable.c
// void ip_htable_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netpfil/ipfilter/netinet/ip_htable.c wave=wave6 loc=1417
export namespace pbsd::port::wave6::hbsd::src::sys::netpfil::ipfilter::netinet::ip_htable {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netpfil::ipfilter::netinet::ip_htable
