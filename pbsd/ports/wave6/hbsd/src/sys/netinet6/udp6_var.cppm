export module pbsd.port.wave6.hbsd.src.sys.netinet6.udp6_var;

module;
// Header bridge — replace #include of hbsd/src/sys/netinet6/udp6_var.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netinet6/udp6_var.h wave=wave6 loc=71
export namespace pbsd::port::wave6::hbsd::src::sys::netinet6::udp6_var {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netinet6::udp6_var
