export module pbsd.port.wave6.hbsd.src.sys.netpfil.ipfw.pmod.ip_fw_pmod;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netpfil/ipfw/pmod/ip_fw_pmod.c
// void ip_fw_pmod_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netpfil/ipfw/pmod/ip_fw_pmod.c wave=wave6 loc=98
export namespace pbsd::port::wave6::hbsd::src::sys::netpfil::ipfw::pmod::ip_fw_pmod {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netpfil::ipfw::pmod::ip_fw_pmod
