export module pbsd.port.wave6.hbsd.src.sys.netinet6.nd6_nbr;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netinet6/nd6_nbr.c
// void nd6_nbr_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netinet6/nd6_nbr.c wave=wave6 loc=1633
export namespace pbsd::port::wave6::hbsd::src::sys::netinet6::nd6_nbr {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netinet6::nd6_nbr
