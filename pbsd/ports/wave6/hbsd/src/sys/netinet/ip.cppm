export module pbsd.port.wave6.hbsd.src.sys.netinet.ip;

module;
// Header bridge — replace #include of hbsd/src/sys/netinet/ip.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netinet/ip.h wave=wave6 loc=227
export namespace pbsd::port::wave6::hbsd::src::sys::netinet::ip {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netinet::ip
