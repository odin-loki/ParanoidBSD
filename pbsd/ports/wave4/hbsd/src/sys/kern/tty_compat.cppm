export module pbsd.port.wave4.hbsd.src.sys.kern.tty_compat;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kern/tty_compat.c
// void tty_compat_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kern/tty_compat.c wave=wave4 loc=482
export namespace pbsd::port::wave4::hbsd::src::sys::kern::tty_compat {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kern::tty_compat
