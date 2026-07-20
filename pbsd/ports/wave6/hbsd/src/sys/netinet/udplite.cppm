export module pbsd.port.wave6.hbsd.src.sys.netinet.udplite;

module;
// Header bridge — replace #include of hbsd/src/sys/netinet/udplite.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netinet/udplite.h wave=wave6 loc=47
export namespace pbsd::port::wave6::hbsd::src::sys::netinet::udplite {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netinet::udplite
