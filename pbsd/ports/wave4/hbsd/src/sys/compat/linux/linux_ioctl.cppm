export module pbsd.port.wave4.hbsd.src.sys.compat.linux.linux_ioctl;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/compat/linux/linux_ioctl.c
// void linux_ioctl_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/compat/linux/linux_ioctl.c wave=wave4 loc=3865
export namespace pbsd::port::wave4::hbsd::src::sys::compat::linux::linux_ioctl {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::compat::linux::linux_ioctl
