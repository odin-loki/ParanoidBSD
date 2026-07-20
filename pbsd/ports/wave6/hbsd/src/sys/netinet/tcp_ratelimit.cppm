export module pbsd.port.wave6.hbsd.src.sys.netinet.tcp_ratelimit;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netinet/tcp_ratelimit.c
// void tcp_ratelimit_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netinet/tcp_ratelimit.c wave=wave6 loc=1795
export namespace pbsd::port::wave6::hbsd::src::sys::netinet::tcp_ratelimit {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netinet::tcp_ratelimit
