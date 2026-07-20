export module pbsd.port.wave6.hbsd.src.sys.netinet6.in6_jail;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netinet6/in6_jail.c
// void in6_jail_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netinet6/in6_jail.c wave=wave6 loc=325
export namespace pbsd::port::wave6::hbsd::src::sys::netinet6::in6_jail {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netinet6::in6_jail
