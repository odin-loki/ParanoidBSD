export module pbsd.port.wave6.hbsd.src.sys.netpfil.ipfw.ip_fw_iface;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netpfil/ipfw/ip_fw_iface.c
// void ip_fw_iface_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netpfil/ipfw/ip_fw_iface.c wave=wave6 loc=538
export namespace pbsd::port::wave6::hbsd::src::sys::netpfil::ipfw::ip_fw_iface {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netpfil::ipfw::ip_fw_iface
