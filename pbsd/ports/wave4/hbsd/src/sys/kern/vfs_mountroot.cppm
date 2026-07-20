export module pbsd.port.wave4.hbsd.src.sys.kern.vfs_mountroot;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kern/vfs_mountroot.c
// void vfs_mountroot_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kern/vfs_mountroot.c wave=wave4 loc=1166
export namespace pbsd::port::wave4::hbsd::src::sys::kern::vfs_mountroot {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kern::vfs_mountroot
