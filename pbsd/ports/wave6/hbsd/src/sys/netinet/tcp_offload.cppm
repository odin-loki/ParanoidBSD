export module pbsd.port.wave6.hbsd.src.sys.netinet.tcp_offload;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netinet/tcp_offload.c
// void tcp_offload_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netinet/tcp_offload.c wave=wave6 loc=231
export namespace pbsd::port::wave6::hbsd::src::sys::netinet::tcp_offload {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netinet::tcp_offload
