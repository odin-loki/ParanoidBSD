export module pbsd.port.wave6.hbsd.src.sys.netipsec.key;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netipsec/key.c
// void key_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netipsec/key.c wave=wave6 loc=9022
export namespace pbsd::port::wave6::hbsd::src::sys::netipsec::key {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netipsec::key
