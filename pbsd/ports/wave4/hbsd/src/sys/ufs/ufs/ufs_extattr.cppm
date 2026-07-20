export module pbsd.port.wave4.hbsd.src.sys.ufs.ufs.ufs_extattr;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/ufs/ufs/ufs_extattr.c
// void ufs_extattr_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/ufs/ufs/ufs_extattr.c wave=wave4 loc=1298
export namespace pbsd::port::wave4::hbsd::src::sys::ufs::ufs::ufs_extattr {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::ufs::ufs::ufs_extattr
