export module pbsd.port.wave6.hbsd.src.sys.netinet.in_jail;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netinet/in_jail.c
// void in_jail_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netinet/in_jail.c wave=wave6 loc=335
export namespace pbsd::port::wave6::hbsd::src::sys::netinet::in_jail {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netinet::in_jail
