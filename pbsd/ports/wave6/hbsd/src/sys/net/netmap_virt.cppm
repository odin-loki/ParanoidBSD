export module pbsd.port.wave6.hbsd.src.sys.net.netmap_virt;

module;
// Header bridge — replace #include of hbsd/src/sys/net/netmap_virt.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/net/netmap_virt.h wave=wave6 loc=98
export namespace pbsd::port::wave6::hbsd::src::sys::net::netmap_virt {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::net::netmap_virt
