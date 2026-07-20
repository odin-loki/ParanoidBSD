export module pbsd.port.wave6.hbsd.src.sys.netpfil.ipfw.ip_fw_pfil;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netpfil/ipfw/ip_fw_pfil.c
// void ip_fw_pfil_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netpfil/ipfw/ip_fw_pfil.c wave=wave6 loc=740
export namespace pbsd::port::wave6::hbsd::src::sys::netpfil::ipfw::ip_fw_pfil {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netpfil::ipfw::ip_fw_pfil
