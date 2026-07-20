export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.tests.zfs_tests.cmd.mmap_sync;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/tests/zfs-tests/cmd/mmap_sync.c
// void mmap_sync_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/tests/zfs-tests/cmd/mmap_sync.c wave=wave6 loc=153
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::tests::zfs_tests::cmd::mmap_sync {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::tests::zfs_tests::cmd::mmap_sync
