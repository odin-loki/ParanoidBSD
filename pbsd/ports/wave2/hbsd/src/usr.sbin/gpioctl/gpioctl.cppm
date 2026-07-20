export module pbsd.port.wave2.hbsd.src.usr_sbin.gpioctl.gpioctl;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.sbin/gpioctl/gpioctl.c
// void gpioctl_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.sbin/gpioctl/gpioctl.c wave=wave2 loc=380
export namespace pbsd::port::wave2::hbsd::src::usr_sbin::gpioctl::gpioctl {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_sbin::gpioctl::gpioctl
