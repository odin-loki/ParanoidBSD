export module pbsd.port.wave4.hbsd.src.sys.compat.freebsd32.freebsd32_ioctl;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/compat/freebsd32/freebsd32_ioctl.c
// void freebsd32_ioctl_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/compat/freebsd32/freebsd32_ioctl.c wave=wave4 loc=231
export namespace pbsd::port::wave4::hbsd::src::sys::compat::freebsd32::freebsd32_ioctl {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::compat::freebsd32::freebsd32_ioctl
