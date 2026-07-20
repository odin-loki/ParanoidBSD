export module pbsd.port.wave6.hbsd.src.sys.netinet6.in6_pcb;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netinet6/in6_pcb.c
// void in6_pcb_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netinet6/in6_pcb.c wave=wave6 loc=1306
export namespace pbsd::port::wave6::hbsd::src::sys::netinet6::in6_pcb {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netinet6::in6_pcb
