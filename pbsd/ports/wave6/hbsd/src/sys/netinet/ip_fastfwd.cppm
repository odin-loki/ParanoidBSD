export module pbsd.port.wave6.hbsd.src.sys.netinet.ip_fastfwd;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netinet/ip_fastfwd.c
// void ip_fastfwd_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netinet/ip_fastfwd.c wave=wave6 loc=588
export namespace pbsd::port::wave6::hbsd::src::sys::netinet::ip_fastfwd {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netinet::ip_fastfwd
