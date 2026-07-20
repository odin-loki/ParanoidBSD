export module pbsd.port.wave4.hbsd.src.sys.kern.vfs_acl;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kern/vfs_acl.c
// void vfs_acl_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kern/vfs_acl.c wave=wave4 loc=599
export namespace pbsd::port::wave4::hbsd::src::sys::kern::vfs_acl {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kern::vfs_acl
