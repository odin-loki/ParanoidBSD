export module pbsd.port.wave6.hbsd.src.sys.netinet.tcp_syncache;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netinet/tcp_syncache.c
// void tcp_syncache_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netinet/tcp_syncache.c wave=wave6 loc=2587
export namespace pbsd::port::wave6::hbsd::src::sys::netinet::tcp_syncache {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netinet::tcp_syncache
