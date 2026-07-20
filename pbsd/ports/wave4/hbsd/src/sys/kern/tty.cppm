export module pbsd.port.wave4.hbsd.src.sys.kern.tty;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kern/tty.c
// void tty_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kern/tty.c wave=wave4 loc=2531
export namespace pbsd::port::wave4::hbsd::src::sys::kern::tty {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kern::tty
