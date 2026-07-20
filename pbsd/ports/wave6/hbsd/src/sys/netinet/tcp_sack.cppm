export module pbsd.port.wave6.hbsd.src.sys.netinet.tcp_sack;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netinet/tcp_sack.c
// void tcp_sack_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netinet/tcp_sack.c wave=wave6 loc=1200
export namespace pbsd::port::wave6::hbsd::src::sys::netinet::tcp_sack {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netinet::tcp_sack
