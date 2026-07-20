export module pbsd.port.wave4.hbsd.src.sys.compat.linux.linux_misc;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/compat/linux/linux_misc.c
// void linux_misc_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/compat/linux/linux_misc.c wave=wave4 loc=3127
export namespace pbsd::port::wave4::hbsd::src::sys::compat::linux::linux_misc {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::compat::linux::linux_misc
