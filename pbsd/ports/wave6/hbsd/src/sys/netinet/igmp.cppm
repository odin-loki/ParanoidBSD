export module pbsd.port.wave6.hbsd.src.sys.netinet.igmp;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netinet/igmp.c
// void igmp_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netinet/igmp.c wave=wave6 loc=3733
export namespace pbsd::port::wave6::hbsd::src::sys::netinet::igmp {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netinet::igmp
