export module pbsd.port.wave6.hbsd.src.sys.netinet.tcp_usrreq;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netinet/tcp_usrreq.c
// void tcp_usrreq_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netinet/tcp_usrreq.c wave=wave6 loc=3015
export namespace pbsd::port::wave6::hbsd::src::sys::netinet::tcp_usrreq {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netinet::tcp_usrreq
