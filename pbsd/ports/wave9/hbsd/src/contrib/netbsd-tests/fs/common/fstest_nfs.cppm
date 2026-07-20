export module pbsd.port.wave9.hbsd.src.contrib.netbsd_tests.fs.common.fstest_nfs;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/netbsd-tests/fs/common/fstest_nfs.c
// void fstest_nfs_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/netbsd-tests/fs/common/fstest_nfs.c wave=wave9 loc=326
export namespace pbsd::port::wave9::hbsd::src::contrib::netbsd_tests::fs::common::fstest_nfs {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::netbsd_tests::fs::common::fstest_nfs
