export module pbsd.port.wave2.hbsd.src.usr_sbin.bhyve.amd64.ps2mouse;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.sbin/bhyve/amd64/ps2mouse.c
// void ps2mouse_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.sbin/bhyve/amd64/ps2mouse.c wave=wave2 loc=439
export namespace pbsd::port::wave2::hbsd::src::usr_sbin::bhyve::amd64::ps2mouse {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_sbin::bhyve::amd64::ps2mouse
