export module pbsd.port.wave6.hbsd.src.sys.netinet6.ip6_gre;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netinet6/ip6_gre.c
// void ip6_gre_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netinet6/ip6_gre.c wave=wave6 loc=578
export namespace pbsd::port::wave6::hbsd::src::sys::netinet6::ip6_gre {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netinet6::ip6_gre
