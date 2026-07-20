export module pbsd.port.wave4.hbsd.src.sys.compat.linux.linux_vdso;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/compat/linux/linux_vdso.c
// void linux_vdso_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/compat/linux/linux_vdso.c wave=wave4 loc=178
export namespace pbsd::port::wave4::hbsd::src::sys::compat::linux::linux_vdso {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::compat::linux::linux_vdso
