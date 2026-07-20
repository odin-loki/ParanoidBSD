export module pbsd.port.wave6.hbsd.src.sys.fs.msdosfs.msdosfs_fat;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/fs/msdosfs/msdosfs_fat.c
// void msdosfs_fat_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/fs/msdosfs/msdosfs_fat.c wave=wave6 loc=1215
export namespace pbsd::port::wave6::hbsd::src::sys::fs::msdosfs::msdosfs_fat {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::fs::msdosfs::msdosfs_fat
