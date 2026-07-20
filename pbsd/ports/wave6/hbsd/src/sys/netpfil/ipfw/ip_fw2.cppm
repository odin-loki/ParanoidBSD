export module pbsd.port.wave6.hbsd.src.sys.netpfil.ipfw.ip_fw2;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netpfil/ipfw/ip_fw2.c
// void ip_fw2_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netpfil/ipfw/ip_fw2.c wave=wave6 loc=3891
export namespace pbsd::port::wave6::hbsd::src::sys::netpfil::ipfw::ip_fw2 {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netpfil::ipfw::ip_fw2
