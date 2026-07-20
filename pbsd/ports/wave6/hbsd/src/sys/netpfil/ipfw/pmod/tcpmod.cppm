export module pbsd.port.wave6.hbsd.src.sys.netpfil.ipfw.pmod.tcpmod;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netpfil/ipfw/pmod/tcpmod.c
// void tcpmod_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netpfil/ipfw/pmod/tcpmod.c wave=wave6 loc=249
export namespace pbsd::port::wave6::hbsd::src::sys::netpfil::ipfw::pmod::tcpmod {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netpfil::ipfw::pmod::tcpmod
