export module pbsd.port.wave6.hbsd.src.sys.netinet.ip_mroute;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netinet/ip_mroute.c
// void ip_mroute_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netinet/ip_mroute.c wave=wave6 loc=3030
export namespace pbsd::port::wave6::hbsd::src::sys::netinet::ip_mroute {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netinet::ip_mroute
