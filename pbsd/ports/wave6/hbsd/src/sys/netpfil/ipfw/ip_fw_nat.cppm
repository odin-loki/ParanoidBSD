export module pbsd.port.wave6.hbsd.src.sys.netpfil.ipfw.ip_fw_nat;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netpfil/ipfw/ip_fw_nat.c
// void ip_fw_nat_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netpfil/ipfw/ip_fw_nat.c wave=wave6 loc=1248
export namespace pbsd::port::wave6::hbsd::src::sys::netpfil::ipfw::ip_fw_nat {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netpfil::ipfw::ip_fw_nat
