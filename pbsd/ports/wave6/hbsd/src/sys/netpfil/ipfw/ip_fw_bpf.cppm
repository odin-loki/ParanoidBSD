export module pbsd.port.wave6.hbsd.src.sys.netpfil.ipfw.ip_fw_bpf;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netpfil/ipfw/ip_fw_bpf.c
// void ip_fw_bpf_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netpfil/ipfw/ip_fw_bpf.c wave=wave6 loc=202
export namespace pbsd::port::wave6::hbsd::src::sys::netpfil::ipfw::ip_fw_bpf {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netpfil::ipfw::ip_fw_bpf
