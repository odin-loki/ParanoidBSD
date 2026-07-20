export module pbsd.port.wave6.hbsd.src.sys.netinet.dccp;

module;
// Header bridge — replace #include of hbsd/src/sys/netinet/dccp.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netinet/dccp.h wave=wave6 loc=76
export namespace pbsd::port::wave6::hbsd::src::sys::netinet::dccp {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netinet::dccp
