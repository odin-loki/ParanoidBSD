export module pbsd.port.wave6.hbsd.src.sys.netpfil.pf.pf_mtag;

module;
// Header bridge — replace #include of hbsd/src/sys/netpfil/pf/pf_mtag.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netpfil/pf/pf_mtag.h wave=wave6 loc=73
export namespace pbsd::port::wave6::hbsd::src::sys::netpfil::pf::pf_mtag {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netpfil::pf::pf_mtag
