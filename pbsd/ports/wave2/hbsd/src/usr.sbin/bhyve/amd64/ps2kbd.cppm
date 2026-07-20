export module pbsd.port.wave2.hbsd.src.usr_sbin.bhyve.amd64.ps2kbd;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.sbin/bhyve/amd64/ps2kbd.c
// void ps2kbd_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.sbin/bhyve/amd64/ps2kbd.c wave=wave2 loc=520
export namespace pbsd::port::wave2::hbsd::src::usr_sbin::bhyve::amd64::ps2kbd {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_sbin::bhyve::amd64::ps2kbd
