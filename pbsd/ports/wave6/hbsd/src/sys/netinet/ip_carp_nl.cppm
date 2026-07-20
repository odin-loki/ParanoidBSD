export module pbsd.port.wave6.hbsd.src.sys.netinet.ip_carp_nl;

module;
// Header bridge — replace #include of hbsd/src/sys/netinet/ip_carp_nl.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netinet/ip_carp_nl.h wave=wave6 loc=40
export namespace pbsd::port::wave6::hbsd::src::sys::netinet::ip_carp_nl {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netinet::ip_carp_nl
