export module pbsd.port.wave6.hbsd.src.sys.netinet6.ip6_ecn;

module;
// Header bridge — replace #include of hbsd/src/sys/netinet6/ip6_ecn.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netinet6/ip6_ecn.h wave=wave6 loc=42
export namespace pbsd::port::wave6::hbsd::src::sys::netinet6::ip6_ecn {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netinet6::ip6_ecn
