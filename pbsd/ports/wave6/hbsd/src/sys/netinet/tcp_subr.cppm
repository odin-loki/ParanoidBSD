export module pbsd.port.wave6.hbsd.src.sys.netinet.tcp_subr;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netinet/tcp_subr.c
// void tcp_subr_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netinet/tcp_subr.c wave=wave6 loc=5048
export namespace pbsd::port::wave6::hbsd::src::sys::netinet::tcp_subr {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netinet::tcp_subr
