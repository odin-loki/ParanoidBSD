export module pbsd.port.wave6.hbsd.src.sys.netpfil.ipfilter.netinet.ip_state;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netpfil/ipfilter/netinet/ip_state.c
// void ip_state_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netpfil/ipfilter/netinet/ip_state.c wave=wave6 loc=5257
export namespace pbsd::port::wave6::hbsd::src::sys::netpfil::ipfilter::netinet::ip_state {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netpfil::ipfilter::netinet::ip_state
