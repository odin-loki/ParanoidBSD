export module pbsd.port.wave2.hbsd.src.usr_sbin.bhyve.bootrom;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.sbin/bhyve/bootrom.c
// void bootrom_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.sbin/bhyve/bootrom.c wave=wave2 loc=326
export namespace pbsd::port::wave2::hbsd::src::usr_sbin::bhyve::bootrom {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_sbin::bhyve::bootrom
