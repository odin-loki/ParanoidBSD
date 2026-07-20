export module pbsd.port.wave4.hbsd.src.sys.kern.tty_pts;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kern/tty_pts.c
// void tty_pts_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kern/tty_pts.c wave=wave4 loc=870
export namespace pbsd::port::wave4::hbsd::src::sys::kern::tty_pts {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kern::tty_pts
