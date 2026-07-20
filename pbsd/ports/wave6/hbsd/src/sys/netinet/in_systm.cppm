export module pbsd.port.wave6.hbsd.src.sys.netinet.in_systm;

module;
// Header bridge — replace #include of hbsd/src/sys/netinet/in_systm.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netinet/in_systm.h wave=wave6 loc=69
export namespace pbsd::port::wave6::hbsd::src::sys::netinet::in_systm {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netinet::in_systm
