export module pbsd.port.wave6.hbsd.src.sys.netinet6.in6_cksum;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netinet6/in6_cksum.c
// void in6_cksum_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netinet6/in6_cksum.c wave=wave6 loc=343
export namespace pbsd::port::wave6::hbsd::src::sys::netinet6::in6_cksum {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netinet6::in6_cksum
