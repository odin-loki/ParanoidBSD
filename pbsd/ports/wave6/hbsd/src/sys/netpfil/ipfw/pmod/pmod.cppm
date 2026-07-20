export module pbsd.port.wave6.hbsd.src.sys.netpfil.ipfw.pmod.pmod;

module;
// Header bridge — replace #include of hbsd/src/sys/netpfil/ipfw/pmod/pmod.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netpfil/ipfw/pmod/pmod.h wave=wave6 loc=33
export namespace pbsd::port::wave6::hbsd::src::sys::netpfil::ipfw::pmod::pmod {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netpfil::ipfw::pmod::pmod
