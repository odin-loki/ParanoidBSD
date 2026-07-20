export module pbsd.port.wave6.hbsd.src.sys.netinet.tcp_lro;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netinet/tcp_lro.c
// void tcp_lro_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netinet/tcp_lro.c wave=wave6 loc=1492
export namespace pbsd::port::wave6::hbsd::src::sys::netinet::tcp_lro {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netinet::tcp_lro
