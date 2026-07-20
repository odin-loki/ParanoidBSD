export module pbsd.port.wave6.hbsd.src.sys.netinet.tcp_reass;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netinet/tcp_reass.c
// void tcp_reass_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netinet/tcp_reass.c wave=wave6 loc=1115
export namespace pbsd::port::wave6::hbsd::src::sys::netinet::tcp_reass {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netinet::tcp_reass
