export module pbsd.port.wave6.hbsd.src.sys.netipsec.keysock;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netipsec/keysock.c
// void keysock_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netipsec/keysock.c wave=wave6 loc=354
export namespace pbsd::port::wave6::hbsd::src::sys::netipsec::keysock {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netipsec::keysock
