export module pbsd.port.wave6.hbsd.src.sys.netpfil.ipfilter.netinet.ip_sync;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netpfil/ipfilter/netinet/ip_sync.c
// void ip_sync_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netpfil/ipfilter/netinet/ip_sync.c wave=wave6 loc=1432
export namespace pbsd::port::wave6::hbsd::src::sys::netpfil::ipfilter::netinet::ip_sync {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netpfil::ipfilter::netinet::ip_sync
