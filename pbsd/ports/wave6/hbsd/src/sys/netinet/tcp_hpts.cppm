export module pbsd.port.wave6.hbsd.src.sys.netinet.tcp_hpts;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netinet/tcp_hpts.c
// void tcp_hpts_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netinet/tcp_hpts.c wave=wave6 loc=2137
export namespace pbsd::port::wave6::hbsd::src::sys::netinet::tcp_hpts {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netinet::tcp_hpts
