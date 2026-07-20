export module pbsd.port.wave6.hbsd.src.sys.netsmb.netbios;

module;
// Header bridge — replace #include of hbsd/src/sys/netsmb/netbios.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netsmb/netbios.h wave=wave6 loc=134
export namespace pbsd::port::wave6::hbsd::src::sys::netsmb::netbios {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netsmb::netbios
