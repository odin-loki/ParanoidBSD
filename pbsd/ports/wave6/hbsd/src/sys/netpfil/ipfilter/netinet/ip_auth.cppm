export module pbsd.port.wave6.hbsd.src.sys.netpfil.ipfilter.netinet.ip_auth;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netpfil/ipfilter/netinet/ip_auth.c
// void ip_auth_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netpfil/ipfilter/netinet/ip_auth.c wave=wave6 loc=1203
export namespace pbsd::port::wave6::hbsd::src::sys::netpfil::ipfilter::netinet::ip_auth {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netpfil::ipfilter::netinet::ip_auth
