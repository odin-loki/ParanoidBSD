export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.tests.zfs_tests.cmd.readmmap;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/tests/zfs-tests/cmd/readmmap.c
// void readmmap_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/tests/zfs-tests/cmd/readmmap.c wave=wave6 loc=140
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::tests::zfs_tests::cmd::readmmap {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::tests::zfs_tests::cmd::readmmap
