export module pbsd.port.wave6.hbsd.src.sys.netipsec.udpencap;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netipsec/udpencap.c
// void udpencap_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netipsec/udpencap.c wave=wave6 loc=409
export namespace pbsd::port::wave6::hbsd::src::sys::netipsec::udpencap {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netipsec::udpencap
