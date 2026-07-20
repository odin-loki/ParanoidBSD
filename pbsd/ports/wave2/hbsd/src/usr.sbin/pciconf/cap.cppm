export module pbsd.port.wave2.hbsd.src.usr_sbin.pciconf.cap;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.sbin/pciconf/cap.c
// void cap_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.sbin/pciconf/cap.c wave=wave2 loc=1348
export namespace pbsd::port::wave2::hbsd::src::usr_sbin::pciconf::cap {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_sbin::pciconf::cap
