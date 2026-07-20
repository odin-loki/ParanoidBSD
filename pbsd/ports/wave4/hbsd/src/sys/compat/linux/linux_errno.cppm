export module pbsd.port.wave4.hbsd.src.sys.compat.linux.linux_errno;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/compat/linux/linux_errno.c
// void linux_errno_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/compat/linux/linux_errno.c wave=wave4 loc=37
export namespace pbsd::port::wave4::hbsd::src::sys::compat::linux::linux_errno {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::compat::linux::linux_errno
