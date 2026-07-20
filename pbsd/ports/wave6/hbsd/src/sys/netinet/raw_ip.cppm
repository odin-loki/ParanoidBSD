export module pbsd.port.wave6.hbsd.src.sys.netinet.raw_ip;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netinet/raw_ip.c
// void raw_ip_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netinet/raw_ip.c wave=wave6 loc=1131
export namespace pbsd::port::wave6::hbsd::src::sys::netinet::raw_ip {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netinet::raw_ip
