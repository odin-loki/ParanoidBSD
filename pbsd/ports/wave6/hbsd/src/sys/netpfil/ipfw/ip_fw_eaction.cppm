export module pbsd.port.wave6.hbsd.src.sys.netpfil.ipfw.ip_fw_eaction;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netpfil/ipfw/ip_fw_eaction.c
// void ip_fw_eaction_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netpfil/ipfw/ip_fw_eaction.c wave=wave6 loc=463
export namespace pbsd::port::wave6::hbsd::src::sys::netpfil::ipfw::ip_fw_eaction {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netpfil::ipfw::ip_fw_eaction
