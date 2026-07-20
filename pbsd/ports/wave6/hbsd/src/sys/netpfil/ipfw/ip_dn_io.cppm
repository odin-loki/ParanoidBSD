export module pbsd.port.wave6.hbsd.src.sys.netpfil.ipfw.ip_dn_io;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netpfil/ipfw/ip_dn_io.c
// void ip_dn_io_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netpfil/ipfw/ip_dn_io.c wave=wave6 loc=1009
export namespace pbsd::port::wave6::hbsd::src::sys::netpfil::ipfw::ip_dn_io {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netpfil::ipfw::ip_dn_io
