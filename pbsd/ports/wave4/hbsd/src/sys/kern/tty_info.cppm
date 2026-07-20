export module pbsd.port.wave4.hbsd.src.sys.kern.tty_info;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kern/tty_info.c
// void tty_info_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kern/tty_info.c wave=wave4 loc=400
export namespace pbsd::port::wave4::hbsd::src::sys::kern::tty_info {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kern::tty_info
