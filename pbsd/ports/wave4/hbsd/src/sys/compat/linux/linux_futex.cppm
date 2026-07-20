export module pbsd.port.wave4.hbsd.src.sys.compat.linux.linux_futex;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/compat/linux/linux_futex.c
// void linux_futex_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/compat/linux/linux_futex.c wave=wave4 loc=1069
export namespace pbsd::port::wave4::hbsd::src::sys::compat::linux::linux_futex {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::compat::linux::linux_futex
