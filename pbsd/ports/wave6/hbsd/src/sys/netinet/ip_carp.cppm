export module pbsd.port.wave6.hbsd.src.sys.netinet.ip_carp;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netinet/ip_carp.c
// void ip_carp_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netinet/ip_carp.c wave=wave6 loc=3123
export namespace pbsd::port::wave6::hbsd::src::sys::netinet::ip_carp {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netinet::ip_carp
