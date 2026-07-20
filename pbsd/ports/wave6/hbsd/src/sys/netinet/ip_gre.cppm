export module pbsd.port.wave6.hbsd.src.sys.netinet.ip_gre;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netinet/ip_gre.c
// void ip_gre_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netinet/ip_gre.c wave=wave6 loc=582
export namespace pbsd::port::wave6::hbsd::src::sys::netinet::ip_gre {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netinet::ip_gre
