export module pbsd.port.wave6.hbsd.src.sys.netpfil.ipfw.dn_aqm;

module;
// Header bridge — replace #include of hbsd/src/sys/netpfil/ipfw/dn_aqm.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netpfil/ipfw/dn_aqm.h wave=wave6 loc=162
export namespace pbsd::port::wave6::hbsd::src::sys::netpfil::ipfw::dn_aqm {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netpfil::ipfw::dn_aqm
