export module pbsd.port.wave6.hbsd.src.sys.netpfil.ipfw.nat64.nat64clat;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netpfil/ipfw/nat64/nat64clat.c
// void nat64clat_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netpfil/ipfw/nat64/nat64clat.c wave=wave6 loc=252
export namespace pbsd::port::wave6::hbsd::src::sys::netpfil::ipfw::nat64::nat64clat {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netpfil::ipfw::nat64::nat64clat
