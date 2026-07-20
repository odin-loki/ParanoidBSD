export module pbsd.port.wave6.hbsd.src.sys.netpfil.ipfw.nat64.ip_fw_nat64;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netpfil/ipfw/nat64/ip_fw_nat64.c
// void ip_fw_nat64_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netpfil/ipfw/nat64/ip_fw_nat64.c wave=wave6 loc=151
export namespace pbsd::port::wave6::hbsd::src::sys::netpfil::ipfw::nat64::ip_fw_nat64 {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netpfil::ipfw::nat64::ip_fw_nat64
