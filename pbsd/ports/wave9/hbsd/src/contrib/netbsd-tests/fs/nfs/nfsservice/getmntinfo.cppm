export module pbsd.port.wave9.hbsd.src.contrib.netbsd_tests.fs.nfs.nfsservice.getmntinfo;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/netbsd-tests/fs/nfs/nfsservice/getmntinfo.c
// void getmntinfo_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/netbsd-tests/fs/nfs/nfsservice/getmntinfo.c wave=wave9 loc=85
export namespace pbsd::port::wave9::hbsd::src::contrib::netbsd_tests::fs::nfs::nfsservice::getmntinfo {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::netbsd_tests::fs::nfs::nfsservice::getmntinfo
