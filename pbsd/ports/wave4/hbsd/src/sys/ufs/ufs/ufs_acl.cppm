export module pbsd.port.wave4.hbsd.src.sys.ufs.ufs.ufs_acl;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/ufs/ufs/ufs_acl.c
// void ufs_acl_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/ufs/ufs/ufs_acl.c wave=wave4 loc=683
export namespace pbsd::port::wave4::hbsd::src::sys::ufs::ufs::ufs_acl {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::ufs::ufs::ufs_acl
