export module pbsd.port.wave6.hbsd.src.sys.netinet.tcp_ecn;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netinet/tcp_ecn.c
// void tcp_ecn_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netinet/tcp_ecn.c wave=wave6 loc=612
export namespace pbsd::port::wave6::hbsd::src::sys::netinet::tcp_ecn {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netinet::tcp_ecn
