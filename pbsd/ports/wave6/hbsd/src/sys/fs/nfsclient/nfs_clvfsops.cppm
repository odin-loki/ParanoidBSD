export module pbsd.port.wave6.hbsd.src.sys.fs.nfsclient.nfs_clvfsops;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/fs/nfsclient/nfs_clvfsops.c
// void nfs_clvfsops_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/fs/nfsclient/nfs_clvfsops.c wave=wave6 loc=2288
export namespace pbsd::port::wave6::hbsd::src::sys::fs::nfsclient::nfs_clvfsops {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::fs::nfsclient::nfs_clvfsops
