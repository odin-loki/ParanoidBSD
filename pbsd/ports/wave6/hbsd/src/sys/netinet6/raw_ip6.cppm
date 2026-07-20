export module pbsd.port.wave6.hbsd.src.sys.netinet6.raw_ip6;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netinet6/raw_ip6.c
// void raw_ip6_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netinet6/raw_ip6.c wave=wave6 loc=876
export namespace pbsd::port::wave6::hbsd::src::sys::netinet6::raw_ip6 {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netinet6::raw_ip6
