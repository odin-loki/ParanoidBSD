export module pbsd.port.wave2.hbsd.src.usr_sbin.rtsold.cap_sendmsg;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.sbin/rtsold/cap_sendmsg.c
// void cap_sendmsg_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.sbin/rtsold/cap_sendmsg.c wave=wave2 loc=282
export namespace pbsd::port::wave2::hbsd::src::usr_sbin::rtsold::cap_sendmsg {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_sbin::rtsold::cap_sendmsg
