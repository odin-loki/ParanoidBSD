export module pbsd.port.wave6.hbsd.src.sys.net.netisr;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/net/netisr.c
// void netisr_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/net/netisr.c wave=wave6 loc=1562
export namespace pbsd::port::wave6::hbsd::src::sys::net::netisr {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::net::netisr
