export module pbsd.port.wave6.hbsd.src.sys.netipsec.ipsec_pcb;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netipsec/ipsec_pcb.c
// void ipsec_pcb_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netipsec/ipsec_pcb.c wave=wave6 loc=498
export namespace pbsd::port::wave6::hbsd::src::sys::netipsec::ipsec_pcb {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netipsec::ipsec_pcb
