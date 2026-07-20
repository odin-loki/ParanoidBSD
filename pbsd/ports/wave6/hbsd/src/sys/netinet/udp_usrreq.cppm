export module pbsd.port.wave6.hbsd.src.sys.netinet.udp_usrreq;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netinet/udp_usrreq.c
// void udp_usrreq_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netinet/udp_usrreq.c wave=wave6 loc=1825
export namespace pbsd::port::wave6::hbsd::src::sys::netinet::udp_usrreq {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netinet::udp_usrreq
