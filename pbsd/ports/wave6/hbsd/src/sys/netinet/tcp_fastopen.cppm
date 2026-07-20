export module pbsd.port.wave6.hbsd.src.sys.netinet.tcp_fastopen;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netinet/tcp_fastopen.c
// void tcp_fastopen_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netinet/tcp_fastopen.c wave=wave6 loc=1219
export namespace pbsd::port::wave6::hbsd::src::sys::netinet::tcp_fastopen {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netinet::tcp_fastopen
