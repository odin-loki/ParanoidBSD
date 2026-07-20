export module pbsd.port.wave6.hbsd.src.sys.netpfil.ipfw.ip_fw_table;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netpfil/ipfw/ip_fw_table.c
// void ip_fw_table_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netpfil/ipfw/ip_fw_table.c wave=wave6 loc=3059
export namespace pbsd::port::wave6::hbsd::src::sys::netpfil::ipfw::ip_fw_table {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netpfil::ipfw::ip_fw_table
