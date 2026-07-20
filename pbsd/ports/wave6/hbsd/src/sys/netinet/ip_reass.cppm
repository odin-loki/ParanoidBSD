export module pbsd.port.wave6.hbsd.src.sys.netinet.ip_reass;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netinet/ip_reass.c
// void ip_reass_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netinet/ip_reass.c wave=wave6 loc=990
export namespace pbsd::port::wave6::hbsd::src::sys::netinet::ip_reass {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netinet::ip_reass
